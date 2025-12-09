import { pool, useConnection, table } from '../src/backend/db/pool.js';

if (process.env.NODE_ENV !== "development") {
    throw new Error('Refusing to preprare test db in a non-development (test) environment.')
}

const cleanDb = async () => {
    const tables = [
        'berytus_account_auth_challenge_message',
        'berytus_account_auth_challenge',
        'berytus_account_def_auth_challenge',
        'berytus_account_auth_session',
        'berytus_account_field',
        'berytus_account_user_attributes',
        'berytus_account_def_key_field_id',
        'berytus_account_def_field',
        'berytus_account_def_category_version',
        'berytus_account_def_category',
        'berytus_account_def',
        'berytus_account'
    ] as const;

    return useConnection(async conn => {
        for (let i = 0; i < tables.length; i++) {
            try {
                await conn`DELETE FROM ${table(tables[i])}`;
            } catch (e) {
                console.error("Unable to delete " + tables[i]);
                throw e;
            }

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
