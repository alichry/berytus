#!/usr/bin/env -S node --loader ts-node/esm
console.log('🚨 Migrating database (if necessary)');
import FlywayCli from './flyway-common.js';

const { success, stdout } = await FlywayCli(true, 'migrate');

if (! success) {
    console.error('❌  Database migration failed!');
} else if (stdout.match(/No migration necessary/)) {
    console.log('🗞️  Migration not needed.');
} else {
    console.log('✅  Database migration succeeded!');
}