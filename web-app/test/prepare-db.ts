import { pool, useConnection } from '../src/backend/db/pool.js';

if (process.env.NODE_ENV !== "development") {
    throw new Error('Refusing to preprare test db in a non-development (test) environment.')
}

const cleanDb = async () => {
    const tables = [
        'berytus_account_auth_challenge_message',
        'berytus_account_auth_challenge_def',
        'berytus_account_field',
        'berytus_account_def_key_field_id',
        'berytus_account_def_field',
        'berytus_account_def_category_version',
        'berytus_account_def_category',
        'berytus_account_def'
    ];

    return useConnection(async conn => {
        for (let i = 0; i < tables.length; i++) {
            await conn.query(`DELETE FROM ${tables[i]}`);
        }
    });
}

beforeEach(async () => {
    await cleanDb();
});


after(async () => {
    await cleanDb();
    pool.end();
});
