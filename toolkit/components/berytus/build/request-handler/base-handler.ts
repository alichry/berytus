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
            ${name}(`;
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
            code += `): void {`;

            code += `
                self.preCall(${hookArgs});\n`

            code += `
                const wrappedResponseCtx: ${responseCtxType} = {
                    response: {
                        resolve(val: Parameters<${responseCtxType}["response"]["resolve"]>[0]) {
                            self.preResolve(${hookArgs})
                            ${ctxVar}.response.resolve(val);
                        },
                        reject(val: unknown) {
                            self.preReject(${hookArgs})
                            ${ctxVar}.response.reject(val);
                        }
                    }
                };`;

            code += `
                self.#impl.${group}.${name}(`;
            code += `{
                    ...${ctxVar},
                    ...wrappedResponseCtx
                }, `;
            code += parameters.slice(1).map(p => p.name).join(', ');
            code += `);`;

            code += `
            },`;
        }
        code += `
        };`;
    }
    code += `
    }`;

    code += `
    preCall(group: string, method: string, args: unknown) {}`;
    code += `
    preResolve(group: string, method: string, value: unknown) {}`;
    code += `
    preReject(group: string, method: string, value: unknown) {}`;

    code += `
}`;
    return {
        classCode: code,
        typesToImport
    } as const;
}