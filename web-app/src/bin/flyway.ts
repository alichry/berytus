#!/usr/bin/env -S node --loader ts-node/esm
import FlywayCli from './flyway-common.js';

await FlywayCli(true, ...process.argv.slice(2));