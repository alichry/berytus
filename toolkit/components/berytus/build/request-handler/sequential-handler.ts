/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 export const generateSequentialdHandler = () => {
    let code = `export class SequentialRequestHandler extends ValidatedRequestHandler {

    protected busy: boolean = false;

    protected preCall(group: string, method: string, args: unknown) {
        if (this.busy) {
            throw new Components.Exception(
                'Refusing to send request to secret manager, '
                + 'an existing request is still pending. '
                + 'make sure to send one request at a time '
                + 'to the secret manager.',
                Cr.NS_ERROR_FAILURE
            );
        }
        super.preCall(group, method, args);
        this.busy = true;
    }
    protected preResolve(group: string, method: string, value: unknown) {
        this.busy = false;
        super.preResolve(group, method, value);
    }
    protected preReject(group: string, method: string, value: unknown) {
        this.busy = false;
        super.preReject(group, method, value);
    }
    protected handleUnexpectedException<G extends keyof RequestHandler, M extends keyof RequestHandler[G]>(group: G, method: M, response: ResponseContext<G, M>["response"], excp: unknown) {
        this.busy = false;
        super.handleUnexpectedException(group, method, response, excp);
    }
}`;
    return code;
}