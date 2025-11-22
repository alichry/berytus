import {
    databaseHost,
    databaseName,
    databasePort,
    databaseUser,
    databasePassword,
    databaseSchema
} from "../env/db.ts";
import postgres from 'postgres'

export const pool = postgres({
    host: databaseHost,
    port: databasePort,
    user: databaseUser,
    database: databaseName,
    password: databasePassword,
    connection: {
        search_path: databaseSchema
    },
    idle_timeout: 0.1,
    types: {
        bigint: postgres.BigInt
    }
});

export type Types = typeof pool extends postgres.Sql<infer U>
    ? U : never;
export type ReservedConnection = postgres.ReservedSql<Types>;
export type PoolConnection = postgres.Sql<Types>;
export type TransactionConnection = postgres.TransactionSql<Types>;


export const useConnection = async <T extends unknown>(
    callback: (conn: PoolConnection) => Promise<T>
): Promise<T> => {
    return callback(pool);
}

export const useReservedConnection = async <T extends unknown>(
    callback: (conn: ReservedConnection) => Promise<T>
): Promise<T> => {
    const conn = await pool.reserve();
    return callback(conn)
        .then((res) => {
            conn.release();
            return res;
        })
        .catch((err) => {
            conn.release();
            throw err;
        });
};

export const useTransaction = async <T extends unknown>(
    callback: (conn: TransactionConnection) => Promise<T>
) => {
    // begin reserves a connection
    return pool.begin(callback);
};

export const toPostgresBigInt = (value: BigInt) => {
    return postgres.BigInt.parse(value);
}