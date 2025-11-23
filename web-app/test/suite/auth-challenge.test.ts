import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts } from '@test/seed/account.js';
import { pool } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages, getAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
const { expect } = chai;
chai.use(chaiAsPromised);

describe('Berytus Auth Challenge', () => {
    const getExistingChallenges = async () => {
        const rows = await pool`
            SELECT SessionID, ChallengeID, Outcome
            FROM berytus_account_auth_challenge
        `;
        assert(rows.length > 0);
        return rows.map(({
            sessionid: sessionId,
            challengeid: challengeId,
            outcome
        }) => {
            assert(typeof sessionId === 'bigint');
            assert(sessionId > 0);
            assert(typeof challengeId === 'string');
            assert(challengeId.trim().length > 0);
            assert(outcome in EAuthOutcome);
            return {
                sessionId,
                challengeId,
                outcome
            };
        });
    };

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });

    it("Should correctly retrieve existing challenge", async () => {
        const existingChallenges = await getExistingChallenges();
        for (const existingChallenge of existingChallenges) {
            const retrievedChallenge =
                await AuthChallenge.getChallenge(existingChallenge.sessionId, existingChallenge.challengeId);
            expect(retrievedChallenge.sessionId).to.equal(existingChallenge.sessionId);
            expect(retrievedChallenge.challengeId).to.equal(existingChallenge.challengeId);
            expect(retrievedChallenge.outcome).to.equal(existingChallenge.outcome);
        }
    });

    it("Should correctly create a challenge and return it", async () => {
        const existingChallenges = await getExistingChallenges();
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const [session, challengeDef] = (() => {
            for (const challengeDef of challengeDefs) {
                for (const session of sessions) {
                    if (session.accountVersion !== challengeDef.accountVersion) {
                        continue;
                    }
                    if (existingChallenges.find(
                            c => c.sessionId === session.sessionId
                            && c.challengeId === challengeDef.challengeId
                        )) {
                        continue;
                    }
                    return [session, challengeDef] as const;
                }
            }
            assert(false, "could not find an appropriate [session, challengeDef]");
        })();
        assert(session);
        assert(challengeDef);
        assert(session.accountVersion === challengeDef.accountVersion);
        assert(! existingChallenges.find(
            c => c.challengeId === challengeDef.challengeId
                && c.sessionId === session.sessionId
        ));

        const createdChallenge = await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(createdChallenge.sessionId).to.equal(session.sessionId);
        expect(createdChallenge.challengeId).to.equal(challengeDef.challengeId);
        expect(createdChallenge.outcome).to.equal(EAuthOutcome.Pending);
        expect(createdChallenge.challengeDef).to.deep.equal(challengeDef);
        expect(await getExistingChallenges()).to.deep.include({
            sessionId: createdChallenge.sessionId,
            challengeId: createdChallenge.challengeId,
            outcome: createdChallenge.outcome
        });

        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedChallenge).to.deep.equal(createdChallenge);
    });

    it("Should correctly update outcome to Succeeded", async () => {
        const existingChallenges = await getExistingChallenges();
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const challengeMessages = await getAuthChallengeMessages();
        const challenge = (() => {
            for (const challengeMessage of challengeMessages) {
                if (challengeMessage.statusMsg !== 'Ok') {
                    continue;
                }
                const challenge = existingChallenges.find(
                    c => c.sessionId === challengeMessage.sessionId
                        && c.challengeId === challengeMessage.challengeId
                );
                assert(challenge);
                if (challenge.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const session = sessions.find(s => s.sessionId === challenge.sessionId);
                assert(session);
                if (session.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const challengeDef = challengeDefs.find(
                    d => d.accountVersion === session.accountVersion
                        && d.challengeId === challenge.challengeId
                );
                assert(challengeDef);
                if (challengeDef.challengeType !== 'Password') {
                    continue;
                }
                return challenge;
            }
            assert(false, "cannot find an appropriate challenge");
        })();
        const session = sessions.find(
            s => s.sessionId === challenge.sessionId
        );
        assert(session);
        const challengeDef = challengeDefs.find(
            c => c.accountVersion === session.accountVersion
                && c.challengeId === challenge.challengeId
        );
        assert(challengeDef);
        const retrievedChallenge = await AuthChallenge.getChallenge(
            challenge.sessionId,
            challenge.challengeId
        );
        expect(retrievedChallenge).to.deep.equal({
            challengeDef,
            ...challenge
        });
        retrievedChallenge.updateOutcome(EAuthOutcome.Succeeded);
    });

    it("Should correctly update outcome to Aborted", async () => {
        const existingChallenges = await getExistingChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const sessions = await getAuthSessions();
        const challenge = existingChallenges.find(
            c => c.outcome === EAuthOutcome.Pending
        );
        assert(challenge);
        const session = sessions.find(
            s => s.sessionId === challenge.sessionId
        );
        assert(session);
        const challengeDef = challengeDefs.find(
            d => d.accountVersion === session.accountVersion
                && d.challengeId === challenge.challengeId
        );
        const retrievedChallenge = await AuthChallenge.getChallenge(
            challenge.sessionId,
            challenge.challengeId
        );
        expect(retrievedChallenge).to.deep.equal({
            challengeDef,
            ...challenge
        });
        await retrievedChallenge.updateOutcome(EAuthOutcome.Aborted);
        const updatedChallenge = {
            ...challenge,
            outcome: EAuthOutcome.Aborted
        };
        expect(retrievedChallenge).to.deep.equal({
            challengeDef,
            ...updatedChallenge
        });
        const existingChallengesAtT1 = await getExistingChallenges();
        expect(existingChallengesAtT1).to.not.deep.include(challenge);
        expect(existingChallengesAtT1).to.deep.include(updatedChallenge);
    });

    xit("Should reject challenge creation when session is not pending", async () => {

    });

    xit("Should reject updating outcome when Auth Session is not in pending state", async () => {

    });

    xit("Should reject updating outcome to Succeeded when not all messages have been processed", async () => {

    });

    xit("Should reject updating outcome to Pending", async () => {
        // Pending is the default state
    });

    xit("Should reject updating outcome when outcome has been already updated", async () => {

    });
});