/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { readdir, writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';

export const getModulePaths = async () => {
    const modulePaths: Array<string> = [];
    const files = await readdir(
        resolve("./modules")
    );
    for (const file of files) {
        if (file.endsWith(".sys.mjs")) {
            modulePaths.push(file);
        }
    }
    modulePaths.sort((a, b) => {
        return a.localeCompare(b);
    });
    return modulePaths;
}

export const addMozBuild = async () => {
    const modulePaths: Array<string> = await getModulePaths();

    const mozFile = `# -*- Mode: python; indent-tabs-mode: nil; tab-width: 40 -*-
# vim: set filetype=python:
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.\n`
        + "EXTRA_JS_MODULES += [\n"
        + modulePaths.map(mn => {
                return `\t"${mn}"`
            }).join(",\n")
        + "\n]";

    await writeFile(
        resolve('./modules/moz.build'),
        mozFile,
        { encoding: 'utf8' }
    );
}