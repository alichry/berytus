import { type PoolConnection, useConnection, useReservedConnection } from "@root/backend/db/pool.js";
import { databaseSchema } from "@root/backend/env/db.js"
;
export const withSearchPath = async <T extends unknown>(
    callback: (conn: PoolConnection) => Promise<T>
): Promise<T> => {
    return useReservedConnection(async conn => {
        try {
            await conn`set search_path = ${conn(databaseSchema)}`;
            return await callback(conn);
        } finally {
            await conn`set search_path = ''`
        }
    });
}
