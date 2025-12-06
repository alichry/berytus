import * as chai  from 'chai';
import { createAccountDefs } from "@test/seed/account-defs.js";
import chaiAsPromised from 'chai-as-promised';
import { createAccounts } from '@test/seed/account.js';
import { createAccountChallengeDefs, getAccountChallengeDefs, type ChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { strict as assert } from 'node:assert';
import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import { AuthChallengeMessage, type ChallengeMessageStatus } from '@root/backend/db/models/AuthChallengeMessage.js';
import { AccountField } from '@root/backend/db/models/AccountField.js';
import { PublicKeyFieldValue } from '@root/backend/logic/field-handler/DigitalSignatureHandler.js';
const { expect } = chai;
chai.use(chaiAsPromised);
import { ArmoredKeyUtils } from "@root/backend/utils/key-utils.js";
import { subtle, randomBytes } from "node:crypto";
import type { JSONValue } from '@root/backend/db/types';

describe("Berytus Digital Signature Challenge Handler", () => {

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });

    const composeMessages = async (
        session: Awaited<ReturnType<typeof getAuthSessions>>[0],
        challengeDef: Awaited<ReturnType<typeof getAccountChallengeDefs>>[0],
        nonceData: number[],
        messageResults: Partial<Record<BerytusDigitalSignatureChallengeMessageName, {
            response: JSONValue;
            statusMsg: ChallengeMessageStatus
        }>>
    ) => {
        assert(session.outcome === EAuthOutcome.Pending);
        assert(challengeDef.challengeType === EChallengeType.DigitalSignature);
        const keyFieldId =
            challengeDef.challengeParameters["keyFieldId"];
        expect(keyFieldId).to.be.a('string');
        const keyField = await AccountField.getField(
            session.accountVersion,
            session.accountId,
            keyFieldId as string
        );
        assert(Object.keys(messageResults).length > 0);
        return await Promise.all((
            Object.keys(messageResults) as BerytusDigitalSignatureChallengeMessageName[]
        ).map(async messageName => {
            const value = messageResults[messageName];
            assert(value, `Key: ${messageName} must not be undefined`);
            const {
                response,
                statusMsg
            } = value;
            switch (messageName) {
                case "SelectKey": {
                    return {
                        messageName,
                        request: keyFieldId,
                        expected: {
                            id: keyFieldId,
                            value: await PublicKeyFieldValue.parseAsync(keyField.fieldValue)
                        },
                        response,
                        statusMsg
                    }
                }
                case "SignNonce": {
                    return {
                        messageName,
                        request: Buffer.from(nonceData).toString('base64'),
                        expected: null,
                        response,
                        statusMsg
                    }
                }
                default:
                    assert(false, `unrecognised messageName:${messageName}`);
            }
        }))
    }

    const generateKeyPair = async () => {
        const genKey = await crypto.subtle.generateKey(
            {
                name: "RSASSA-PKCS1-v1_5",
                modulusLength: 4096,
                publicExponent: new Uint8Array([1, 0, 1]),
                hash: "SHA-256",
            },
            true,
            ["sign", "verify"]
        );
        const privKeyBuf = Buffer.from(await subtle.exportKey(
            "pkcs8", genKey.privateKey
        ));
        const pubKeyBuf = Buffer.from(await subtle.exportKey(
            "spki",
            genKey.publicKey
        ));
        return {
            cryptoKey: genKey,
            public: {
                raw: pubKeyBuf,
                armored: ArmoredKeyUtils.armorBase64(
                    pubKeyBuf.toString('base64'),
                    "public"
                )
            },
            private: {
                raw: privKeyBuf,
                armored: ArmoredKeyUtils.armorBase64(
                    pubKeyBuf.toString('base64'),
                    "private"
                )
            }
        };
    }

    const getSessionAndChallengeDef = async () => {
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        for (const session of sessions) {
            if (session.outcome !== EAuthOutcome.Pending) {
                continue;
            }
            const challengeDef = challengeDefs.find(
                d => d.challengeType === EChallengeType.DigitalSignature
                    && d.accountVersion === session.accountVersion
            );
            if (! challengeDef) {
                continue;
            }
            const hasPasswordCh = challenges.find(
                c => c.challengeId === challengeDef.challengeId
                    && c.sessionId === session.sessionId
            );
            if (hasPasswordCh) {
                continue;
            }
            return { session, challengeDef };
        }
        assert(false, "can't find appropriate session . challenge def");
    }

    const testCases = {
        successful: async () => {
            const {
                session,
                challengeDef
            } = await getSessionAndChallengeDef();
            const keyFieldId =
            challengeDef.challengeParameters["keyFieldId"];
            expect(keyFieldId).to.be.a('string');
            const keyField = await AccountField.getField(
                session.accountVersion,
                session.accountId,
                keyFieldId as string
            );
            const testKey = await generateKeyPair();
            await keyField.updateValue({
                publicKey: testKey.public.armored
            });
            const nonceData = [0x1, 0x2, 0x3];
            // @ts-ignore
            const randomBytesStub: typeof randomBytes = (size: number, cb: unknown) => {
                const buf = Buffer.from(nonceData);
                if (typeof cb !== "undefined") {
                    throw new Error("callback not supprted");
                }
                return buf;
            };
            const signedNonce = Buffer.from(await subtle.sign(
                "RSASSA-PKCS1-v1_5",
                testKey.cryptoKey.privateKey,
                new Uint8Array(nonceData)
            )).toString('base64');
            return {
                session,
                challengeDef,
                randomBytesStub,
                messages: await composeMessages(session, challengeDef, nonceData, {
                    SelectKey: {
                        response: {
                            id: keyFieldId,
                            value: {
                                publicKey: testKey.public.armored
                            }
                        },
                        statusMsg: 'Ok'
                    },
                    SignNonce: {
                        response: signedNonce,
                        statusMsg: 'Ok'
                    },
                }),
            }
        }
    }

    it("Should handle all successful messages in one shot", async () => {
        const {
            session,
            challengeDef,
            messages,
            randomBytesStub
        } = await testCases.successful();
        const ch = await setupChallenge(
            session.sessionId,
            challengeDef.challengeId,
            undefined,
            { randomBytes: randomBytesStub }
        );
        expect(ch.challengeDef).to.deep.equal(challengeDef);
        const previousMessages: Record<string, typeof messages[0]> = {};
        for (let i = 0; i < messages.length; i++) {
            const message = messages[i];
            const pendingMessage = await ch.getPendingMessage();
            expect(pendingMessage).to.not.be.null;
            expect(await ch.getPendingMessage()).to.deep.equal({
                ...message,
                response: null,
                statusMsg: null,
            });
            await ch.processPendingMessageResponse(message.response);
            previousMessages[message.messageName] = message;
            expect((await ch.getMessages()).processedMessages)
                .to.deep.equal(previousMessages);
        }
        expect(await ch.getPendingMessage()).to.be.null;
        await ch.save();
        const expectedChallenge = {
            challengeDef,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            outcome: EAuthOutcome.Succeeded
        };
        expect(ch.challenge).to.deep.equal(expectedChallenge);
        expect(await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.deep.equal(expectedChallenge);
        expect(await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).to.deep.equal(Object.values(previousMessages).map(pm => ({
            ...pm,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));
    });
});