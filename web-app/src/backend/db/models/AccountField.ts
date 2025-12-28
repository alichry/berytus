import type { PoolConnection } from "../pool.js";
import { table, toPostgresBigInt, useConnection } from "../pool.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import type { JSONValue } from "../types.js";
import { IllegalDatabaseStateError } from "../errors/IllegalDatabaseStateError.js";

export interface PGetFieldValue {
    fieldvalue: JSONValue;
}

export class AccountField {
    readonly accountVersion: number;
    readonly accountId: BigInt;
    readonly fieldId: string;
    fieldValue: JSONValue;

    protected constructor(
        accountVersion: number,
        accountId: BigInt,
        fieldId: string,
        fieldValue: JSONValue,
    ) {
        this.accountVersion = accountVersion;
        this.accountId = accountId;
        this.fieldId = fieldId;
        this.fieldValue = fieldValue;
    }

    static async getField(
        accountVersion: number,
        accountId: BigInt,
        fieldId: string,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AccountField.#getField(
                existingConn,
                accountVersion,
                accountId,
                fieldId,
            );
        }
        return useConnection(async (conn) =>
            AccountField.#getField(
                conn,
                accountVersion,
                accountId,
                fieldId
            )
        );
    }

    static async #getField(
        conn: PoolConnection,
        accountVersion: number,
        accountId: BigInt,
        fieldId: string,
    ) {
        const res = await conn<PGetFieldValue[]>`
            SELECT FieldValue
            FROM ${table('berytus_account_field')}
            WHERE AccountVersion = ${accountVersion}
            AND AccountID = ${toPostgresBigInt(accountId)}
            AND FieldID = ${fieldId}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountField.name,
                `${accountVersion},${accountId},${fieldId}`,
                `AccountVersion,AccountID,FieldID`
            );
        }
        return new AccountField(
            accountVersion,
            accountId,
            fieldId,
            res[0].fieldvalue
        );
    }

    async updateValue(
        fieldValue: JSONValue,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return this.#updateValue(existingConn, fieldValue);
        }
        return useConnection(
            conn => this.#updateValue(conn, fieldValue)
        );
    }

    async #updateValue(
        conn: PoolConnection,
        fieldValue: JSONValue
    ) {
        const res = await conn`
            UPDATE ${table('berytus_account_field')}
            SET FieldValue = ${conn.json(fieldValue)}
            WHERE AccountVersion = ${this.accountVersion}
            AND AccountID = ${toPostgresBigInt(this.accountId)}
            AND FieldID = ${this.fieldId}
        `;
        if (res.count === 0) {
            throw new IllegalDatabaseStateError(
                "Failed to update field value"
            );
        }
        this.fieldValue = fieldValue;
    }
}