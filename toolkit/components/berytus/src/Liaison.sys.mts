/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { PublicRequestHandler, SequentialRequestHandler } from "resource://gre/modules/BerytusRequestHandler.sys.mjs";
import { IPublicRequestHandler, IUnderlyingRequestHandler } from "./types";

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
        id: string,
        label: string,
        type: ESecretManagerType,
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
        if (typeof label !== "string" || label.trim().length === 0) {
            throw new Error(
                `Cannot register manager for id "${id}". `
                + `Passed Label (${String(label)}) is invalid.`
            );
        }
        if (!(type in ESecretManagerType)) {
            throw new Error(
                `Cannot register manager for id "${id}". `
                + `Passed Type (${String(type)}) is invalid.`
            );
        }
        this.#managers[id] = {
            metadata: new SecretManagerInfo(id, label, type),
            handler: new SequentialRequestHandler(handler)
        };
    }

    isManagerRegistered(id: string) {
        return id in this.#managers;
    }

    getRequestHandler(id: string): IPublicRequestHandler {
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

class SecretManagerInfo {
    #id: string;
    #label: string;
    #type: ESecretManagerType;

    constructor(
        id: string,
        label: string,
        type: ESecretManagerType
    ) {
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

export const liaison = new Liaison();

// @ts-ignore
SecretManagerInfo.prototype.QueryInterface = ChromeUtils.generateQI(
    ["mozIBerytusSecretManagerInfo"]
);