/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { basename, dirname, join } from 'node:path';
import { getModulePaths } from '../modules/add-mozbuild.js';
import type { PathEntry } from './produce-ts-paths.js';

export const produceBerytusPaths = async (): Promise<Array<PathEntry>> => {
    const modules = await getModulePaths();

    return modules.map(path => {
        const moduleName: string = basename(path, '.sys.mjs')
            .replace(/^Berytus/, '');
        const dir = dirname(path);
        return {
            path: 'resource://gre/modules/' + basename(path),
            target: [join('./src', dir, moduleName + '.mts')]
        } as const;
    });
}