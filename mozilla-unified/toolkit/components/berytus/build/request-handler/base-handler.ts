/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { RequestHandlerParser } from "./RequestHandlerParser.js";

export const generateIsolatedRequestHandler = () => {
    let code = `
export class IsolatedRequestHandler implements IUnderlyingRequestHandler {
    #impl: IUnderlyingRequestHandler;`;
    const h = new RequestHandlerParser();
    const groups = h.getGroups();
    const typesToImport: Record<string, true> = {
        "IUnderlyingRequestHandler": true,
        "ResponseContext": true
    }
    for (let i = 0; i < groups.length; i++) {
        const group = groups[i];
        code += `
    ${group}: IUnderlyingRequestHandler["${group}"];`;
    }
    code += `
    constructor(impl: IUnderlyingRequestHandler) {
        this.#impl = impl;
        const self = this;`;
    for (let i = 0; i < groups.length; i++) {
        const group = groups[i];
        const methods = h.getMethods(group);
        code += `
        this.${group} = {`;

        for (let j = 0; j < methods.length; j++) {
            const {
                name,
                parameters
            } = h.parseMethod(group, methods[j]);
            const ctxVar = parameters[0].name;
            const responseCtxType = `ResponseContext<"${group}", "${name}">`;
            const hookArgs = `"${group}", "${name}", { ${parameters.map(p => `${p.name}`).join(', ')} }`;

            code += `
            async ${name}(`;
            code += parameters.map((p, i) => {
                if (!("alias" in p.type) || p.type.alias === undefined) {
                    throw new Error(
                        "Expecting argument type to have an alias"
                        + ", got otherwise."
                    );
                }
                typesToImport[p.type.alias] = true;
                return `${p.name}: ${p.type.alias}`
                    + (i === 0 ? ` & ${responseCtxType}` : '')
            }).join(', ');
            code += `): Promise<void> {`;

            code += `
                try {
                    await self.preCall(${hookArgs});
                } catch (e) {
                    ${ctxVar}.response.reject(e);
                    return;
                }`

            code += `
                const wrappedResponseCtx: ${responseCtxType} = {
                    response: {
                        async resolve(val: Parameters<${responseCtxType}["response"]["resolve"]>[0]) {
                            try {
                                await self.preResolve("${group}", "${name}", val)
                            } catch (e: any) {
                                ${ctxVar}.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            ${ctxVar}.response.resolve(val);
                        },
                        async reject(val: unknown) {
                            try {
                                await self.preReject("${group}", "${name}", val);
                            } catch (e) {
                                ${ctxVar}.response.reject(e);
                                throw e;
                            }
                            ${ctxVar}.response.reject(val);
                        }
                    }
                };`;

            code += `
                try {
                    await self.#impl.${group}.${name}({
                        ...${ctxVar},
                        ...wrappedResponseCtx
                    }, ${parameters.slice(1).map(p => p.name).join(', ')});
                } catch (e) {
                    self.handleUnexpectedException(
                        "${group}",
                        "${name}",
                        ${ctxVar}.response,
                        e
                    );
                    return;
                }`;
            code += `
            },`;
        }
        code += `
        };`;
    }
    code += `
    }`;

    code += `
    protected preCall(group: string, method: string, args: unknown) {}`;
    code += `
    protected preResolve(group: string, method: string, value: unknown) {}`;
    code += `
    protected preReject(group: string, method: string, value: unknown) {}`;
    code += `
    protected handleUnexpectedException<G extends keyof RequestHandler, M extends keyof RequestHandler[G]>(group: G, method: M, response: ResponseContext<G, M>["response"], excp: unknown) {
        // TODO(berytus): Define what would be the
        // accepted values to reject a request with.
        console.error("Unexpected exception from secret manager:", excp);
        const err = new Components.Exception(
            "Error sending request to secret manager. "
            + "The secret manager unexpectedly threw an exception "
            + "instead of using the reject callback (RequestType: "
            + \`\${group}:\${String(method)}). Exception: \${excp}\`,
            Cr.NS_ERROR_FAILURE
        );
        response.reject(err);
    }`;
    code += `
}`;
    return {
        classCode: code,
        typesToImport
    } as const;
}