import type { PoolConnection } from "mysql2/promise";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { useConnection } from "../pool.js";
import { AccountDefKeyField, type PAccountDefKeyField } from "./AccountDefKeyField.js"
import type { FieldInput } from "../types.js";

interface KeyFieldInput extends FieldInput {
    isKey: true;
}

export class AccountDefKeyFieldList {
    public readonly accountVersion: number;
    public readonly fields: AccountDefKeyField[];

    protected constructor(accountVersion: number, fields: AccountDefKeyField[]) {
        this.accountVersion = accountVersion;
        this.fields = fields;
    }

    doFieldInputsContainAllKeyFields(fieldInputs: FieldInput[]): boolean {
        for (let i = 0; i < this.fields.length; i++) {
            const { fieldId } = this.fields[i];
            const fieldInput = fieldInputs.find(f => f.id === fieldId);
            if (! fieldInput) {
                return false;
            }
        }
        return true;
    }

    pickKeyFieldInputsFrom(fieldInputs: FieldInput[]): KeyFieldInput[] {
        const keyFieldInputs: Array<KeyFieldInput> = [];
        this.fields.forEach(({ fieldId }) => {
            const fieldInput = fieldInputs.find(f => f.id === fieldId);
            if (! fieldInput) {
                throw new Error(
                    "Unable to filter field inputs to key field inputs. " +
                    "Missing field id '" + fieldId + "' in field inputs."
                );
            }
            keyFieldInputs.push({ ...fieldInput, isKey: true });
        });
        return keyFieldInputs;
    }

    static async fetchList(
        accountVersion: number,
        existingConnection?: PoolConnection
    ): Promise<AccountDefKeyFieldList> {
        if (existingConnection) {
            return AccountDefKeyFieldList.#fetchList(
                existingConnection,
                accountVersion
            );
        }
        return useConnection((conn) => {
            return AccountDefKeyFieldList.#fetchList(
                conn,
                accountVersion
            );
        })
    }

    static async #fetchList(
        conn: PoolConnection,
        accountVersion: number
    ): Promise<AccountDefKeyFieldList> {
        const [rows] = await conn.query<PAccountDefKeyField[]>(
            'SELECT FieldID FROM berytus_account_def_key_field_id ' +
            'WHERE AccountVersion = ?',
            [accountVersion]
        );
        if (rows.length === 0) {
            throw EntityNotFoundError.default(
                AccountDefKeyField.name,
                accountVersion,
                "AccountVersion",
                "Unable to retrieve account key fields."
            )
        }
        const keyFields = rows.map(r => new AccountDefKeyField(r.FieldID));
        return new AccountDefKeyFieldList(accountVersion, keyFields);
    }
}