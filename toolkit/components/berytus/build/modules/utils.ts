/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { readdir, rename } from 'node:fs/promises';
import { dirname, basename } from 'node:path';

/**
 * @param {string} dir
 * @returns A sorted list of *.sys.mjs file paths. Each
 *  entry is a relative path with `dir` as its parent directory.
 */
export const getModules = async (
    dir: string
): Promise<Array<string>> => {
    const ext = ".sys.mjs";
    const modulePaths: Array<string> = await getFilesByExtension(
        dir,
        ext
    );
    modulePaths.sort((a, b) => {
        return a.localeCompare(b);
    });
    return modulePaths;
}

export const renameModules = async (
    dir: string,
    renameFn: (moduleName: string) => string
) => {
    const modules: Array<string> = await getModules(dir);
    for (const module of modules) {
        const moduleName = basename(module);
        const newModuleName = renameFn(moduleName);

        if (moduleName === newModuleName) {
            continue;
        }

        const newModule =  dirname(module)
            + '/'
            + newModuleName;
        await rename(
            dir + '/' + module,
            dir + '/' + newModule
        );
    }
}

export const getFilesByExtension = async (
    dir: string,
    ext: string,
    recursive = false
): Promise<Array<string>> => {
    const res: Array<string> = [];
    const files = await readdir(dir, { recursive });
    for (const file of files) {
        if (file.endsWith(ext)) {
            res.push(file);
        }
    }
    return res;
}