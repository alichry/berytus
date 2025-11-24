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

    it("Should reject updating a challenge outcome with an invalid status msg", async () => {
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
            )).to.be.rejectedWith('statusMsg is malformed');
        }

    });

    xit("Should reject challenge message creation when challenge is not pending", async () => {
        // pick a pending sesson, pick challenge aborted/succeeded
    });

    xit("Should reject challenge message creation when session is not pending", async () => {
        // pick a non-pending sesson, pick challenge of all states
    });

    xit("Should reject challenge message creation with invalid message name [unrecognised message name]", async () => {

    });

    xit("Should reject challenge message creation with invalid message name [incorrect next message name]", async () => {

    });
});