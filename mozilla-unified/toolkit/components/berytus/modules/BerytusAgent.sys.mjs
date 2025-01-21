/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
let lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    liaison: "resource://gre/modules/BerytusLiaison.sys.mjs"
});
;
export function Agent() { }
Agent.target = function (managerId) {
    return new AgentTarget(lazy.liaison, managerId);
};
Agent.collectCredentialsMetadata = async (innerWindowId, uri, args) => {
    const managers = lazy.liaison.managers;
    const entries = [];
    const keys = args.channelConstraints.secretManagerPublicKey;
    const context = {
        document: {
            id: innerWindowId,
            uri
        },
    };
    const relevantManagers = keys && keys.length > 0
        ? (await Promise.all(managers.map(async (manager) => {
            const target = Agent.target(manager.id);
            let key;
            try {
                key = await target.manager.getSigningKey(context, { webAppActor: args.webAppActor });
            }
            catch (e) {
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
            credentialsMetadata: target.manager.getCredentialsMetadata(context, args)
        });
    }
    return entries;
};
class AgentTarget {
    #liaison;
    #managerId;
    constructor(liaison, managerId) {
        this.#liaison = liaison;
        this.#managerId = managerId;
    }
    get #requestHandler() {
        return this.#liaison.getRequestHandler(this.#managerId);
    }
    get manager() {
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
