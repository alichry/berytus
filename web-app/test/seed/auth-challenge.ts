import { useConnection } from "@root/backend/db/pool.js";
import { getAuthSessions, type Sessions } from "./auth-session.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { strict as assert } from 'node:assert';
import type { ChallengeDefs } from "./account-challenge-defs.js";
import { getAccountChallengeDefs } from "./account-challenge-defs.js";
import { EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";

const getStatements = (sessions: Sessions, challengeDefs: ChallengeDefs) => {
    assert(sessions.length >= 13, "need more sessions");
    const session1 = sessions[0];
    const session2 = sessions[1];
    const session3 = sessions[2];
    const session4 = sessions[3];
    const session5 = sessions[4];
    const session6 = sessions[5];
    const session7 = sessions[6];
    const session10 = sessions[9];
    const session11 = sessions[10];
    const session12 = sessions[11];
    const session13 = sessions[12];
    const passwordChallengeDef = challengeDefs.find(
        c =>  c.challengeType === EChallengeType.Password
            && c.accountVersion === session1.accountVersion
            && c.accountVersion === session2.accountVersion
            && c.accountVersion === session3.accountVersion
            && c.accountVersion === session4.accountVersion
            && c.accountVersion === session5.accountVersion
            && c.accountVersion === session6.accountVersion
            && c.accountVersion === session7.accountVersion
            && c.accountVersion === session10.accountVersion
            && c.accountVersion === session11.accountVersion
            && c.accountVersion === session12.accountVersion);
    const srpChallengeDef = challengeDefs.find(
        d => d.challengeType === EChallengeType.SecureRemotePassword
            && d.accountVersion === session13.accountVersion
    );
    assert(passwordChallengeDef, "could not find appropriate pass challenge for 10 sessions.");
    assert(srpChallengeDef, "could not find appropriate srp challenge for 1 session");
    assert(session1.outcome === EAuthOutcome.Pending, "session1 must be in pending state");
    assert(session2.outcome === EAuthOutcome.Aborted, "session2 must be in aborted state");
    assert(session3.outcome === EAuthOutcome.Succeeded, "session3 must be in succeeded state");
    assert(session4.outcome === EAuthOutcome.Pending, "session4 must be in pending state");
    assert(session5.outcome === EAuthOutcome.Pending, "session5 must be in pending state");
    assert(session6.outcome === EAuthOutcome.Pending, "session6 must be in pending state");
    assert(session7.outcome === EAuthOutcome.Pending, "session7 must be in pending state");
    assert(session10.outcome === EAuthOutcome.Aborted, "session10 must be in aborted state");
    assert(session11.outcome === EAuthOutcome.Pending, "session11 must be in a pending state");
    assert(session12.outcome === EAuthOutcome.Aborted, "session12 must be in an aborted state");
    assert(session13.outcome === EAuthOutcome.Pending, "session13 must be in a pending state");
    return [
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session1.sessionId}, '${passwordChallengeDef.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session2.sessionId}, '${passwordChallengeDef.challengeId}', 'Aborted')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session3.sessionId}, '${passwordChallengeDef.challengeId}', 'Succeeded')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session4.sessionId}, '${passwordChallengeDef.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session5.sessionId}, '${passwordChallengeDef.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session6.sessionId}, '${passwordChallengeDef.challengeId}', 'Succeeded')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session7.sessionId}, '${passwordChallengeDef.challengeId}', 'Aborted')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session10.sessionId}, '${passwordChallengeDef.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session11.sessionId}, '${passwordChallengeDef.challengeId}', 'Aborted')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session12.sessionId}, '${passwordChallengeDef.challengeId}', 'Pending')`,
        `INSERT INTO berytus_account_auth_challenge
        (SessionID, ChallengeID, Outcome)
        VALUES
        (${session13.sessionId}, '${srpChallengeDef.challengeId}', 'Pending')`,
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