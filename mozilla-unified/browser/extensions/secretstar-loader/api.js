/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    AddonManager: "resource://gre/modules/AddonManager.sys.mjs",
    ExtensionParent: "resource://gre/modules/ExtensionParent.sys.mjs",
    setTimeout: "resource://gre/modules/Timer.sys.mjs"
});

Object.defineProperties(lazy, {
    browserActionFor: {
        get() {
            return lazy.ExtensionParent.apiManager.global.browserActionFor;
        },
    },
});

const ssPath = Services.dirsvc.get("GreD", Ci.nsIFile);
ssPath.appendRelativePath("browser/berytus/secretstar@alichry/")

let loaded = false;

this.ssLoader = class extends ExtensionAPI {
    getAPI(context) {
        return {
            ssLoader: {
                async load() {
                    if (loaded) {
                        console.debug("Secret* already loaded; refusing to re-load");
                        return;
                    }
                    console.debug("Loading Secret* at", ssPath.path);
                    if (!(await IOUtils.exists(ssPath.path))) {
                        console.error("Secret* directory was not found.");
                        return;
                    }
                    await lazy.AddonManager.installTemporaryAddon(ssPath);
                    loaded = true;
                    lazy.setTimeout(() => {
                        try {
                            const ssExt = lazy.ExtensionParent.GlobalManager.getExtension("secretstar@alichry");
                            if (!ssExt) {
                                return;
                            }
                            const wd = Services.wm.getMostRecentBrowserWindow();
                            if (!wd) {
                                return;
                            }
                            const ba = lazy.browserActionFor(ssExt);
                            ba.openPopup(wd);
                        } catch (e) {
                            console.error(e);
                        }
                    }, 3000);
                }
            }

        }
    }
};