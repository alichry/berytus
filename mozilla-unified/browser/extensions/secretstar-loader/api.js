/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    AddonManager: "resource://gre/modules/AddonManager.sys.mjs"
});

this.ssLoader = class extends ExtensionAPI {
    getAPI(context) {
        return {
            ssLoader: {
                async load() {
                    console.debug("Loading Secret*");
                    const extensionDirURL = "resource://builtin-addons/secretstar/";
                    const filePath = Services.io.getProtocolHandler("resource").resolveURI(
                        Services.io.newURI(extensionDirURL)
                    )
                    const file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
                    file.initWithPath(filePath.substring('file://'.length));
                    await lazy.AddonManager.installTemporaryAddon(file);
                }
            }

        }
    }
};