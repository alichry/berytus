/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 export const generateSequentialdHandler = () => {
    let code = `export class SequentialRequestHandler extends ValidatedRequestHandler {

    protected busy: boolean = false;

    protected async preCall(group: string, method: string, input: PreCallInput) {
        if (this.busy) {
            throw new Components.Exception(
                'Refusing to send request to secret manager, '
                + 'an existing request is still pending. '
                + 'make sure to send one request at a time '
                + 'to the secret manager.',
                Cr.NS_ERROR_FAILURE
            );
        }
        this.busy = true;
        try {
            await super.preCall(group, method, input);
        } catch (e) {
            this.busy = false;
            throw e;
        }
    }
    protected async preResolve(group: string, method: string, input: PreCallInput, value: unknown) {
        this.busy = false;
        await super.preResolve(group, method, input, value);
    }
    protected async preReject(group: string, method: string, input: PreCallInput, value: unknown) {
        this.busy = false;
        await super.preReject(group, method, input, value);
    }
    protected handleUnexpectedException<G extends keyof RequestHandler, M extends keyof RequestHandler[G]>(group: G, method: M, response: ResponseContext<G, M>["response"], excp: unknown) {
        this.busy = false;
        super.handleUnexpectedException(group, method, response, excp);
    }
}`;
    return code;
}