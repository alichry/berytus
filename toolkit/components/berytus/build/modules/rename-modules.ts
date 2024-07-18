/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { readdir, rename } from 'node:fs/promises';
import { resolve, extname, dirname, basename } from 'node:path';

/**
 * Rename any (*).mjs (but not *.sys.mjs) file
 * to Berytus\1.sys.mjs
 */
export const renameModules = async () => {
    const wd = resolve("./modules");
    const files = await readdir(
        wd
    );
    for (const file of files) {
        if (file === 'moz.build') {
            continue;
        }
        if (file.endsWith('.sys.mjs')) {
            continue;
        }
        const ext = extname(file);
        if (ext !== '.mjs') {
            continue;
        }
        const moduleName = "Berytus"
            + basename(file, ext) + '.sys.mjs';
        await rename(
            wd + '/' + file,
            wd + '/' + dirname(file) + '/' + moduleName
        );
    }
}