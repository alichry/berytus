import type { PoolConnection, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";

export interface PGetFieldValue extends RowDataPacket {
    FieldValue: string; /* JSON */
}

export class AccountField {
    readonly accountVersion: number;
    readonly accountId: number;
    readonly fieldId: string;
    readonly fieldValue: unknown;

    protected constructor(
        accountVersion: number,
        accountId: number,
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
        accountId: number,
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
        accountId: number,
        fieldId: string,
    ) {
        const [res] = await conn.query<PGetFieldValue[]>(
            'SELECT FieldValue ' +
            'FROM berytus_account_field ' +
            'WHERE AccountVersion = ? AND AccountID = ? AND FieldID = ?',
            [accountVersion, accountId, fieldId]
        );
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
            JSON.parse(res[0].FieldValue)
        );
    }
}