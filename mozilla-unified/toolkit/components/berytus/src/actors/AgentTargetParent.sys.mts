/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Agent } from "resource://gre/modules/BerytusAgent.sys.mjs";
import type { AgentTarget } from "src/Agent.sys.mjs";
import type { RequestGroup } from "src/types";

export const Actor = "BerytusAgentTarget";

export class BerytusAgentTargetParent extends JSWindowActorParent {

    #isGroupValid(target: AgentTarget, group: unknown): group is RequestGroup {
        if (typeof group !== "string") {
            return false;
        }
        return group in target;
    }

    async #processMessage(msg: ActorMessage) {
        if (msg.name !== `${Actor}:invokeRequestHandler`) {
            throw new Error(
                `Received malformed message name (${msg.name})`
            );
        }
        const {
            managerId,
            group,
            method,
            requestContext,
            requestArgs
        } = msg.data;
        if (typeof managerId !== 'string') {
            throw new Error(
                "Received malformed message data; " +
                "managerId is not a string."
            );
        }
        const target = Agent.target(managerId);
        if (! this.#isGroupValid(target, group)) {
            throw new Error(
                "Received malformed message data; " +
                `group is not valid (${group}).`
            );
        }
        const fn: unknown = target[group][method as keyof typeof target[typeof group]];
        if (undefined === fn || !(fn instanceof Function)) {
            throw new Error(
                "Received malformed message data; " +
                `method is not valid (${method}).`
            );
        }
        const result = await fn.apply(
            target,
            [requestContext, requestArgs]
        );
        return result;
    }

    didDestroy() {
        console.debug(`BerytusAgentTargetParent::didDestroy()`);
    }

    async receiveMessage(msg: ActorMessage) {
        try {
            return (await this.#processMessage(msg));
        } catch (e: any) {
            console.error(e);
            // Some errors, such as Components.Exception,
            // cannot be cloned. Here we throw plain objects instead.
            const defaultErr = {
                result: Cr.NS_ERROR_FAILURE,
                message: "Exception occurred during request processing."
            };
            if (typeof e !== "object" || e == null) {
                throw defaultErr;
            }
            throw {
                result: e.result || defaultErr.result,
                message: e.message || defaultErr.message
            };
        }
    }
}
