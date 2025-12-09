import { withSearchPath } from "@test/with-search-path.js";
import type { PoolConnection } from "../../src/backend/db/pool.js"
import { strict as assert } from 'node:assert';

const getStatements = (accountIds: BigInt[]) => {
    assert(accountIds.length >= 7);
    return [
        `INSERT INTO berytus_account_field
        (AccountID, AccountVersion, FieldID, FieldValue)
        VALUES
        (${accountIds[0]}, 1, 'username', '"bob123"'),
        (${accountIds[0]}, 1, 'password', '{"salt":"c2beaeba21c92df08e0e75190d8fe318","hash":"1cbbf7d455c7aee52dc1f05af7bc61ac5e7d10c60deaff6e1dba80a48db3f475b58c96f292542903fa08c0d6976085e796d9b7e9dd469a6b544e50841e7ee153"}'),

        (${accountIds[0]}, 2, 'username', '"bob123"'),
        (${accountIds[0]}, 2, 'securePassword', '"securePassBob"'),

        (${accountIds[1]}, 1, 'username', '"john123"'),
        (${accountIds[1]}, 1, 'password', '{"salt":"c2beaeba21c92df08e0e75190d8fe318","hash":"1cbbf7d455c7aee52dc1f05af7bc61ac5e7d10c60deaff6e1dba80a48db3f475b58c96f292542903fa08c0d6976085e796d9b7e9dd469a6b544e50841e7ee153"}'),

        (${accountIds[1]}, 2, 'username', '"john123"'),
        (${accountIds[1]}, 2, 'securePassword', '"securePassJohn"'),

        (${accountIds[2]}, 3, 'username', '"john123"'),
        (${accountIds[2]}, 3, 'key', '{ "publicKey": "-----BEGIN PUBLIC KEY-----\\ndummydata\\n-----END PUBLIC KEY-----" }'),

        (${accountIds[3]}, 2000, 'partyId', '"jerry-and-sons"'),
        (${accountIds[3]}, 2000, 'username', '"jerry"'),
        (${accountIds[3]}, 2000, 'securePassword', '"securePassJerry"'),

        (${accountIds[4]}, 2000, 'partyId', '"jerry-and-sons"'),
        (${accountIds[4]}, 2000, 'username', '"caleb"'),
        (${accountIds[4]}, 2000, 'securePassword', '"securePassCaleb"'),

        (${accountIds[5]}, 2000, 'partyId', '"marks-and-sons"'),
        (${accountIds[5]}, 2000, 'username', '"mark"'),
        (${accountIds[5]}, 2000, 'securePassword', '"securePassMark"'),

        (${accountIds[6]}, 2000, 'partyId', '"mark-and-sons"'),
        (${accountIds[6]}, 2000, 'username', '"matty"'),
        (${accountIds[6]}, 2000, 'securePassword', '"securePassMatty"')`,

        `INSERT INTO berytus_account_user_attributes
        (AccountID, UserAttributeDictionary)
        VALUES
        (${accountIds[0]}, '{ "name": "John", "gender": "Male" }'),
        (${accountIds[1]}, '{ "name": "Jane", "gender": "Female" }')`,
    ]
}

let cachedAccountIds: BigInt[] | null = null;

const createAccountIds = async (conn: PoolConnection, count: number): Promise<BigInt[]> => {
    const rows = [];
    for (let i = 0; i < count; i++) {
        const result = await conn`
            INSERT INTO berytus_account DEFAULT VALUES RETURNING AccountID
        `;
        assert(typeof result[0].accountid === 'bigint');
        rows.push(result[0].accountid);
    }
    cachedAccountIds = rows
    return [...rows];
}

export const getAccountIds = async (
    conn?: PoolConnection
): Promise<BigInt[]> => {
    if (cachedAccountIds) {
        return [...cachedAccountIds];
    }
    assert(conn);
    const accountIds = await createAccountIds(conn, 6);
    assert(accountIds.length >= 6);
    return accountIds;
}

let cachedAccounts: Array<{
    accountId: BigInt,
    accountVersion: number
}> | null = null;

export const createAccounts = async () => {
    return withSearchPath(async conn => {
        const accountIds = await createAccountIds(conn, 7);
        const stmts = getStatements(accountIds);
        assert(stmts.length > 0);
        for (let i = 0; i < stmts.length; i++) {
            try {
                await conn.unsafe(stmts[i]);
            } catch (e) {
                console.error(`Failed to execute: ${stmts[i]}`);
                throw e;
            }
        }
        const inserted = await conn`
            SELECT DISTINCT AccountID, AccountVersion, CreatedAt
            FROM berytus_account_field
            ORDER BY CreatedAt ASC, AccountID ASC
        `;
        assert(inserted.length > 0);
        const createdAccounts = [];
        for (const row of inserted) {
            assert(typeof row.accountid === 'bigint');
            assert(row.accountid > 0);
            assert(typeof row.accountversion === 'number');
            assert(row.accountversion > 0);
            createdAccounts.push({
                accountId: row.accountid,
                accountVersion: row.accountversion
            });
        }
        cachedAccounts = createdAccounts;
        return [...cachedAccounts];
    });
}

export const getAccounts = async () => {
    if (cachedAccounts) {
        return cachedAccounts;
    }
    const accounts = await createAccounts();
    assert(accounts.length > 0);
    return accounts;
}

export type Accounts = Awaited<ReturnType<typeof getAccounts>>;