/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { readFile } from 'node:fs/promises';
import { join, dirname, relative, resolve } from 'node:path';
import type { PathEntry } from './produce-ts-compiler-paths';
import { fileURLToPath } from 'node:url';

// @ts-ignore: TS compiler complains since we have set
// "module" to ES6 and not ES2020+. This is fine since
// we run this code in a modern node.js version.
const __dirname = dirname(fileURLToPath(import.meta.url));

export const constructAnyPathEntries = async (): Promise<Array<PathEntry>> => {
    const typesDir = join(__dirname, "types");
    const content = await readFile(
        join(__dirname, "any-modules.txt"), { encoding: 'utf8' }
    );
    if (content.length === 0) {
        return [];
    }
    return content.split("\n").map((path: string) => {
        return {
            path,
            target: [join(relative(resolve(), typesDir), 'any.d.ts')]
        } as const;
    })
}