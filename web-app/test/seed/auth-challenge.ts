import { useConnection } from "@root/backend/db/pool.js";
import { getAuthSessions, type Sessions } from "./auth-session.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { strict as assert } from 'node:assert';
import type { ChallengeDefs } from "./account-challenge-defs.js";
import { getAccountChallengeDefs } from "./account-challenge-defs.js";

const getStatements = (sessions: Sessions, challenges: ChallengeDefs) => {
    assert(sessions.length >= 7, "need more sessions");
    assert(challenges.length > 0, "need more challenges");
    const session1 = sessions[0];
    const session2 = sessions[1];
    const session3 = sessions[2];
    const session4 = sessions[3];
    const session5 = sessions[4];
    const session6 = sessions[5];
    const session7 = sessions[6];
    const challenge = challenges.find(
        c => c.accountVersion === session1.accountVersion
            && c.accountVersion === session2.accountVersion
            && c.accountVersion === session3.accountVersion
            && c.accountVersion === session4.accountVersion
            && c.accountVersion === session5.accountVersion
            && c.accountVersion === session6.accountVersion
            && c.accountVersion === session7.accountVersion);
    assert(challenge, "could not find appropriate challenge for 6 sessions.");
    assert(session1.outcome === EAuthOutcome.Pending, "session1 must be in pending state");
    assert(session2.outcome === EAuthOutcome.Aborted, "session2 must be in aborted state");
    assert(session3.outcome === EAuthOutcome.Succeeded, "session3 must be in succeeded state");
    assert(session4.outcome === EAuthOutcome.Pending, "session4 must be in pending state");
    assert(session5.outcome === EAuthOutcome.Pending, "session5 must be in pending state");
    assert(session6.outcome === EAuthOutcome.Pending, "session6 must be in pending state");
    assert(session7.outcome === EAuthOutcome.Pending, "session7 must be in pending state");
    return [
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session1.sessionId}, '${challenge.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session2.sessionId}, '${challenge.challengeId}', 'Aborted')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session3.sessionId}, '${challenge.challengeId}', 'Succeeded')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session4.sessionId}, '${challenge.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session5.sessionId}, '${challenge.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session6.sessionId}, '${challenge.challengeId}', 'Succeeded')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session7.sessionId}, '${challenge.challengeId}', 'Aborted')`,
    ];
}


let cachedAuthChallenges: Array<{
    sessionId: BigInt;
    challengeId: string;
    outcome: EAuthOutcome;
}> | null = null;

export const createAuthChallenges = async () => {
    return useConnection(async conn => {
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const stmts = getStatements(sessions, challengeDefs);
        assert(stmts.length > 0);
        for (let i = 0; i < stmts.length; i++) {
            await conn.unsafe(stmts[i]);
        }
        const rows = await conn`
            SELECT SessionID, ChallengeID, Outcome
            FROM berytus_account_auth_challenge
            ORDER BY CreatedAt ASC
        `;
        cachedAuthChallenges = rows.map(({
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
        assert(cachedAuthChallenges.length > 0);
        return [...cachedAuthChallenges];
    });
}

export const getAuthChallenges = async () => {
    if (cachedAuthChallenges) {
        return cachedAuthChallenges;
    }
    const result = await createAuthChallenges();
    assert(result.length > 0);
    return result;
}

export type AuthChallenges = Awaited<ReturnType<typeof getAuthChallenges>>;