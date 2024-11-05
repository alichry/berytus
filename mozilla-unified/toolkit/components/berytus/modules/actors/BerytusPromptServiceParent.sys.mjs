/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
import { PassthroughParent } from "resource://gre/modules/BerytusParentUtils.sys.mjs";
import { PromptService } from "resource://gre/modules/BerytusPromptService.sys.mjs";
export const Actor = 'BerytusPromptService';
/* Class name must match Actor name */
export class BerytusPromptServiceParent extends PassthroughParent {
    Actor = Actor;
    #inst;
    getInstance() {
        if (!this.#inst) {
            this.#inst = new PromptService();
        }
        return this.#inst;
    }
}
