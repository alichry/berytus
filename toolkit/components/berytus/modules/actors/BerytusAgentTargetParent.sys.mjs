/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { Agent } from "resource://gre/modules/BerytusAgent.sys.mjs";
export const Actor = "BerytusAgentTarget";
export class BerytusAgentTargetParent extends JSWindowActorParent {
    #isGroupValid(target, group) {
        if (typeof group !== "string") {
            return false;
        }
        return group in target;
    }
    async receiveMessage(msg) {
        if (msg.name !== `${Actor}:invokeRequestHandler`) {
            throw new Error(`Received malformed message name (${msg.name})`);
        }
        const { managerId, group, method, requestContext, requestArgs } = msg.data;
        if (typeof managerId !== 'string') {
            throw new Error("Received malformed message data; " +
                "managerId is not a string.");
        }
        const target = Agent.target(managerId);
        if (!this.#isGroupValid(target, group)) {
            throw new Error("Received malformed message data; " +
                `group is not valid (${group}).`);
        }
        const fn = target[group][method];
        if (undefined === fn || !(fn instanceof Function)) {
            throw new Error("Received malformed message data; " +
                `method is not valid (${method}).`);
        }
        return fn.apply(target, [requestContext, requestArgs]);
    }
}
