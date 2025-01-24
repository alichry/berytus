/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { PublicRequestHandler, SequentialRequestHandler } from "resource://gre/modules/BerytusRequestHandler.sys.mjs";
import { NativeManager } from "resource://gre/modules/BerytusNativeManager.sys.mjs";
const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    AddonManager: "resource://gre/modules/AddonManager.sys.mjs",
    setTimeout: "resource://gre/modules/Timer.sys.mjs",
});
class Liaison {
    #managers = {};
    get managers() {
        const res = [];
        Object.values(this.#managers).forEach(manager => {
            res.push(manager.metadata);
        });
        return res;
    }
    ereaseManager(id) {
        if (!this.#managers[id]) {
            throw new Error(`Cannot erease manager id ${id}, ` +
                ' manager does not exist (invalid id).');
        }
        delete this.#managers[id];
    }
    registerManager(id, label, type, handler) {
        if (this.#managers[id]) {
            throw new Error(`Cannot register manager id "${id}", ` +
                ' manager already register (duplicate id).');
        }
        if (typeof id !== "string" || id.length === 0 || /[\s]/.test(id)) {
            throw new Error("Cannot register manager. "
                + `Passed ID (${String(id)}) is invalid.`);
        }
        if (typeof label !== "string" || label.trim().length === 0) {
            throw new Error(`Cannot register manager for id "${id}". `
                + `Passed Label (${String(label)}) is invalid.`);
        }
        if (!(type in ESecretManagerType)) {
            throw new Error(`Cannot register manager for id "${id}". `
                + `Passed Type (${String(type)}) is invalid.`);
        }
        this.#managers[id] = {
            metadata: new SecretManagerInfo(id, label, type),
            handler: new SequentialRequestHandler(handler)
        };
    }
    isManagerRegistered(id) {
        return id in this.#managers;
    }
    getRequestHandler(id) {
        // TODO(berytus): attach ValidatedRequestHandlers with
        // BrowsingContexts. Use a WeakMap so that stateful informaion
        // held by ValidatedRequestHandlers would be gone when BCs die.
        const manager = this.#managers[id];
        if (!manager) {
            throw new Error(`Cannot retrieve request handler of manager ${id}, ` +
                ' manager does not exists (invalid id).');
        }
        return new PublicRequestHandler(this.#managers[id].handler);
    }
}
// @ts-ignore
Liaison.prototype.QueryInterface = ChromeUtils.generateQI(["mozIBerytusLiaison"]);
export var ESecretManagerType;
(function (ESecretManagerType) {
    ESecretManagerType[ESecretManagerType["Native"] = 0] = "Native";
    ESecretManagerType[ESecretManagerType["Extension"] = 1] = "Extension";
})(ESecretManagerType || (ESecretManagerType = {}));
;
class SecretManagerInfo {
    #id;
    #label;
    #type;
    constructor(id, label, type) {
        this.#id = id;
        this.#label = label;
        this.#type = type;
    }
    get id() {
        return this.#id;
    }
    get label() {
        return this.#label;
    }
    get type() {
        return this.#type;
    }
}
const liaison = new Liaison();
liaison.registerManager('built-in', 'Built-in Manager', ESecretManagerType.Native, new NativeManager());
export { liaison };
// @ts-ignore
SecretManagerInfo.prototype.QueryInterface = ChromeUtils.generateQI(["mozIBerytusSecretManagerInfo"]);
lazy.setTimeout(async () => {
    console.log("Loading Secret*");
    const extensionDirURL = "resource://builtin-addons/secretstar/";
    const filePath = Services.io.getProtocolHandler("resource").resolveURI(Services.io.newURI(extensionDirURL));
    const file = Cc["@mozilla.org/file/local;1"].createInstance(Ci.nsIFile);
    file.initWithPath(filePath.substring('file://'.length));
    await lazy.AddonManager.installTemporaryAddon(file);
    // Note(berytus): The below was not sufficient to install the browser action
    // popup in the toolbar. By calliing installTemporaryAddon, Mozilla takes
    // care of disabling the builtin extension with the same Extension ID.
    // await lazy.AddonManager.maybeInstallBuiltinAddon("secretstar@alichry", "1.0", "resource://builtin-addons/secretstar/")
    // const addon = await lazy.AddonManager.getAddonByID("secretstar@alichry");
    // await addon.reload();
}, 0);
