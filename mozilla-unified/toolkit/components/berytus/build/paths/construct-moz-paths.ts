/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { basename, dirname, join, resolve, relative } from 'node:path';
import { getFilesByExtension } from '../modules/utils.js';
import type { PathEntry } from './produce-ts-compiler-paths.js';
import { fileURLToPath } from 'node:url';

// @ts-ignore: TS compiler complains since we have set
// "module" to ES6 and not ES2020+. This is fine since
// we run this code in a modern node.js version.
const __dirname = dirname(fileURLToPath(import.meta.url));

export const constructMozPathEntries = async (): Promise<Array<PathEntry>> => {
    const typesDir = join(__dirname, "types");
    const types = (await getFilesByExtension(
        typesDir,
        ".d.ts",
        true
    )).filter(t => basename(t) !== 'any.d.ts');

    return types.map(typePath => {
        const moduleName: string = basename(typePath, '.d.ts');
        const importDir: string = dirname(typePath);
        return {
            path: 'resource://' + join(importDir, moduleName) + '.sys.mjs',
            target: [join(relative(resolve(), typesDir), typePath)]
        } as const;
    });
}