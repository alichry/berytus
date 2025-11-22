import { readFileSync } from 'fs';
import { env, int } from './utils.js';

export const databaseHost = env('DB_HOST', 'localhost');
export const databasePort = int(env('DB_PORT', "5432"));
export const databaseUser = env('DB_USER', 'root');
export const databasePassword =
    process.env.DB_PASS_FILE
        ? readFileSync(env('DB_PASS_FILE'), 'ascii')
        : env('DB_PASS', 'root');
export const databaseName = env('DB_NAME', 'berytus');
export const databaseSchema = env('DB_SCHEMA', 'app');