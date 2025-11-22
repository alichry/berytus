import type { PoolConnection } from "../pool.js";
import { toPostgresBigInt, useConnection } from "../pool.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";

export interface PGetFieldValue {
    fieldvalue: unknown; /* JSON-parsed value */
}

export class AccountField {
    readonly accountVersion: number;
    readonly accountId: BigInt;
    readonly fieldId: string;
    readonly fieldValue: unknown;

    protected constructor(
        accountVersion: number,
        accountId: BigInt,
        fieldId: string,
        fieldValue: unknown,
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
            FROM berytus_account_field
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
}