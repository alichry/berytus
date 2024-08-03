/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { readFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import type { PathEntry } from './produce-ts-compiler-paths';

export const constructAnyPathEntries = async (): Promise<Array<PathEntry>> => {
    const content = await readFile(
        resolve("./build/paths/any-modules.txt"), { encoding: 'utf8' }
    );
    if (content.length === 0) {
        return [];
    }
    return content.split("\n").map((path: string) => {
        return {
            path,
            target: ["./build/paths/modules/any.d.ts"]
        } as const;
    })
}