import type { PoolConnection } from "mysql2/promise";
import {
    databaseHost,
    databaseName,
    databasePort,
    databaseUser,
    databasePassword
} from "../env/db.ts";
import * as mysql from 'mysql2/promise';

export const pool = mysql.createPool({
    host: databaseHost,
    port: databasePort,
    user: databaseUser,
    database: databaseName,
    password: databasePassword,
});

export const useConnection = async <T extends unknown>(
    callback: (conn: PoolConnection) => Promise<T>
): Promise<T> => {
    const conn = await pool.getConnection();
    return callback(conn)
        .then((res) => {
            conn.release();
            return res;
        })
        .catch((err) => {
            conn.release();
            throw err;
        })
}