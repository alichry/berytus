import type { PoolConnection, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";
import type { JSONString } from "./types";

export interface PGetUserAttributeDictionary extends RowDataPacket {
    UserAttributeDictionary: JSONString;
}

export class AccountUserAttributes {
    accountId: number;
    userAttributes: Record<string, unknown>;

    constructor(accountId: number, userAttrs: Record<string, unknown>) {
        this.accountId = accountId;
        this.userAttributes = userAttrs;
    }

    asArray() {
        const arr: { id: string; value: unknown; }[] = [];
        Object.keys(this.userAttributes).forEach(key => {
            arr.push({ id: key, value: this.userAttributes[key] });
        });
        return arr;
    }

    static async createUserAttributes(
        accountId: number,
        attrs: Record<string, unknown>,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AccountUserAttributes.#createUserAttributes(
                existingConn,
                accountId,
                attrs
            );
        }
        return useConnection(conn =>
            AccountUserAttributes.#createUserAttributes(
                conn,
                accountId,
                attrs
            )
        );
    }

    static async #createUserAttributes(
        conn: PoolConnection,
        accountId: number,
        attrs: Record<string, unknown>
    ) {
        await conn.query(
            'INSERT INTO berytus_account_user_attributes ' +
            '(AccountID, UserAttributeDictionary) VALUES (?, ?)',
            [accountId, JSON.stringify(attrs)]
        );
        return new AccountUserAttributes(
            accountId,
            attrs
        );
    }

    static async getUserAttributes(
        accountId: number,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AccountUserAttributes.#getUserAttributes(
                existingConn,
                accountId
            );
        }
        return useConnection((conn) =>
            AccountUserAttributes.#getUserAttributes(
                conn,
                accountId
            )
        );
    }

    static async #getUserAttributes(
        conn: PoolConnection,
        accountId: number
    ): Promise<AccountUserAttributes> {
        const [res] = await conn.query<PGetUserAttributeDictionary[]>(
            'SELECT UserAttributeDictionary FROM berytus_account_user_attributes ' +
            'WHERE AccountID = ?',
            [accountId]
        );
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountUserAttributes.name,
                accountId,
                "AccountID"
            );
        }
        return new AccountUserAttributes(
            accountId,
            JSON.parse(res[0].UserAttributeDictionary)
        );
    }
}