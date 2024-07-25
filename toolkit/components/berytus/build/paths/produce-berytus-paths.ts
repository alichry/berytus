/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { basename, dirname, join, resolve } from 'node:path';
import { getModulePaths } from '../modules/add-mozbuild.js';
import type { PathEntry } from './produce-ts-paths.js';

export const produceBerytusPaths = async (): Promise<Array<PathEntry>> => {
    const modules = await getModulePaths(
        resolve("./src"),
        ".sys.mts"
    );

    return modules.map(path => {
        const moduleName: string = basename(path, '.sys.mts');
        return {
            path: 'resource://gre/modules/' + 'Berytus' + moduleName + '.sys.mjs',
            target: [join('./src', path)]
        } as const;
    });
}