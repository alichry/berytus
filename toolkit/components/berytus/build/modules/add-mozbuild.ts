/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { getModules } from './utils.js';

export const addMozBuild = async () => {
    const modules: Array<string> = await getModules(
        resolve("./modules")
    );
    const actorModules: Array<string> = await getModules(
        resolve("./modules/actors")
    );

    const mozFile = `# -*- Mode: python; indent-tabs-mode: nil; tab-width: 40 -*-
# vim: set filetype=python:
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.\n`
        + "EXTRA_JS_MODULES += [\n"
        + modules.map(mn => {
                return `\t"${mn}"`
            }).join(",\n")
        + "\n]"
        + `
FINAL_TARGET_FILES.actors += [
    ${actorModules.map(p => `"actors/${p}"`).join("\t,\n")}
]`;

    await writeFile(
        resolve('./modules/moz.build'),
        mozFile,
        { encoding: 'utf8' }
    );
}