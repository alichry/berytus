/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export const generateValidatedHandler = () => {
    let code = `export class ValidatedRequestHandler extends IsolatedRequestHandler {
    constructor(impl: IUnderlyingRequestHandler) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
    }
    preCall(group: string, method: string, args: unknown) {
        // TODO(berytus): validate input
    }
    preResolve(group: string, method: string, value: unknown) {
        // TODO(berytus): validate output
    }
    preReject(group: string, method: string, value: unknown) {
        // TODO(berytus): validate error value
    }
}`;
    return code;
}