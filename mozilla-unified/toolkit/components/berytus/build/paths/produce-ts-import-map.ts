/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { writeFile } from 'node:fs/promises';
import { resolve } from 'node:path';
import { constructAnyPathEntries } from './construct-any-paths.js';
import { constructBerytusPathEntries } from './construct-berytus-paths.js';
import { constructMozPathEntries } from './construct-moz-paths.js';

const run = async () => {
    const anyPaths = await constructAnyPathEntries();
    const berytusPaths = await constructBerytusPathEntries();
    const mozPaths = await constructMozPathEntries();

    const imports = [...berytusPaths, ...anyPaths, ...mozPaths].map(({ path, target }) => {
        return `"${path}": typeof import(${JSON.stringify("./" + target[0])})`;
    });

    await writeFile(
        resolve("./esm-import-map.index.d.ts"),
        `// This file is automatically generated; do not edit.
export type ModuleMap = {
    ${imports.join(",\n\t")}
}`,
        { encoding: 'utf8' }
    );
}

run();
