/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { PublicRequestHandler, SequentialRequestHandler } from "resource://gre/modules/BerytusRequestHandler.sys.mjs";
import { IPublicRequestHandler, IUnderlyingRequestHandler } from "./types";
import { NativeManager } from "resource://gre/modules/BerytusNativeManager.sys.mjs";

interface Manager {
    metadata: SecretManagerInfo,
    handler: SequentialRequestHandler;
}

class Liaison {
    #managers: Record<string, Manager> = {};

    get managers() {
        const res: Array<SecretManagerInfo> = [];
        Object.values(this.#managers).forEach(manager => {
            res.push(manager.metadata);
        })
        return res;
    }

    ereaseManager(id: string) {
        if (! this.#managers[id]) {
            throw new Error(
                `Cannot erease manager id ${id}, ` +
                ' manager does not exist (invalid id).'
            );
        }
        delete this.#managers[id];
    }

    registerManager(
        { id, type, name, icon }: ISecretManagerInfo,
        handler: IUnderlyingRequestHandler
    ) {
        if (this.#managers[id]) {
            throw new Error(
                `Cannot register manager id "${id}", ` +
                ' manager already register (duplicate id).'
            );
        }
        if (typeof id !== "string" || id.length === 0 || /[\s]/.test(id)) {
            throw new Error(
                "Cannot register manager. "
                + `Passed ID (${String(id)}) is invalid.`
            )
        }
        if (typeof name !== "string" || name.trim().length === 0) {
            throw new Error(
                `Cannot register manager for id "${id}". `
                + `Passed name (${String(name)}) is invalid.`
            );
        }
        if (!(type in ESecretManagerType)) {
            throw new Error(
                `Cannot register manager for id "${id}". `
                + `Passed Type (${String(type)}) is invalid.`
            );
        }
        this.#managers[id] = {
            metadata: new SecretManagerInfo(id, name, type, icon),
            handler: new SequentialRequestHandler(handler)
        };
    }

    isManagerRegistered(id: string) {
        return id in this.#managers;
    }

    getRequestHandler(id: string): IPublicRequestHandler {
        // TODO(berytus): attach ValidatedRequestHandlers with
        // BrowsingContexts. Use a WeakMap so that stateful informaion
        // held by ValidatedRequestHandlers would be gone when BCs die.
        const manager = this.#managers[id];
        if (! manager) {
            throw new Error(
                `Cannot retrieve request handler of manager ${id}, ` +
                ' manager does not exists (invalid id).'
            );
        }
        return new PublicRequestHandler(this.#managers[id].handler);
    }
}

// @ts-ignore
Liaison.prototype.QueryInterface = ChromeUtils.generateQI(
    ["mozIBerytusLiaison"]
);

export enum ESecretManagerType {
    Native = 0,
    Extension = 1
};

export interface ISecretManagerInfo {
    id: string;
    name: string;
    type: ESecretManagerType;
    icon?: string;
}

class SecretManagerInfo implements ISecretManagerInfo {
    #id: string;
    #name: string;
    #type: ESecretManagerType;
    #icon?: string;

    constructor(
        id: string,
        name: string,
        type: ESecretManagerType,
        icon?: string
    ) {
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
liaison.registerManager(
    {
        id: 'built-in',
        name: 'Built-in Manager',
        type: ESecretManagerType.Native
    },
    new NativeManager()
);

export { liaison };
export type { Liaison };

// @ts-ignore
SecretManagerInfo.prototype.QueryInterface = ChromeUtils.generateQI(
    ["mozIBerytusSecretManagerInfo"]
);