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
    idle_timeout: 0.1,
    types: {
        bigint: postgres.BigInt
    },
    prepare: false,
});

type TableName =
    | "berytus_account"
    | "berytus_account_user_attributes"
    | "berytus_account_def"
    | "berytus_account_def_category"
    | "berytus_account_def_category_version"
    | "berytus_account_def_field"
    | "berytus_account_def_key_field_id"
    | "berytus_account_field"
    | "berytus_account_def_auth_challenge"
    | "berytus_account_auth_session"
    | "berytus_account_auth_challenge"
    | "berytus_account_auth_challenge_message"

export const table = (name: TableName) => {
    return pool(`${databaseSchema}.${name}`);
}

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
