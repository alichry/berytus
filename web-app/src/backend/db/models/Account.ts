import { toPostgresBigInt, useConnection, useTransaction } from '../pool.js';
import type { PoolConnection } from '../pool.js';
import type { FieldInput } from '../types.js';
import { AccountDefKeyFieldList } from './AccountDefKeyFieldList.js';
import { EntityAlreadyExistsError } from '../errors/EntityAlreadyExistsError.js';
import { EntityNotFoundError } from '../errors/EntityNotFoundError.js';

interface PCreateAccount {
    accountid: BigInt;
}

interface PConflictCheck {
    accountid: BigInt;
    accountkeyfieldmatchcount: number;
}

interface PGetLatest {
    latestaccountversion: number;
}

export class Account {
    accountId: BigInt;
    accountVersion: number;

    protected constructor(accountId: BigInt, accountVersion: number) {
        this.accountId = accountId;
        this.accountVersion = accountVersion;
    }

    static async latest(
        accountId: BigInt,
        existingConn?: PoolConnection
    ): Promise<Account> {
        if (existingConn) {
            return Account.#latest(existingConn, accountId);
        }
        return useConnection((conn) =>
            Account.#latest(
                conn,
                accountId
            )
        );
    }

    static async #latest(conn: PoolConnection, accountId: BigInt): Promise<Account> {
        const rows = await conn<PGetLatest[]>`
            SELECT MAX(AccountVersion) as LatestAccountVersion
            FROM berytus_account_field
            WHERE AccountID = ${toPostgresBigInt(accountId)}
            ORDER BY MAX(AccountVersion) DESC
            LIMIT 1
        `;
        if (rows.length === 0) {
            throw EntityNotFoundError.default(
                'Account',
                String(accountId),
                'AccountID',
                'Cannot find latest account version'
            );
        }
        return new Account(
            accountId,
            rows[0].latestaccountversion
        );
    }

    static async accountExists(
        accountVersion: number,
        fieldInputs: FieldInput[]
    ): Promise<boolean> {
        return null !== await this.getAccount(accountVersion, fieldInputs);
    }

    static async getAccount(
        accountVersion: number,
        fieldInputs: FieldInput[]
    ): Promise<Account | null> {
        return useConnection(async (conn) => {
            return this.#getAccount(
                conn,
                accountVersion,
                fieldInputs
            );
        });
    }

    static async #createAccountId(
        conn: PoolConnection
    ): Promise<BigInt> {
        const [row] = await conn<PCreateAccount[]>
            `INSERT INTO berytus_account DEFAULT VALUES RETURNING AccountID`;
        return row.accountid;
    }

    static async createAccount(
        accountVersion: number,
        fieldInputs: FieldInput[]
    ): Promise<Account> {
        return useTransaction(async (conn) => {
            await conn`SET TRANSACTION ISOLATION LEVEL SERIALIZABLE`;
            const existingAccount = await this.#getAccount(
                conn,
                accountVersion,
                fieldInputs
            );
            if (existingAccount) {
                throw EntityAlreadyExistsError.default(
                    Account.name,
                    "[...[AccountVersion, FieldID, FieldValue], ...[AccountVersion, FieldID, FieldValue]], ..",
                    "STRIPPED",
                    "Cannot create an account. Passed identity fields are reserved."
                );
            }
            const newAccountId = await this.#createAccountId(conn);
            await conn
                `INSERT INTO berytus_account_field
                ${conn(fieldInputs.map(field => ({
                    accountid: toPostgresBigInt(newAccountId),
                    accountversion: accountVersion,
                    fieldid: field.id,
                    fieldvalue: conn.json(field.value)
                })))}`;
            return new Account(
                newAccountId,
                accountVersion
            );
        });
    }

    /**
     * Given an array of field inputs, filter the key (identity)
     * fields from it and use it in combination with an account version
     * to locate the corresponding account id.
     *
     * One solution is to find all account key field records that would match
     * the key field input (AccountVersion, FieldID, FieldValue) tuples, and
     * check if the number of records found, groupped by AccountID, equals to
     * the number of account key fields, e.g.:
     *
     * Account def key fields for AccountVersion 2000:
     * SELECT * FROM berytus_account_def_key_field_id WHERE AccountVersion = 2000;
     * +----------------+----------+
     * | AccountVersion | FieldID  |
     * +----------------+----------+
     * |           2000 | partyId  |
     * |           2000 | username |
     * +----------------+----------+
     *
     * AccountID 21 and AccountID 22 account fields:
     * SELECT * FROM berytus_account_field WHERE AccountID = 21 OR AccountID = 22;
     * +-----------+----------------+----------------+-------------------+
     * | AccountID | AccountVersion | FieldID        | FieldValue        |
     * +-----------+----------------+----------------+-------------------+
     * |        21 |           2000 | partyId        | "jerry-and-sons"  |
     * |        21 |           2000 | securePassword | "securePassJerry" |
     * |        21 |           2000 | username       | "jerry"           |
     * |        22 |           2000 | partyId        | "jerry-and-sons"  |
     * |        22 |           2000 | securePassword | "securePassCaleb" |
     * |        22 |           2000 | username       | "caleb"           |
     * +-----------+----------------+----------------+-------------------+
     *
     *
     * We can run the following query to get the count.
     *
     * SELECT AccountID, COUNT(*) AS AccountKeyFieldMatchCount
     * FROM berytus_account_field WHERE AccountVersion = 2000
     * AND (
     *  (FieldID = 'partyId' AND FieldValue = '"jerry-and-sons"')
     *  OR (FieldID = 'username' AND FieldValue = '"jerry"')
     * )  GROUP BY AccountID;
     * +-----------+------------------------------+
     * | AccountID | AccountKeyFieldMatchCount    |
     * +-----------+------------------------------+
     * |        21 |                            2 |
     * |        22 |                            1 |
     * +-----------+------------------------------+
     *
     * AccountID 21 has a match count of 2 which is equal to the count
     * of account key fields for version = 2000. Thus, we can assert
     * that this input list of (FieldID, FieldValue) tuples points to
     * AccountID 21.
     *
     * Here, we filter an array of field inputs, i.e. key or non-key
     * field inputs, to an array of key field inputs before running
     * the key field match count query.
     */
    static async #getAccount(
        conn: PoolConnection,
        accountVersion: number,
        fieldInputs: FieldInput[]
    ) {
        const keyFieldList = await AccountDefKeyFieldList.fetchList(accountVersion, conn);
        if (! keyFieldList.doFieldInputsContainAllKeyFields(fieldInputs)) {
            throw new Error('Malformed account field inputs');
        }
        const keyFieldInputs = keyFieldList.pickKeyFieldInputsFrom(fieldInputs);
        const rows = await conn<PConflictCheck[]>`
            SELECT AccountID,
                    COUNT(*) AS AccountKeyFieldMatchCount
            FROM berytus_account_field
            WHERE AccountVersion = ${keyFieldList.accountVersion}
            AND (FieldID, FieldValue) IN
                ${conn(keyFieldInputs.map(
                    ({ id, value }) =>
                        conn([
                            id,
                            conn.json(value)
                        ])
                ))}
            GROUP BY AccountID
            ORDER BY AccountKeyFieldMatchCount DESC
        `;
        if (
            rows.length === 0 ||
            rows[0].accountkeyfieldmatchcount < keyFieldList.fields.length
        ) {
            return null;
        }
        const accountId = rows[0].accountid;
        return new Account(accountId, accountVersion);
    }
}