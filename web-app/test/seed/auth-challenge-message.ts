import { useConnection } from "@root/backend/db/pool.js";
import { getAuthSessions, type Sessions } from "./auth-session.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { strict as assert } from 'node:assert';
import { getAccountChallengeDefs, type ChallengeDefs } from "./account-challenge-defs.js";
import { getAuthChallenges, type AuthChallenges } from "./auth-challenge.js";

const getStatements = (
    challengeDefs: ChallengeDefs,
    sessions: Sessions,
    challenges: AuthChallenges
) => {
    const packs = [];
    {
        const challenge = challenges.find(c => c.outcome === EAuthOutcome.Succeeded);
        assert(challenge, "challenge is unset");
        const session = sessions.find(s => s.sessionId === challenge.sessionId);
        assert(session, "session is unset");
        const challengeDef = challengeDefs.find(c => {
            return c.accountVersion === session.accountVersion
            && c.challengeId === challenge.challengeId
        });
        assert(challengeDef, "challengeDef is unset");
        packs.push({
            challenge,
            session,
            challengeDef
        });
    }
    {
        const challenge = challenges.find(c => c.outcome === EAuthOutcome.Pending);
        assert(challenge, "challenge is unset");
        const session = sessions.find(s => s.sessionId === challenge.sessionId);
        assert(session, "session is unset");
        assert(session.outcome === EAuthOutcome.Pending, "sesson is not pending");
        const challengeDef = challengeDefs.find(c => {
            return c.accountVersion === session.accountVersion
            && c.challengeId === challenge.challengeId
        });
        assert(challengeDef, "challengeDef is unset");
        packs.push({
            challenge,
            session,
            challengeDef
        });
    }

    return [
        `INSERT INTO berytus_account_auth_challenge_message
        (SessionID, ChallengeID, MessageName, Request,
        Expected, Response, StatusMsg)
        VALUES ('${packs[0].challenge.sessionId}',
                '${packs[0].challenge.challengeId}',
                'Dummy',
                '{ "magicWord": "123" }', '{ "proof": "456" }',
                '{ "proof": "456" }', 'Ok')`,
        `INSERT INTO berytus_account_auth_challenge_message
        (SessionID, ChallengeID, MessageName, Request,
        Expected, Response, StatusMsg)
        VALUES ('${packs[1].challenge.sessionId}',
                '${packs[1].challenge.challengeId}',
                'Dummy',
                '{ "magicWord": "123" }', '{ "proof": "456" }',
                'null', null)`,

    ];
}

let cachedAuthChallengeMessages: Array<{
    sessionId: BigInt;
    challengeId: string;
    messageName: string;
    request: unknown;
    expected: unknown;
    response: unknown;
    statusMsg: string
}> | null = null;

export const createAuthChallengeMessages = async () => {
    return useConnection(async conn => {
        const sessions = await getAuthSessions();
        const challengeDefs = await getAccountChallengeDefs();
        const challenges = await getAuthChallenges();
        const stmts = getStatements(
            challengeDefs,
            sessions,
            challenges
        );
        assert(stmts.length > 0);
        for (let i = 0; i < stmts.length; i++) {
            await conn.unsafe(stmts[i]);
        }
        const rows = await conn`
            SELECT SessionID, ChallengeID, MessageName,
                   Request, Expected, Response, StatusMsg
            FROM berytus_account_auth_challenge_message
        `;
        assert(rows.length > 0);
        cachedAuthChallengeMessages = rows.map(({
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
            assert(request !== undefined);
            assert(expected !== undefined);
            assert(response !== undefined);
            assert(typeof statusMsg === 'string' || statusMsg === null);
            assert(statusMsg ? statusMsg.match(/^(Ok|Error:.*)$/) : true);
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
        return [...cachedAuthChallengeMessages];
    });
}

export const getAuthChallengeMessages = async () => {
    if (cachedAuthChallengeMessages) {
        return cachedAuthChallengeMessages;
    }
    const result = await createAuthChallengeMessages();
    assert(result.length > 0);
    return result;
}