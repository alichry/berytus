import * as chai  from 'chai';
import { createAccountDefs } from "@test/seed/account-defs.js";
import chaiAsPromised from 'chai-as-promised';
import { createAccounts } from '@test/seed/account.js';
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { strict as assert } from 'node:assert';
import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import { AuthChallengeMessage, type ChallengeMessageStatus } from '@root/backend/db/models/AuthChallengeMessage.js';
import { AccountField } from '@root/backend/db/models/AccountField.js';
import { SelectSecurePasswordExpected } from '@root/backend/logic/challenge-handler/SecureRemotePasswordChallengeHandler.js';
import type { JSONValue } from '@root/shared-types';
import { EncodedSecurePasswordFieldValue } from '@root/backend/logic/field-handler/SecurePasswordHandler.js';
import { AccountDefField } from '@root/backend/db/models/AccountDefField.js';

const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus Secure Remote Password Challenge Handler", () => {

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
        { salt, valueB, valueM1, valueM2 }: {
            salt: ArrayBuffer,
            valueB: ArrayBuffer,
            valueM1: ArrayBuffer,
            valueM2: ArrayBuffer
        },
        messageResults: Partial<Record<BerytusSecureRemotePasswordChallengeMessageName, {
            response: JSONValue;
            statusMsg: ChallengeMessageStatus
        }>>
    ) => {
        assert(session.outcome === EAuthOutcome.Pending);
        assert(challengeDef.challengeType === EChallengeType.SecureRemotePassword);
        const srpFieldId =
            challengeDef.challengeParameters["field"];
        expect(srpFieldId).to.be.a('string');
        const srpDefField = await AccountDefField.getField(
            session.accountVersion,
            srpFieldId as string
        );
        const srpField = await AccountField.getField(
            session.accountVersion,
            session.accountId,
            srpFieldId as string
        );
        const srpFieldValue = await EncodedSecurePasswordFieldValue.parseAsync(srpField.fieldValue);
        const identityField = await AccountField.getField(
            session.accountVersion,
            session.accountId,
            (srpDefField.fieldOptions as BerytusSecurePasswordFieldOptions).identityFieldId
        );
        assert(Object.keys(messageResults).length > 0);
        return await Promise.all((
            Object.keys(messageResults) as BerytusSecureRemotePasswordChallengeMessageName[]
        ).map(async messageName => {
            const value = messageResults[messageName];
            assert(value, `Key: ${messageName} must not be undefined`);
            const {
                response,
                statusMsg
            } = value;
            switch (messageName) {
                case "SelectSecurePassword": {
                    return {
                        messageName,
                        request: null,
                        expected: await SelectSecurePasswordExpected.parseAsync(
                            identityField.fieldValue
                        ),
                        response,
                        statusMsg
                    }
                }
                case "ExchangePublicKeys": {
                    return {
                        messageName,
                        request: new Uint8Array(valueB)
                            // @ts-ignore: Node 25+
                            .toBase64(),
                        expected: null,
                        response,
                        statusMsg
                    }
                }
                case "ComputeClientProof": {
                    return {
                        messageName,
                        request: new Uint8Array(salt)
                            // @ts-ignore: Node 25+
                            .toBase64(),
                        expected: new Uint8Array(valueM1)
                            // @ts-ignore: Node 25+
                            .toBase64(),
                        response,
                        statusMsg
                    }
                }
                case "VerifyServerProof": {
                    return {
                        messageName,
                        request: new Uint8Array(valueM2)
                            // @ts-ignore: Node 25+
                            .toBase64(),
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

    const getTestAccount = () => {
        const identity = "bobby";
        const password = "hello098653zzxcvb";
        // randomBytes(32).toString('base64')
        const salt = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64("UDw3JH/gz3q3gA5NzSoJV9qqBFFuP176x9Nh4K64ZjU=");
        // require('fast-srp-hap').SRP.computeVerifier(require('fast-srp-hap').SRP.params[4096], salt, Buffer.from('bobby'), Buffer.from('hello098653zzxcvb')).toString('base64')
        const verifier = Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64("ygezf5UyT0HGiIyknyc5OhwpZCLERNOvvbmZwrDW0r06PVN3vsEOVUkPI8/sCoKIQA6IXYSlBG42jOZNW7klNZ6vz7vxWqWbpzh9TlBoctzCQn9sWhd5OlwzrSVUJoV+VKIClT8mCfYxPpNOGXygB/uXJ6gq8oN1Pu5RLZy7iFia0JxrJh3GkEWs5h1f4dO40e18nn/YqjmPejLjLQ+FgqGCenz8wYA+9U0J1RDX04p9ZZd3KZOGtgaU+zALSxdTtORJYMch1YLsIYDvBiZcN/cvvi8ZmzJi58Fi1QoIAT92CDbNXtCeu6X2qfN4sCveWwKYGq20LFOVCXlV/ucTEzLkwaUWCdKR9RSaZ5PvhVgnRlXZp9eLuVtnQBH+aqasJ9+S6uxF6dNr8LcgOQ63rt0pUuTJJcTjYgX0bmaS2nsx2oE53NX4Wknfscc+/Ut3sE5Ntk2nnkh2g8V2pYHwyX0TTyAyWRdP6ij1+Wnzj0LShhwE8IiZWkarllQnk4WsQquObhj7Iy2rPQA8rflqTwqIyLcL2lX3dpsivnvhk+DgD5ij5BzAtXylc0n7IJ4/B9xjTiBLI7dV14tEzfNCyLB9siICyLZy/RFxY6Yp877noZLjwKYdi1itAzNIfj/IRiTlEs+h86B41Qe5B+ykW9wWZ37Bmv+KQZu75UB/jYQ=");
        return {
            identity,
            password,
            salt,
            verifier
        };
    }

    const getTestSrpMaterial = () => {
        // depends on the exact values defined in getTestAccount()
        // randomBytes(32).toString('base64')
        const clientSecret = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                'iHHqBG2lQYXh0webev+ecxsMdaEtmN75bo0QDEhKEUM='
            );
        const serverSecret = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                'I6oT+N+qBV/bdZjcQ1FB2dHYTQPtDgv0FF7ZE4ZzfpU='
            );
        // new (require('fast-srp-hap').SrpClient)(require('fast-srp-hap').SRP.params[4096], Buffer.from(""), Buffer.from(""), Buffer.from(""), Buffer.from("iHHqBG2lQYXh0webev+ecxsMdaEtmN75bo0QDEhKEUM=", "base64")).computeA().toBase64()
        const valueA = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                '3fdsIkLySVomQhkbMc2LQT7yP5N0nB0rX+We9l0sQDKpqUCMXm7rFt7sMEQz5RNDaUPpLrgToszVVRENh6tdUjFc1gdaXu0/Y635pCXdzmc6CcybmPaWsmWDR7JfDkkl99S7DWFau3CFxciwFjQ/2EVbc/CmTN54mBEO34V1Lj99Vcq1xrvxOIQCwCKNDdlUw1Y+fRCEQCV4ryZHXCOhm1miezvNilTiJZL9FeeAHuJU251NEZTFmZBE5O+724wFixiB+kPxbqzce6ILAvPXBjJeR8P4r4O/FHGmiW/RpsyP+UQHapsDl1gL5DNw0dr/KN+GhUJnUft0silsYLQA/MXofPyqDIZ3QjirdD+2HceM16+A43wPXWqYCkGjsAYvwJH3HFTsRLkYdvsM3TGRRN1n5Am7AK+3+llt0Wfq+5nYpYXWOk+UE4tMy7ytOAALvVolT3HsshPNZbUY4oq7qHnj4hyDuU6bBqoO0jx6QYEKU37n1DQHfOEvRYBw87UeymtYs0bYh/pSyxNQrE4JaQkmwUDad7ll2srOcsVyYBiyti5pRxRku5TKb7KmNV3SKkjNWLQdqX+Z4ww36Ih1UaIMw91s1n9I2cd0SNI7v2uHHvQMlNQf9s04/x5PGtF2sT9+T7+9AnsHEMG+DH6IYObfsqF2CceEABkUfrjF6Ug='
            );
        // new (require('fast-srp-hap').SrpServer)(require('fast-srp-hap').SRP.params[4096], Buffer.from("ygezf5UyT0HGiIyknyc5OhwpZCLERNOvvbmZwrDW0r06PVN3vsEOVUkPI8/sCoKIQA6IXYSlBG42jOZNW7klNZ6vz7vxWqWbpzh9TlBoctzCQn9sWhd5OlwzrSVUJoV+VKIClT8mCfYxPpNOGXygB/uXJ6gq8oN1Pu5RLZy7iFia0JxrJh3GkEWs5h1f4dO40e18nn/YqjmPejLjLQ+FgqGCenz8wYA+9U0J1RDX04p9ZZd3KZOGtgaU+zALSxdTtORJYMch1YLsIYDvBiZcN/cvvi8ZmzJi58Fi1QoIAT92CDbNXtCeu6X2qfN4sCveWwKYGq20LFOVCXlV/ucTEzLkwaUWCdKR9RSaZ5PvhVgnRlXZp9eLuVtnQBH+aqasJ9+S6uxF6dNr8LcgOQ63rt0pUuTJJcTjYgX0bmaS2nsx2oE53NX4Wknfscc+/Ut3sE5Ntk2nnkh2g8V2pYHwyX0TTyAyWRdP6ij1+Wnzj0LShhwE8IiZWkarllQnk4WsQquObhj7Iy2rPQA8rflqTwqIyLcL2lX3dpsivnvhk+DgD5ij5BzAtXylc0n7IJ4/B9xjTiBLI7dV14tEzfNCyLB9siICyLZy/RFxY6Yp877noZLjwKYdi1itAzNIfj/IRiTlEs+h86B41Qe5B+ykW9wWZ37Bmv+KQZu75UB/jYQ=", "base64"), Buffer.from("I6oT+N+qBV/bdZjcQ1FB2dHYTQPtDgv0FF7ZE4ZzfpU=", "base64")).computeB().toBase64()
        const valueB = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                '9Fxz7QRh2258UFZyOvVgE9tEHyShGXIntQwAZNjk0HsLAUo5/WizjMZ/3vKmGWq3tjPQyude2VaZu3CUEw735g9i/kfl69k2NE4Wo4LI8nG7kDl4hTVAdqozd66zc6VHWsOTP7t2H4tlg5JIx1dKr0K1oQWlMW++X42JrMNr+XFXlOPLXgSW5s11chXqUB3hh2MaYdMHCjSGXi8PaQbxtrWqNNh46hXqB/F5fWt7SMaf11sSMsEJeG4n1jP5kxhUYkYYkf53Za/+ppqPMqfe9Arn/9qQzljraFUQ4b/x9sRIESIklaBUXaa+laBhkbNn9JvWpSA904QnQElukXelwk7O3iGNkUFCKrh8sEq5i30hbgykHT6MXDzwkWMUEqDfaR6whC1iVN4e/49e7i2UW6ogQnHnKbi5Djq7ukr4oVegU25GNCKwBh90wTUTd42sWWrMeJnvi8Iy4CDsHNwbTXTWvFn8H/nz0pnajaG+w2gsHntMX+4+zw3isZXvoJa89BVTHsbkUr93V3HCqy8f0u+o2761dbsApIgW6PI5NFyrG8NuOXVTleqK9RKFHi2egskWQgBQ0d5iAmFNmvTXHqGmrpqVT7SGGZU4emYJoAyJVD4/jJMDp5etGTxbS8O2sno5mzN8WhKx8uuAogT6neoOrPT3aF1XwwOr0Vj+ASY='
            );
        // var client = new (require('fast-srp-hap').SrpClient)(require('fast-srp-hap').SRP.params[4096], Buffer.from("UDw3JH/gz3q3gA5NzSoJV9qqBFFuP176x9Nh4K64ZjU=", "base64"), Buffer.from("bobby"), Buffer.from("hello098653zzxcvb"), Buffer.from("iHHqBG2lQYXh0webev+ecxsMdaEtmN75bo0QDEhKEUM=", "base64"));
        // client.setB(Buffer.from('9Fxz7QRh2258UFZyOvVgE9tEHyShGXIntQwAZNjk0HsLAUo5/WizjMZ/3vKmGWq3tjPQyude2VaZu3CUEw735g9i/kfl69k2NE4Wo4LI8nG7kDl4hTVAdqozd66zc6VHWsOTP7t2H4tlg5JIx1dKr0K1oQWlMW++X42JrMNr+XFXlOPLXgSW5s11chXqUB3hh2MaYdMHCjSGXi8PaQbxtrWqNNh46hXqB/F5fWt7SMaf11sSMsEJeG4n1jP5kxhUYkYYkf53Za/+ppqPMqfe9Arn/9qQzljraFUQ4b/x9sRIESIklaBUXaa+laBhkbNn9JvWpSA904QnQElukXelwk7O3iGNkUFCKrh8sEq5i30hbgykHT6MXDzwkWMUEqDfaR6whC1iVN4e/49e7i2UW6ogQnHnKbi5Djq7ukr4oVegU25GNCKwBh90wTUTd42sWWrMeJnvi8Iy4CDsHNwbTXTWvFn8H/nz0pnajaG+w2gsHntMX+4+zw3isZXvoJa89BVTHsbkUr93V3HCqy8f0u+o2761dbsApIgW6PI5NFyrG8NuOXVTleqK9RKFHi2egskWQgBQ0d5iAmFNmvTXHqGmrpqVT7SGGZU4emYJoAyJVD4/jJMDp5etGTxbS8O2sno5mzN8WhKx8uuAogT6neoOrPT3aF1XwwOr0Vj+ASY=', 'base64'))
        // client.computeM1().toString('base64')
        const valueM1 = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                'dQqeRSsiLJN8ypwPQZxa8LhgvbJS/jneIv/RbXaaAlo='
            )
        // var server = new (require('fast-srp-hap').SrpServer)(require('fast-srp-hap').SRP.params[4096], { username: Buffer.from("bobby"), salt: Buffer.from("UDw3JH/gz3q3gA5NzSoJV9qqBFFuP176x9Nh4K64ZjU=", "base64"), verifier: Buffer.from("ygezf5UyT0HGiIyknyc5OhwpZCLERNOvvbmZwrDW0r06PVN3vsEOVUkPI8/sCoKIQA6IXYSlBG42jOZNW7klNZ6vz7vxWqWbpzh9TlBoctzCQn9sWhd5OlwzrSVUJoV+VKIClT8mCfYxPpNOGXygB/uXJ6gq8oN1Pu5RLZy7iFia0JxrJh3GkEWs5h1f4dO40e18nn/YqjmPejLjLQ+FgqGCenz8wYA+9U0J1RDX04p9ZZd3KZOGtgaU+zALSxdTtORJYMch1YLsIYDvBiZcN/cvvi8ZmzJi58Fi1QoIAT92CDbNXtCeu6X2qfN4sCveWwKYGq20LFOVCXlV/ucTEzLkwaUWCdKR9RSaZ5PvhVgnRlXZp9eLuVtnQBH+aqasJ9+S6uxF6dNr8LcgOQ63rt0pUuTJJcTjYgX0bmaS2nsx2oE53NX4Wknfscc+/Ut3sE5Ntk2nnkh2g8V2pYHwyX0TTyAyWRdP6ij1+Wnzj0LShhwE8IiZWkarllQnk4WsQquObhj7Iy2rPQA8rflqTwqIyLcL2lX3dpsivnvhk+DgD5ij5BzAtXylc0n7IJ4/B9xjTiBLI7dV14tEzfNCyLB9siICyLZy/RFxY6Yp877noZLjwKYdi1itAzNIfj/IRiTlEs+h86B41Qe5B+ykW9wWZ37Bmv+KQZu75UB/jYQ=", "base64") }, Buffer.from("I6oT+N+qBV/bdZjcQ1FB2dHYTQPtDgv0FF7ZE4ZzfpU=", "base64"));
        // server.setA(Buffer.from('3fdsIkLySVomQhkbMc2LQT7yP5N0nB0rX+We9l0sQDKpqUCMXm7rFt7sMEQz5RNDaUPpLrgToszVVRENh6tdUjFc1gdaXu0/Y635pCXdzmc6CcybmPaWsmWDR7JfDkkl99S7DWFau3CFxciwFjQ/2EVbc/CmTN54mBEO34V1Lj99Vcq1xrvxOIQCwCKNDdlUw1Y+fRCEQCV4ryZHXCOhm1miezvNilTiJZL9FeeAHuJU251NEZTFmZBE5O+724wFixiB+kPxbqzce6ILAvPXBjJeR8P4r4O/FHGmiW/RpsyP+UQHapsDl1gL5DNw0dr/KN+GhUJnUft0silsYLQA/MXofPyqDIZ3QjirdD+2HceM16+A43wPXWqYCkGjsAYvwJH3HFTsRLkYdvsM3TGRRN1n5Am7AK+3+llt0Wfq+5nYpYXWOk+UE4tMy7ytOAALvVolT3HsshPNZbUY4oq7qHnj4hyDuU6bBqoO0jx6QYEKU37n1DQHfOEvRYBw87UeymtYs0bYh/pSyxNQrE4JaQkmwUDad7ll2srOcsVyYBiyti5pRxRku5TKb7KmNV3SKkjNWLQdqX+Z4ww36Ih1UaIMw91s1n9I2cd0SNI7v2uHHvQMlNQf9s04/x5PGtF2sT9+T7+9AnsHEMG+DH6IYObfsqF2CceEABkUfrjF6Ug=', 'base64'))
        // server.computeM2().toString('base64')
        const valueM2 = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                'lOgtI2Mia2qE5S9F961ELwEiLA7CC3YRABFSnO4e9JY='
            );
        // server.computeK().toString('base64') or client.computeK().toString('base64')
        const valueK = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(
                'Hr3UZGglT9Hxgk3Xch3TzplpIn/XD9bkd1B3zsw3Tnw='
            );
        return {
            clientSecret,
            serverSecret,
            valueA,
            valueB,
            valueM1,
            valueM2
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
                d => d.challengeType === EChallengeType.SecureRemotePassword
                    && d.accountVersion === session.accountVersion
            );
            if (! challengeDef) {
                continue;
            }
            const hasDsCh = challenges.find(
                c => c.challengeId === challengeDef.challengeId
                    && c.sessionId === session.sessionId
            );
            if (hasDsCh) {
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
            const srpFieldId =
                challengeDef.challengeParameters["field"];
            expect(srpFieldId).to.be.a('string');
            const srpDefField = await AccountDefField.getField(
                session.accountVersion,
                srpFieldId as string
            );
            const srpField = await AccountField.getField(
                session.accountVersion,
                session.accountId,
                srpFieldId as string
            );
            const identityField = await AccountField.getField(
                session.accountVersion,
                session.accountId,
                (srpDefField.fieldOptions as BerytusSecurePasswordFieldOptions).identityFieldId
            );
            const testAccount = getTestAccount();
            const testSrpMaterial = getTestSrpMaterial();
            await identityField.updateValue(
                testAccount.identity
            );
            await srpField.updateValue({
                salt: testAccount.salt.toBase64(),
                verifier: testAccount.verifier.toBase64()
            });
            // @ts-ignore
            const randomBytesStub: typeof randomBytes = (_size: number, cb: unknown) => {
                const buf = Buffer.from(testSrpMaterial.serverSecret);
                if (typeof cb !== "undefined") {
                    throw new Error("callback not supprted");
                }
                return buf;
            };
            return {
                session,
                challengeDef,
                randomBytesStub,
                payloadInputs: {
                    SelectSecurePassword: testAccount.identity,
                    ExchangePublicKeys: new Blob([testSrpMaterial.valueA], { type: "application/octet-stream" }),
                    ComputeClientProof: new Blob([testSrpMaterial.valueM1], { type: "application/octet-stream" }),
                    VerifyServerProof: null
                },
                messages: await composeMessages(session, challengeDef, {
                    salt: testAccount.salt.buffer,
                    valueB: testSrpMaterial.valueB.buffer,
                    valueM1: testSrpMaterial.valueM1.buffer,
                    valueM2: testSrpMaterial.valueM2.buffer
                }, {
                    SelectSecurePassword: {
                        response: testAccount.identity,
                        statusMsg: 'Ok'
                    },
                    ExchangePublicKeys: {
                        response: testSrpMaterial.valueA.toBase64(),
                        statusMsg: 'Ok'
                    },
                    ComputeClientProof: {
                        response: testSrpMaterial.valueM1.toBase64(),
                        statusMsg: 'Ok'
                    },
                    VerifyServerProof: {
                        response: null,
                        statusMsg: 'Ok'
                    }
                }),
            }
        }
    }

    it("Should handle all successful messages in one shot", async () => {
        const {
            session,
            challengeDef,
            messages,
            payloadInputs,
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
            await ch.processPendingMessageResponse(payloadInputs[message.messageName]);
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