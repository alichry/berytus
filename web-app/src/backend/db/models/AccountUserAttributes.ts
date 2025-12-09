import { useConnection, toPostgresBigInt, table } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import type { JSONValue } from "../types.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";

export interface PGetUserAttributeDictionary {
    userattributedictionary: Record<string, JSONValue>; /* JSON-parsed value */
}

export class AccountUserAttributes {
    accountId: BigInt;
    userAttributes: Record<string, JSONValue>;

    constructor(accountId: BigInt, userAttrs: Record<string, JSONValue>) {
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
        accountId: BigInt,
        attrs: Record<string, JSONValue>,
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
        accountId: BigInt,
        attrs: Record<string, JSONValue>
    ) {
        await conn`
            INSERT INTO ${table('berytus_account_user_attributes')}
            (AccountID, UserAttributeDictionary)
            VALUES (${toPostgresBigInt(accountId)}, ${conn.json(attrs)})
        `;
        return new AccountUserAttributes(
            accountId,
            attrs
        );
    }

    static async getUserAttributes(
        accountId: BigInt,
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
        accountId: BigInt
    ): Promise<AccountUserAttributes> {
        const res = await conn<PGetUserAttributeDictionary[]>`
            SELECT UserAttributeDictionary
            FROM ${table('berytus_account_user_attributes')}
            WHERE AccountID = ${toPostgresBigInt(accountId)}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountUserAttributes.name,
                String(accountId),
                "AccountID"
            );
        }
        return new AccountUserAttributes(
            accountId,
            res[0].userattributedictionary
        );
    }
}