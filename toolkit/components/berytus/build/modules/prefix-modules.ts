/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { renameModules } from "./utils.js"
import { resolve } from 'node:path';

export const prefixModules = async () => {
    const renameFn = (moduleName: string) => {
        if (moduleName.startsWith('Berytus')) {
            return moduleName;
        }
        return 'Berytus' + moduleName;
    };

    await renameModules(
        resolve("./modules"),
        renameFn
    );
    await renameModules(
        resolve("./modules/actors"),
        renameFn
    );
}