/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { PublicRequestHandler, SequentialRequestHandler } from "resource://gre/modules/BerytusRequestHandler.sys.mjs";
import { NativeManager } from "resource://gre/modules/BerytusNativeManager.sys.mjs";
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
    registerManager({ id, type, name, icon }, handler) {
        if (this.#managers[id]) {
            throw new Error(`Cannot register manager id "${id}", ` +
                ' manager already register (duplicate id).');
        }
        if (typeof id !== "string" || id.length === 0 || /[\s]/.test(id)) {
            throw new Error("Cannot register manager. "
                + `Passed ID (${String(id)}) is invalid.`);
        }
        if (typeof name !== "string" || name.trim().length === 0) {
            throw new Error(`Cannot register manager for id "${id}". `
                + `Passed name (${String(name)}) is invalid.`);
        }
        if (!(type in ESecretManagerType)) {
            throw new Error(`Cannot register manager for id "${id}". `
                + `Passed Type (${String(type)}) is invalid.`);
        }
        this.#managers[id] = {
            metadata: new SecretManagerInfo(id, name, type, icon),
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
    #name;
    #type;
    #icon;
    constructor(id, name, type, icon) {
        this.#id = id;
        this.#name = name;
        this.#type = type;
        this.#icon = icon;
    }
    get id() {
        return this.#id;
    }
    get name() {
        return this.#name;
    }
    get type() {
        return this.#type;
    }
    get icon() {
        return this.#icon;
    }
}
const liaison = new Liaison();
liaison.registerManager({
    id: 'built-in',
    name: 'Built-in Manager',
    type: ESecretManagerType.Native
}, new NativeManager());
export { liaison };
// @ts-ignore
SecretManagerInfo.prototype.QueryInterface = ChromeUtils.generateQI(["mozIBerytusSecretManagerInfo"]);
