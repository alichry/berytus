/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { CredentialsMetadata, GetCredentialsMetadataArgs, IPublicRequestHandler } from "./types";
import type { Liaison } from './Liaison.sys.mjs';

let lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    liaison: "resource://gre/modules/BerytusLiaison.sys.mjs"
});

export interface CollectCredentialsMetadataEntry {
    managerId: string;
    credentialsMetadata: Promise<CredentialsMetadata>
};

export function Agent() { }

Agent.target = function (managerId: string) {
    return new AgentTarget(lazy.liaison, managerId);
}

Agent.collectCredentialsMetadata = async function (
    innerWindowId: number,
    args: GetCredentialsMetadataArgs
): Promise<Array<CollectCredentialsMetadataEntry>> {
    const managers = lazy.liaison.managers;
    const entries: Array<CollectCredentialsMetadataEntry> = [];
    for (let i = 0; i < managers.length; i++) {
        const manager = managers[i];
        const handler = lazy.liaison.getRequestHandler(manager.id);
        entries.push({
            managerId: manager.id,
            credentialsMetadata: handler.manager.getCredentialsMetadata(
                { document: { id: innerWindowId } },
                args
            )
        });
    }
    return entries;
}

class AgentTarget implements IPublicRequestHandler {
    #liaison: Liaison;
    #managerId: string;

    constructor(liaison: Liaison, managerId: string) {
        this.#liaison = liaison;
        this.#managerId = managerId;
    }

    get #requestHandler() {
        return this.#liaison.getRequestHandler(this.#managerId);
    }

    get manager()  {
        return this.#requestHandler.manager;
    }
    get channel() {
        return this.#requestHandler.channel;
    }
    get login() {
        return this.#requestHandler.login;
    }
    get accountCreation() {
        return this.#requestHandler.accountCreation;
    }
    get accountAuthentication() {
        return this.#requestHandler.accountAuthentication;
    }
}

export type { AgentTarget }