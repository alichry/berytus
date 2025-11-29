import { useConnection } from "../../src/backend/db/pool.js"
import { strict as assert } from 'node:assert';
import { getAccounts, type Accounts } from "./account.js";
import { EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { getAccountChallengeDefs, type ChallengeDefs } from "./account-challenge-defs.js";
import { EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";

const getStatements = (challengeDefs: ChallengeDefs, accounts: Accounts) => {
    const passwordChallengeDef = challengeDefs.find(
        d => d.challengeType === EChallengeType.Password
    );
    assert(passwordChallengeDef);
    const srpChallengeDef = challengeDefs.find(
        d => d.challengeType === EChallengeType.SecureRemotePassword
    );
    assert(srpChallengeDef);
    const accountForPassAuth = accounts.find(
        a => a.accountVersion === passwordChallengeDef.accountVersion
    );
    assert(accountForPassAuth);
    const accountForSrpAuth = accounts.find(
        a => a.accountVersion === srpChallengeDef.accountVersion
    );
    assert(accountForSrpAuth);
    return [
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Aborted')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Succeeded')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Aborted')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Aborted')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForPassAuth.accountId}, ${accountForPassAuth.accountVersion}, 'Aborted')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForSrpAuth.accountId}, ${accountForSrpAuth.accountVersion}, 'Pending')`,
        `INSERT INTO berytus_account_auth_session
        (AccountID, AccountVersion, Outcome)
        VALUES
        (${accountForSrpAuth.accountId}, ${accountForSrpAuth.accountVersion}, 'Pending')`,
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