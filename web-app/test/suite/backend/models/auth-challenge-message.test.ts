import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts } from '@test/seed/account.js';
import { pool } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallengeMessage } from '@root/backend/db/models/AuthChallengeMessage.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError.js';
import { AuthError } from '@root/backend/db/errors/AuthError.js';
const { expect } = chai;
chai.use(chaiAsPromised);


describe("Berytus Auth Challenge Message", () => {
    const getExistingMessages = async () => {
        const rows = await pool`
            SELECT SessionID, ChallengeID, StatusMsg,
                   MessageName, Request, Expected, Response
            FROM berytus_account_auth_challenge_message
            ORDER BY CreatedAt ASC
        `;
        assert(rows.length > 0);
        return rows.map(({
            sessionid: sessionId,
            challengeid: challengeId,
            messagename: messageName,
            request,
            expected,
            response,
            statusmsg: statusMsg
        }) => {
            assert(typeof sessionId === 'bigint');
            assert(sessionId > 0);
            assert(typeof challengeId === 'string');
            assert(challengeId.trim().length > 0);
            assert(typeof messageName === 'string');
            assert(messageName.trim().length > 0);
            assert(typeof statusMsg === 'string' || statusMsg === null);
            assert(statusMsg ? statusMsg.match(/^(Ok|Error:.*)$/) : true);
            assert(typeof request !== 'undefined');
            assert(typeof expected !== 'undefined');
            expect(typeof response !== 'undefined');
            return {
                sessionId,
                challengeId,
                messageName,
                request,
                expected,
                response,
                statusMsg
            };
        });
    }

    /**
     *
     * @returns A Password challenge that is associated with zero messages
     */
    const getPasswordChallenge = async () => {
        const existingMessages = await getExistingMessages();
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const challenge = challenges.find(
            c => {
                const session = sessions.find(s => s.sessionId === c.sessionId);
                assert(session, "session not found");
                if (session.outcome !== EAuthOutcome.Pending) {
                    return false;
                }
                const challengeDef = challengeDefs.find(
                    d => d.challengeId === c.challengeId
                        && d.accountVersion === session.accountVersion
                );
                assert(challengeDef, "challengeDef not found");
                if (challengeDef.challengeType !== 'Password') {
                    return false;
                }
                const hasMessage = -1 !== existingMessages.findIndex(m => m.sessionId === c.sessionId && m.challengeId === c.challengeId);
                return ! hasMessage;
            }
        );
        assert(challenge, `did not find an appropriate password challenge`);
        assert(! existingMessages.find(
            c => c.challengeId === challenge.challengeId && c.sessionId === challenge.sessionId),
            `expected existing messages to not include a message for password challenge [sessonId=${challenge.sessionId}, challengeId=${challenge.challengeId}]`
        );
        return challenge;
    }

    /**
     * Get an SRP challenge in pending with zero messages.
     */
    const getSrpChallenge = async () => {
        const existingMessages = await getExistingMessages();
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();

        for (const challenge of challenges) {
            const sess = sessions.find(
                s => s.sessionId === challenge.sessionId
            );
            assert(sess);
            if (sess.outcome !== EAuthOutcome.Pending) {
                continue;
            }
            const def = challengeDefs.find(
                d => d.accountVersion === sess.accountVersion
                    && d.challengeId === challenge.challengeId
            );
            assert(def);
            if (def.challengeType !== EChallengeType.SecureRemotePassword) {
                continue;
            }
            const msgs = existingMessages.filter(
                m => m.sessionId === challenge.sessionId
                    && m.challengeId === challenge.challengeId
            );
            if (msgs.length) {
                continue;
            }
            return challenge;
        }
        assert(false);
    }

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });

    it("Should retrieve existing challenge messages", async () => {
        const existingMessages = await getExistingMessages();

        const existingMessagesBySessionId =
            existingMessages.reduce((prev, curr) => {
                if (!(String(curr.sessionId) in prev)) {
                    prev[String(curr.sessionId)] = {};
                }
                const sub = prev[String(curr.sessionId)];
                if (!(curr.challengeId in sub)) {
                    sub[curr.challengeId] = [];
                }
                sub[curr.challengeId].push(curr);
                return prev;
            }, {} as Record<string, Record<string, Awaited<ReturnType<typeof getExistingMessages>>>>);
        for (const sessionId in existingMessagesBySessionId) {
            for (const challengeId in existingMessagesBySessionId[sessionId]) {
                const expectedExistingMessages = existingMessagesBySessionId[sessionId][challengeId];
                const actualMessages = await AuthChallengeMessage.getAllMessages(
                        BigInt(sessionId),
                        challengeId
                    );
                expect(expectedExistingMessages).to.have.deep.members(actualMessages);
            }
        }
    });

    it("Should correctly create a challenge message", async () => {
        const challenge = await getPasswordChallenge();
        await AuthChallengeMessage.createMessage(
            challenge.sessionId,
            challenge.challengeId,
            'GetPasswordFields',
            ["password"],
            [ { id: "password", password: "helloWorld" } ],
            [ { id: "password", password: "wrongPass" } ]
        );
    });

    it("Should update a challenge message response [Ok] and status", async () => {
        const existingMessages = await getExistingMessages();
        const pendingMessage = existingMessages.find(c => c.statusMsg === null);
        assert(pendingMessage, "could not find appropriate pending message");
        const messages = await AuthChallengeMessage.getAllMessages(pendingMessage.sessionId, pendingMessage.challengeId);
        expect(messages).to.deep.include(pendingMessage);
        const message = messages.find(m => m.messageName === pendingMessage.messageName);
        expect(message).to.deep.include(pendingMessage);

        await message!.updateResponseAndStatus({ 'magicWord': '777' }, 'Ok');
        expect(message).to.not.deep.equal(pendingMessage);
        expect(message).to.deep.equal({
            ...pendingMessage,
            statusMsg: 'Ok',
            response: { 'magicWord': '777' }
        });

        const existingMessagesAtT1 = await getExistingMessages();
        expect(existingMessagesAtT1).to.not.deep.include(pendingMessage);
        expect(existingMessagesAtT1).to.deep.include(message);

        const messagesAtT1 = await AuthChallengeMessage.getAllMessages(pendingMessage.sessionId, pendingMessage.challengeId);
        expect(messagesAtT1).to.not.deep.include(pendingMessage);
        expect(messagesAtT1).to.deep.include(message);
    });

    it("Should update a challenge message response [Error] and status", async () => {
        const existingMessages = await getExistingMessages();
        const pendingMessage = existingMessages.find(c => c.statusMsg === null);
        assert(pendingMessage, "could not find appropriate pending message");
        const messages = await AuthChallengeMessage.getAllMessages(pendingMessage.sessionId, pendingMessage.challengeId);
        expect(messages).to.deep.include(pendingMessage);
        const message = messages.find(m => m.messageName === pendingMessage.messageName);
        expect(message).to.deep.include(pendingMessage);

        await message!.updateResponseAndStatus({ 'magicWord': '777' }, 'Error:Test');
        expect(message).to.not.deep.equal(pendingMessage);
        expect(message).to.deep.equal({
            ...pendingMessage,
            statusMsg: 'Error:Test',
            response: { 'magicWord': '777' }
        });

        const existingMessagesAtT1 = await getExistingMessages();
        expect(existingMessagesAtT1).to.not.deep.include(pendingMessage);
        expect(existingMessagesAtT1).to.deep.include(message);

        const messagesAtT1 = await AuthChallengeMessage.getAllMessages(pendingMessage.sessionId, pendingMessage.challengeId);
        expect(messagesAtT1).to.not.deep.include(pendingMessage);
        expect(messagesAtT1).to.deep.include(message);
    });

    it("Should reject updating a message status with an invalid status msg", async () => {
        const cases = [
            'hello',
            '',
            null,
            'ok2',
            'ok',
            'Ok2',
            'error:',
            'Error:',
            'Error2:H'
        ]
        const dummyChallengeMessage = new AuthChallengeMessage(
            0n,
            'zero',
            'GetPasswordFields',
            null,
            null,
            null,
            null
        );
        for (const testCase of cases) {
            await expect(dummyChallengeMessage.updateResponseAndStatus(
                'helloWorld',
                // @ts-ignore
                testCase
            )).to.be.rejectedWith(
                InvalidArgError,
                'statusMsg is malformed'
            );
            expect(await getExistingMessages()).to.not.deep.include({
                sessionId: dummyChallengeMessage.sessionId,
                challengeId: dummyChallengeMessage.challengeId,
                messageName: testCase,
                request: null,
                expected: null,
                response: null,
                statusMsg: null
            });
        }
    });

    it("Should reject updating a message status of a message with a non null status msg", async () => {
        const existingMessages = await getExistingMessages();
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const [session, challenge, message] = (() => {
            for (const message of existingMessages) {
                if (message.statusMsg === null) {
                    continue;
                }
                const sess = sessions.find(
                    s => s.sessionId === message.sessionId
                );
                assert(sess, 'sess');
                if (sess.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const challenge = challenges.find(
                    c => c.sessionId === sess.sessionId
                        && c.challengeId === message.challengeId
                );
                assert(challenge, "challenge");
                if (challenge.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                return [sess, challenge, message];
            }
            assert(false, "false");
        })();
        assert(session.outcome === EAuthOutcome.Pending, "session.outcome === EAuthOutcome.Pending");
        assert(challenge.outcome === EAuthOutcome.Pending, "challenge.outcome === EAuthOutcome.Pending");
        assert(session.sessionId === challenge.sessionId, "session.sessionId === challenge.challengeId");
        assert(message.challengeId === challenge.challengeId, "message.challengeId === challenge.challengeId");
        assert(message.sessionId === session.sessionId, "message.sessionId === session.sessionId");
        assert(message.statusMsg !== null, "message.statusMsg !== null");
        const retrievedMessages = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challenge.challengeId
        );
        const retrievedMessage = retrievedMessages.find(
            m => m.messageName === message.messageName
        );
        assert(retrievedMessage, "retrievedMessage");

        await expect(retrievedMessage.updateResponseAndStatus(
            ["hello world"],
            'Ok'
        )).to.be.rejectedWith(
            AuthError,
            'statusMsg is already set; Refusing to update message status'
        );
        const prop = "statusMsg";
        assert(retrievedMessage[prop] !== null);
        Object.defineProperty(
            retrievedMessage,
            prop,
            { value: null }
        );
        assert(retrievedMessage[prop] === null);
        await expect(retrievedMessage.updateResponseAndStatus(
            ["hello world"],
            'Ok'
        )).to.be.rejectedWith(
            AuthError,
            'Cannot update response and status. Integrity validation failed'
        );
    });

    it("Should reject updating a message status of in non pending challenge", async () => {
        const existingMessages = await getExistingMessages();
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const desiredOutcomes = [EAuthOutcome.Aborted];
        const testCases = desiredOutcomes.map(desiredOutcome => {
            return (() => {
                for (const message of existingMessages) {
                    if (message.statusMsg !== null) {
                        continue;
                    }
                    const sess = sessions.find(
                        s => s.sessionId === message.sessionId
                    );
                    assert(sess, 'sess');
                    if (sess.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    const challenge = challenges.find(
                        c => c.sessionId === sess.sessionId
                            && c.challengeId === message.challengeId
                    );
                    assert(challenge, "challenge");
                    if (challenge.outcome !== desiredOutcome) {
                        continue;
                    }
                    return [sess, challenge, message] as const;
                }
                assert(false, "cant find appropriate session . challenge . message");
            })();
        });
        for (const [session, challenge, message] of testCases) {
            assert(session.outcome === EAuthOutcome.Pending, "session.outcome === EAuthOutcome.Pending");
            assert(challenge.outcome !== EAuthOutcome.Pending, "challenge.outcome !== EAuthOutcome.Pending");
            assert(session.sessionId === challenge.sessionId, "session.sessionId === challenge.challengeId");
            assert(message.challengeId === challenge.challengeId, "message.challengeId === challenge.challengeId");
            assert(message.sessionId === session.sessionId, "message.sessionId === session.sessionId");
            assert(message.statusMsg === null, "message.statusMsg === null");
            const retrievedMessages = await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challenge.challengeId
            );
            const retrievedMessage = retrievedMessages.find(
                m => m.messageName === message.messageName
            );
            assert(retrievedMessage, "retrievedMessage");

            await expect(retrievedMessage.updateResponseAndStatus(
                ["hello world"],
                'Ok'
            )).to.be.rejectedWith(
                AuthError,
                'Cannot update response and status. Integrity validation failed'
            );
        }
    });

    it("Should reject updating a message status of in non pending session", async () => {
        const existingMessages = await getExistingMessages();
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const desiredOutcomes = [EAuthOutcome.Aborted];
        const testCases = desiredOutcomes.map(desiredOutcome => {
            return (() => {
                for (const message of existingMessages) {
                    if (message.statusMsg !== null) {
                        continue;
                    }
                    const sess = sessions.find(
                        s => s.sessionId === message.sessionId
                    );
                    assert(sess, 'sess');
                    if (sess.outcome !== desiredOutcome) {
                        continue;
                    }
                    const challenge = challenges.find(
                        c => c.sessionId === sess.sessionId
                            && c.challengeId === message.challengeId
                    );
                    assert(challenge, "challenge");
                    if (challenge.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    return [sess, challenge, message] as const;
                }
                assert(false, "cant find appropriate session . challenge . message");
            })();
        });
        for (const [session, challenge, message] of testCases) {
            assert(session.outcome !== EAuthOutcome.Pending, "session.outcome !== EAuthOutcome.Pending");
            assert(challenge.outcome === EAuthOutcome.Pending, "challenge.outcome === EAuthOutcome.Pending");
            assert(session.sessionId === challenge.sessionId, "session.sessionId === challenge.challengeId");
            assert(message.challengeId === challenge.challengeId, "message.challengeId === challenge.challengeId");
            assert(message.sessionId === session.sessionId, "message.sessionId === session.sessionId");
            assert(message.statusMsg === null, "message.statusMsg === null");
            const retrievedMessages = await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challenge.challengeId
            );
            const retrievedMessage = retrievedMessages.find(
                m => m.messageName === message.messageName
            );
            assert(retrievedMessage, "retrievedMessage");

            await expect(retrievedMessage.updateResponseAndStatus(
                ["hello world"],
                'Ok'
            )).to.be.rejectedWith(
                AuthError,
                'Cannot update response and status. Integrity validation failed'
            );
        }
    });

    it("Should reject challenge message creation when challenge is not pending", async () => {
        // pick a pending sesson, pick challenge aborted/succeeded
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const existingMessages = await getExistingMessages();
        const desiredOutcomes = [EAuthOutcome.Aborted];
        const testCases = desiredOutcomes.map(desiredOutcome => {
            return (() => {
                for (const challenge of challenges) {
                    if (challenge.outcome !== desiredOutcome) {
                        continue;
                    }
                    const session = sessions.find(
                        s => s.sessionId === challenge.sessionId
                    );
                    assert(session, "session");
                    if (session.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    const challengeDef = challengeDefs.find(
                        d => d.accountVersion === session.accountVersion
                            && d.challengeId === challenge.challengeId
                    );
                    assert(challengeDef, "challengeDef");
                    if (challengeDef.challengeType !== EChallengeType.Password) {
                        continue;
                    }
                    const hasMessage = existingMessages.find(
                        m => m.sessionId === challenge.sessionId
                            && m.challengeId === challenge.challengeId
                    );
                    if (hasMessage) {
                        continue;
                    }
                    return [session, challenge] as const;
                }
                assert(false, "cant find appropriate session . challenge");
            })();
        });
        for (const [session, challenge] of testCases) {
            assert(session.outcome === EAuthOutcome.Pending, "session.outcome === EAuthOutcome.Pending");
            assert(challenge.outcome !== EAuthOutcome.Pending, "challenge.outcome !== EAuthOutcome.Pending");
            assert(session.sessionId === challenge.sessionId, "session.sessionId === challenge.challengeId");
            assert(! existingMessages.find(
                m => m.sessionId === challenge.sessionId
                    && m.challengeId === challenge.challengeId
            ), "existingMessages.find(...)");

            await expect(AuthChallengeMessage.createMessage(
                session.sessionId,
                challenge.challengeId,
                'GetPasswordFields',
                ["password"],
                {"password": "123"},
                {"password": "123"}
            )).to.be.rejectedWith(
                AuthError,
                'Cannot create message. Auth challenge is NOT in pending state, rather it is in Aborted state'
            );
        }
    });

    it("Should reject challenge message creation when session is not pending", async () => {
        // pick a non-pending sesson, pick challenge of all states
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const existingMessages = await getExistingMessages();
        const desiredOutcomes = [EAuthOutcome.Aborted];
        const testCases = desiredOutcomes.map(desiredOutcome => {
            return (() => {
                for (const challenge of challenges) {
                    if (challenge.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    const session = sessions.find(
                        s => s.sessionId === challenge.sessionId
                    );
                    assert(session, "session");
                    if (session.outcome !== desiredOutcome) {
                        continue;
                    }
                    const challengeDef = challengeDefs.find(
                        d => d.accountVersion === session.accountVersion
                            && d.challengeId === challenge.challengeId
                    );
                    assert(challengeDef, "challengeDef");
                    if (challengeDef.challengeType !== EChallengeType.Password) {
                        continue;
                    }
                    const hasMessage = existingMessages.find(
                        m => m.sessionId === challenge.sessionId
                            && m.challengeId === challenge.challengeId
                    );
                    if (hasMessage) {
                        continue;
                    }
                    return [session, challenge] as const;
                }
                assert(false, "cant find appropriate session . challenge");
            })();
        });
        for (const [session, challenge] of testCases) {
            assert(session.outcome !== EAuthOutcome.Pending, "session.outcome !== EAuthOutcome.Pending");
            assert(challenge.outcome === EAuthOutcome.Pending, "challenge.outcome === EAuthOutcome.Pending");
            assert(session.sessionId === challenge.sessionId, "session.sessionId === challenge.challengeId");
            assert(! existingMessages.find(
                m => m.sessionId === challenge.sessionId
                    && m.challengeId === challenge.challengeId
            ), "existingMessages.find(...)");

            await expect(AuthChallengeMessage.createMessage(
                session.sessionId,
                challenge.challengeId,
                'GetPasswordFields',
                ["password"],
                {"password": "123"},
                {"password": "123"}
            )).to.be.rejectedWith(
                AuthError,
                'Cannot create message. Auth session is NOT in pending state, rather it is in Aborted state'
            );
        }
    });

    it("Should reject challenge message creation with invalid message name [unrecognised message name]", async () => {
        const challenge = await getPasswordChallenge();
        const cases = [
            null,
            '',
            'GetPasswordFields2',
            'Get',
            1123,
            true,
            false,
            123n
        ]
        for (const testCase of cases) {
            await expect(AuthChallengeMessage.createMessage(
                challenge.sessionId,
                challenge.challengeId,
                // @ts-ignore
                testCase,
                ["password"],
                { "password": "test123" },
                { "password": "test123" }
            )).to.be.rejectedWith(
                InvalidArgError,
                `Cannot create message. Passed message name ` +
                `'${testCase}' is not appropriatet for the challenge.`
            );
            expect(await getExistingMessages()).to.not.deep.include({
                sessionId: challenge.sessionId,
                challengeId: challenge.challengeId,
                messageName: testCase,
                request: ["password"],
                expected: { "password": "test123" },
                response: { "password": "test123" },
                statusMsg: null
            });
        }
    });

    it("Should reject challenge message creation with invalid message name [incorrect next message name]", async () => {
        const existingMessagesAtT0 = await getExistingMessages();
        const challenge = await getSrpChallenge();
        const msg1 = await AuthChallengeMessage.createMessage(
            challenge.sessionId,
            challenge.challengeId,
            'SelectSecurePassword',
            ["username"],
            null,
            null
        );
        await msg1.updateResponseAndStatus(null, 'Ok');
        expect(existingMessagesAtT0).to.not.deep.include(msg1);
        const existingMessagesAtT1 = await getExistingMessages();
        expect(existingMessagesAtT1).to.deep.include(msg1);

        const cases = [
            "SelectSecurePassword", // already created
            // "ExchangePublicKeys" <-- expected
            "ComputeClientProof" // too far ahead
        ] as const;

        for (const testCase of cases) {
            await expect(AuthChallengeMessage.createMessage(
                challenge.sessionId,
                challenge.challengeId,
                testCase,
                null,
                null,
                null
            )).to.be.rejectedWith(
                AuthError,
                `Cannot create message. Challenge message ` +
                `integrity check failed.`
            );
            expect(await getExistingMessages()).to.not.deep.include({
                sessionId: challenge.sessionId,
                challengeId: challenge.challengeId,
                messageName: testCase,
                request: null,
                expected: null,
                response: null,
                statusMsg: null
            });
        }
    });
});