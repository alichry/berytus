import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts } from '@test/seed/account.js';
import { pool } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages, getAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthError } from '@root/backend/db/errors/AuthError.js';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError.js';
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

    it("Should reject challenge creation when session is not pending [aborted]", async () => {
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const challenges = await getAuthChallenges();
        const [session, challengeDef] = (() => {
            for (const session of sessions) {
                if (session.outcome !== EAuthOutcome.Aborted) {
                    continue;
                }
                const correspondingChallengeDefs =
                    challengeDefs.filter(
                        d => d.accountVersion === session.accountVersion
                    );
                const correspondingChallenges =
                    challenges.filter(
                        c => c.sessionId === session.sessionId
                    );
                for (const challengeDef of correspondingChallengeDefs) {
                    const challenge = correspondingChallenges.find(
                        c => c.challengeId === challengeDef.challengeId
                    );
                    if (challenge) {
                        continue;
                    }
                    return [session, challengeDef]
                }
            }
            assert(false, "cant find appropriate session");
        })();
        assert(session, "session");
        assert(session.outcome === EAuthOutcome.Aborted, 'session.outcome === EAuthOutcome.Aborted');
        assert(session, "challengeDef");
        await expect(AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(
            AuthError,
            `Cannot create a new password challenge, ` +
            `auth session#${session.sessionId} is not pending`
        );
    });

    it("Should reject updating outcome to Pending", async () => {
        // Pending is the default state
        const states = new Set([EAuthOutcome.Aborted, EAuthOutcome.Pending, EAuthOutcome.Succeeded]);
        const sessions = await getAuthSessions();
        const challenges = await getExistingChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const targetChallenges = (() => {
            const result: typeof challenges = [];
            for (const state of states) {
                for (const session of sessions) {
                    if (session.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    const appropriateChallenge = challenges.find(
                        c => c.sessionId === session.sessionId
                            && c.outcome === state
                    );
                    if (! appropriateChallenge) {
                        continue;
                    }
                    result.push(appropriateChallenge);
                    break;
                }
            }
            assert(result.length === states.size, `cant find appropriate challenges`);
            return result;
        })();
        assert(targetChallenges.length > 0, "targetChallenges.length > 0");
        for (const challenge of targetChallenges) {
            const session = sessions.find(
                s => s.sessionId === challenge.sessionId
            );
            assert(session, "session");
            const challengeDef = challengeDefs.find(
                d => d.accountVersion === session.accountVersion
                    && d.challengeId === challenge.challengeId
            );
            assert(challengeDef, "challengeDef");
            const retrievedChallenge = await AuthChallenge.getChallenge(
                session.sessionId,
                challenge.challengeId
            );
            expect({
                challengeDef,
                ...challenge
            }).to.deep.equal(retrievedChallenge);
            await expect(
                retrievedChallenge.updateOutcome(EAuthOutcome.Pending)
            ).to.be.rejectedWith(
                InvalidArgError,
                `Cannot update ${challenge.challengeId} challenge outcome. `
                + `Refusing to update to default outcome of Pending.`
            )
        }
    });

    it("Should reject updating outcome when outcome has been already updated [succeeded]", async () => {
        // find a succeeded challenge in a pending session
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const challenges = await getExistingChallenges();
        const [session, challenge] = (() => {
            for (const session of sessions) {
                if (session.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const correspondingChallenges = challenges.filter(
                    c => c.sessionId === session.sessionId
                );
                for (const challenge of correspondingChallenges) {
                    if (challenge.outcome !== EAuthOutcome.Succeeded) {
                        continue;
                    }
                    return [session, challenge];
                }
            }
            assert(false, "can't find appropriate session/challenge");
        })();
        const challengeDef = challengeDefs.find(
            d => d.accountVersion === session.accountVersion
                && d.challengeId === challenge.challengeId
        );
        assert(challengeDef, "challengeDef");
        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challenge.challengeId
        );
        expect({
            challengeDef,
            ...challenge
        }).to.deep.equal(retrievedChallenge);
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Aborted)
        ).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Succeeded)
        ).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Pending)
        ).to.be.rejectedWith(
            InvalidArgError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Refusing to update to default outcome of Pending.`
        );
        // hack into the object, and set the outcome to pending
        // and see what happens..
        const prop = 'outcome';
        assert(retrievedChallenge[prop] === EAuthOutcome.Succeeded);
        Object.defineProperty(
            retrievedChallenge,
            prop,
            {
                value: EAuthOutcome.Pending
            }
        );
        // @ts-ignore
        assert(retrievedChallenge[prop] === EAuthOutcome.Pending);
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Aborted)
        ).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Succeeded)
        ).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Pending)
        ).to.be.rejectedWith(
            InvalidArgError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Refusing to update to default outcome of Pending.`
        );
    });

    it("Should reject updating outcome when Auth Session is not in pending state", async () => {
        // find a pending challenge in a non pending session
        const sessions = await getAuthSessions();
        const challenges = await getExistingChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const [session, challenge] = (() => {
            for (const session of sessions) {
                if (session.outcome === EAuthOutcome.Pending) {
                    continue;
                }
                const correspondingChallenges = challenges.filter(
                    c => c.sessionId === session.sessionId
                );
                for (const challenge of correspondingChallenges) {
                    if (challenge.outcome !== EAuthOutcome.Pending) {
                        continue;
                    }
                    return [session, challenge];
                }
            }
            assert(false, "can't find appropriate session/challenge");
        })();
        assert(session.outcome !== EAuthOutcome.Pending);
        assert(challenge.outcome === EAuthOutcome.Pending);
        const challengeDef = challengeDefs.find(
            d => d.accountVersion === session.accountVersion
                && d.challengeId === challenge.challengeId
        );
        assert(challengeDef, "challengeDef");
        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challenge.challengeId
        );
        expect({
            challengeDef,
            ...challenge
        }).to.deep.equal(retrievedChallenge);
        await expect(retrievedChallenge.updateOutcome(
            EAuthOutcome.Aborted
        )).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(retrievedChallenge.updateOutcome(
            EAuthOutcome.Succeeded
        )).to.be.rejectedWith(
            AuthError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Challenge does not exist or is not in a pending state, or `
            + `Session does not exist or is not in a pending state.`
        );
        await expect(
            retrievedChallenge.updateOutcome(EAuthOutcome.Pending)
        ).to.be.rejectedWith(
            InvalidArgError,
            `Cannot update ${challenge.challengeId} challenge outcome. `
            + `Refusing to update to default outcome of Pending.`
        );
    });

    xit("Should reject updating outcome to Succeeded when not all messages have been processed", async () => {

    });
});