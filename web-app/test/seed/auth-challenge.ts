import { useConnection } from "@root/backend/db/pool.js";
import { getAuthSessions, type Sessions } from "./auth-session.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { strict as assert } from 'node:assert';
import type { ChallengeDefs } from "./account-challenge-defs.js";
import { getAccountChallengeDefs } from "./account-challenge-defs.js";

const getStatements = (sessions: Sessions, challenges: ChallengeDefs) => {
    assert(sessions.length > 3, "need more sessions");
    assert(challenges.length > 0, "need more challenges");
    const session1 = sessions[0];
    const session2 = sessions[1];
    const session3 = sessions[2];
    const session4 = sessions[3];
    const challenge = challenges.find(
        c => c.accountVersion === session1.accountVersion
            && c.accountVersion === session2.accountVersion
            && c.accountVersion === session3.accountVersion
            && c.accountVersion === session4.accountVersion);
    assert(challenge, "could not find appropriate challenge for 4 sessions.");
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