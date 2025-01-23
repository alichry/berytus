import type { ResultSetHeader, RowDataPacket } from 'mysql2';
import type { PoolConnection} from 'mysql2/promise';
import { useConnection } from '../pool.js';
import type { FieldInput } from '../types.js';
import { AccountDefKeyFieldList } from './AccountDefKeyFieldList.js';
import { EntityAlreadyExistsError } from '../errors/EntityAlreadyExistsError.js';

// interface PBerytusAccountField extends RowDataPacket {
//     AccountID: string;
//     FieldID: string;
// }

interface PConflictCheck extends RowDataPacket {
    AccountID: number;
    AccountKeyFieldMatchCount: number;
}

export class Account {
    accountId: number;

    protected constructor(accountId: number) {
        this.accountId = accountId;
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
    ): Promise<number> {
        const [res] = await conn.query<ResultSetHeader>(
            'INSERT INTO berytus_account () VALUES ()'
        );
        return res.insertId;
    }

    static async createAccount(
        accountVersion: number,
        fieldInputs: FieldInput[]
    ): Promise<Account> {
        return useConnection(async (conn) => {
            await conn.query('SET TRANSACTION ISOLATION LEVEL SERIALIZABLE');
            await conn.beginTransaction();
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
            const records: Array<Array<number | string>> = [];
            fieldInputs.forEach(field => {
                records.push(
                    [
                        newAccountId,
                        accountVersion,
                        field.id,
                        JSON.stringify(field.value)
                    ]
                );
            })
            await conn.query(
                `INSERT INTO berytus_account_field
                (AccountID, AccountVersion, FieldID, FieldValue)
                VALUES ?`,
                [records]
            );
            await conn.commit();
            return new Account(
                newAccountId
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
     *  (FieldID = "partyId" AND FieldValue = '"jerry-and-sons"')
     *  OR (FieldID = "username" AND FieldValue = '"jerry"')
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

        const filterStatements: Array<string> = [];
        const filterValues: Array<string> = [];
        keyFieldInputs.forEach(({ id, value }) => {
            filterStatements.push(
                `(FieldID = ? AND FieldValue = ?)`
            );
            filterValues.push(
                id, JSON.stringify(value) // values are stored in JSON format
            );
        })

        const [rows] = await conn.query<PConflictCheck[]>(
            `SELECT AccountID,
                    COUNT(*) AS AccountKeyFieldMatchCount
            FROM berytus_account_field
            WHERE AccountVersion = ? AND (${filterStatements.join(' OR ')})
            GROUP BY AccountID
            ORDER BY AccountKeyFieldMatchCount DESC`,
            [keyFieldList.accountVersion, ...filterValues]
        );
        if (
            rows.length === 0 ||
            rows[0].AccountKeyFieldMatchCount < keyFieldList.fields.length
        ) {
            return null;
        }
        const accountId = rows[0].AccountID;
        return new Account(accountId);
    }
}