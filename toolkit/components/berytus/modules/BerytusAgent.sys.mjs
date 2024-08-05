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
Agent.collectCredentialsMetadata = async function (innerWindowId, args) {
    const managers = lazy.liaison.managers;
    const entries = [];
    for (let i = 0; i < managers.length; i++) {
        const manager = managers[i];
        const handler = lazy.liaison.getRequestHandler(manager.id);
        entries.push({
            managerId: manager.id,
            credentialsMetadata: handler.manager.getCredentialsMetadata({ document: { id: innerWindowId } }, args)
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
