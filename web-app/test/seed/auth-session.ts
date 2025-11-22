import { useConnection } from "../../src/backend/db/pool.js"
import { strict as assert } from 'node:assert';
import { getAccounts, type Accounts } from "./account.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { getAccountChallengeDefs, type ChallengeDefs } from "./account-challenge-defs.js";

const getStatements = (challengeDefs: ChallengeDefs, accounts: Accounts) => {
    const challengeDef = challengeDefs[0];
    assert(challengeDef);
    const account = accounts.find(
        c => c.accountVersion === challengeDef.accountVersion
    );
    assert(account);
    return [
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${account.accountId}, ${account.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${account.accountId}, ${account.accountVersion}, 'Aborted')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${account.accountId}, ${account.accountVersion}, 'Succeeded')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${account.accountId}, ${account.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${account.accountId}, ${account.accountVersion}, 'Pending')`,
    ];
}

let cachedAuthSessions: Array<{
    sessionId: BigInt,
    accountId: BigInt,
    accountVersion: number,
    outcome: EAuthOutcome
}> | null = null;

export const createAuthSessions = async () => {
    return useConnection(async conn => {
        const accounts = await getAccounts();
        const challengeDefs = await getAccountChallengeDefs();
        const stmts = getStatements(challengeDefs, accounts);
        assert(stmts.length > 0);
        for (let i = 0; i < stmts.length; i++) {
            await conn.unsafe(stmts[i]);
        }
        const inserted = await conn`
            SELECT SessionID, AccountID, AccountVersion, Outcome
            FROM   berytus_account_auth_session
            ORDER BY CreatedAt ASC
        `;
        assert(inserted.length > 0);
        const createdSessions = [];
        for (const row of inserted) {
            assert(typeof row.sessionid === 'bigint');
            assert(row.sessionid > 0);
            assert(typeof row.accountid === 'bigint');
            assert(row.accountid > 0);
            assert(typeof row.accountversion === 'number');
            assert(row.accountversion > 0);
            assert(typeof row.outcome === 'string');
            assert(row.outcome in EAuthOutcome);
            createdSessions.push({
                sessionId: row.sessionid,
                accountId: row.accountid,
                accountVersion: row.accountversion,
                outcome: (row.outcome as EAuthOutcome)
            });
        }
        cachedAuthSessions = createdSessions;
        return [...createdSessions];
    });
}

export const getAuthSessions = async () => {
    if (cachedAuthSessions) {
        return cachedAuthSessions;
    }
    const sessions = await createAuthSessions();
    assert(sessions.length > 0);
    return sessions;
}

export type Sessions = Awaited<ReturnType<typeof getAuthSessions>>;