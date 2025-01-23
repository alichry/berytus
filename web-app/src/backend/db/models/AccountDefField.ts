import type { PoolConnection, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";

export interface PGetFieldTypeNOptions extends RowDataPacket {
    FieldType: string;
    FieldOptions: unknown;
}

export enum EFieldType {
    Identity = 'Identity',
    ForeignIdentity = 'ForeignIdentity',
    Password = 'Password',
    SecurePassword = 'SecurePassword',
    ConsumablePassword = 'ConsumablePassword',
    Key = 'Key',
    SharedKey = 'SharedKey'
}

export class AccountDefField {
    readonly accountVersion: number;
    readonly fieldId: string;
    readonly fieldType: string;
    readonly fieldOpttions: unknown

    protected constructor(
        accountVersion: number,
        fieldId: string,
        fieldType: string,
        fieldOptions: unknown
    ) {
        this.accountVersion = accountVersion;
        this.fieldId = fieldId;
        this.fieldType = fieldType;
        this.fieldOpttions = fieldOptions;
    }

    static async getField(
        accountVersion: number,
        fieldId: string,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AccountDefField.#getField(
                existingConn,
                accountVersion,
                fieldId,
            );
        }
        return useConnection(async (conn) =>
            AccountDefField.#getField(
                conn,
                accountVersion,
                fieldId
            )
        );
    }

    static async #getField(
        conn: PoolConnection,
        accountVersion: number,
        fieldId: string,
    ) {
        const [res] = await conn.query<PGetFieldTypeNOptions[]>(
            'SELECT FieldType, FieldOptions ' +
            'FROM berytus_account_def_field ' +
            'WHERE AccountVersion = ? AND FieldID = ?',
            [accountVersion, fieldId]
        );
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountDefField.name,
                `${accountVersion},${fieldId}`,
                `AccountVersion,FieldID`
            );
        }
        return new AccountDefField(
            accountVersion,
            fieldId,
            res[0].FieldType,
            res[0].FieldOptions
        );
    }
}