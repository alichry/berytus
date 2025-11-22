import { useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";

interface PGetFieldTypeNOptions {
    fieldtype: string;
    fieldoptions: unknown;
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
    readonly fieldOptions: unknown

    protected constructor(
        accountVersion: number,
        fieldId: string,
        fieldType: string,
        fieldOptions: unknown
    ) {
        this.accountVersion = accountVersion;
        this.fieldId = fieldId;
        this.fieldType = fieldType;
        this.fieldOptions = fieldOptions;
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
        const res = await conn<PGetFieldTypeNOptions[]>`
            SELECT FieldType, FieldOptions
            FROM berytus_account_def_field
            WHERE AccountVersion = ${accountVersion}
            AND FieldID = ${fieldId}
        `;
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
            res[0].fieldtype,
            res[0].fieldoptions
        );
    }
}