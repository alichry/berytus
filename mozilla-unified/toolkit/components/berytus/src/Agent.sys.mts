/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import type { CredentialsMetadata, GetCredentialsMetadataArgs, IPublicRequestHandler, UriParams } from "./types";
import type { ISecretManagerInfo, Liaison } from './Liaison.sys.mjs';

let lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    liaison: "resource://gre/modules/BerytusLiaison.sys.mjs"
});

export interface CollectCredentialsMetadataEntry {
    manager: ISecretManagerInfo;
    credentialsMetadata: CredentialsMetadata | Promise<CredentialsMetadata>
};

export function Agent() { }

Agent.target = function (managerId: string) {
    return new AgentTarget(lazy.liaison, managerId);
}

Agent.collectCredentialsMetadata = async (
    innerWindowId: number,
    uri: UriParams,
    args: GetCredentialsMetadataArgs
): Promise<Array<CollectCredentialsMetadataEntry>> => {
    const managers = lazy.liaison.managers;
    const entries: Array<CollectCredentialsMetadataEntry> = [];
    const keys = args.channelConstraints.secretManagerPublicKey;
    const context = {
        document: {
            id: innerWindowId,
            uri
        },
    };
    const relevantManagers = keys && keys.length > 0
        ? (await Promise.all(managers.map(async manager => {
            const target = Agent.target(manager.id);
            let key: string;
            try {
                key = await target.manager.getSigningKey(
                    context,
                    { webAppActor: args.webAppActor }
                );
            } catch (e) {
                return null;
            }
            if (keys.indexOf(key) !== -1) {
                return manager;
            }
            return null;
            // TODO(berytus): Perhaps, in the future, we could
            // include another list of managers, the ones that
            // threw an exceptions (see try-catch above), and the
            // ones who where filtered out.
        }))).filter(m => !!m)
        : managers;

    for (let i = 0; i < relevantManagers.length; i++) {
        const manager = relevantManagers[i];
        const target = Agent.target(manager.id);
        entries.push({
            manager,
            credentialsMetadata: target.manager.getCredentialsMetadata(
                context,
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