/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { addMozBuild } from "./add-mozbuild.js";
import { prefixModules } from "./prefix-modules.js";

const tailor = async () => {
    await prefixModules();
    await addMozBuild();
}

tailor();