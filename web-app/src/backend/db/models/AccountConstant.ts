import { table, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";

interface PGetAccountConstant {
    value: string;
}

export class AccountConstant {
    readonly accountVersion: number;
    readonly constantName: string;
    readonly value: string;

    protected constructor(
        accountVersion: number,
        constantName: string,
        value: string
    ) {
        this.accountVersion = accountVersion;
        this.constantName = constantName;
        this.value = value;
    }

    static async getConstant(
        accountVersion: number,
        constantName: string,
        existingConn?: PoolConnection
    ): Promise<AccountConstant> {
        if (existingConn) {
            return AccountConstant.#getConstant(
                existingConn,
                accountVersion,
                constantName
            );
        }
        return useConnection((conn) =>
            AccountConstant.#getConstant(conn, accountVersion, constantName)
        );
    }

    static async #getConstant(
        conn: PoolConnection,
        accountVersion: number,
        constantName: string
    ): Promise<AccountConstant> {
        const res = await conn<PGetAccountConstant[]>`
            SELECT Value
            FROM ${table('berytus_account_constant')}
            WHERE AccountVersion = ${accountVersion}
            AND ConstantName = ${constantName}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AccountConstant.name,
                `${accountVersion},${constantName}`,
                `AccountVersion,ConstantName`
            );
        }
        return new AccountConstant(accountVersion, constantName, res[0].value);
    }

    static async createConstant(
        accountVersion: number,
        constantName: string,
        value: string,
        existingConn?: PoolConnection
    ): Promise<AccountConstant> {
        if (existingConn) {
            return AccountConstant.#createConstant(
                existingConn,
                accountVersion,
                constantName,
                value
            );
        }
        return useConnection((conn) =>
            AccountConstant.#createConstant(conn, accountVersion, constantName, value)
        );
    }

    static async #createConstant(
        conn: PoolConnection,
        accountVersion: number,
        constantName: string,
        value: string
    ): Promise<AccountConstant> {
        await conn`
            INSERT INTO ${table('berytus_account_constant')}
            (AccountVersion, ConstantName, Value)
            VALUES (${accountVersion}, ${constantName}, ${value})
        `;
        return new AccountConstant(accountVersion, constantName, value);
    }
}