/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { RequestHandlerParser } from "./RequestHandlerParser.js";

const capitlise = (str: string) =>
        str.charAt(0).toUpperCase() + str.slice(1);

export const generatePublicHandler = () => {
    const typesToImport: Record<string, true> = {
        "IPublicRequestHandler": true,
        "IUnderlyingRequestHandler": true,
        "ResponseContext": true,
        "RequestHandler": true
    }
    let code = `
function uuid(): string {
    // @ts-ignore: TODO(berytus): add to index.d.ts
    return Services.uuid
        .generateUUID()
        .toString();
}
export class PublicRequestHandler implements IPublicRequestHandler {
    #impl: IUnderlyingRequestHandler;`;
    const parser = new RequestHandlerParser();
    const groups = parser.getGroups();
    for (let i = 0; i < groups.length; i++) {
        const group = groups[i];
        code += `
    ${group}: IPublicRequestHandler["${group}"];`;
    }

    code += `
    constructor(impl: ValidatedRequestHandler) {
        if (!(impl instanceof ValidatedRequestHandler)) {
            throw new Error(
                'Cannot construct PublicRequestHandler '
                + 'Expecting request handler to be an '
                + 'instance of ValidatedRequestHandler; '
                + 'got otherwise.'
            );
        }
        this.#impl = impl;
        const self = this;`;

    for (let i = 0; i < groups.length; i++) {
        const group = groups[i];
        const methods = parser.getMethods(group);
        code += `
        this.${group} = {`;
        for (let j = 0; j < methods.length; j++) {
            const {
                name,
                parameters,
            } = parser.parseMethod(group, methods[j]);
            const ctxVar = parameters[0].name;
            const responseCtxType = `ResponseContext<"${group}", "${name}">`;
            const returnType = `ReturnType<RequestHandler["${group}"]["${name}"]>`;

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
                return `${p.name}: `
                    + (i === 0 ? `Omit<${p.type.alias}, "request">` : p.type.alias);
            }).join(', ');
            code += `): Promise<${returnType}> {
                return new Promise<${returnType}>((_resolve, _reject) => {
                    const responseCtx: ${responseCtxType} = {
                        response: {
                            resolve(val: ${returnType}) {
                                _resolve(val);
                            },
                            reject(val: unknown) {
                                _reject(val);
                            }
                        }
                    }
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "${capitlise(group)}_${capitlise(name)}" as const
                        }
                    };
                    self.#impl.${group}.${name}({
                        ...${ctxVar},
                        ...responseCtx,
                        ...requestCtx
                    }, ${parameters.slice(1).map(p => p.name).join(', ')});
                })
            },`;
        }
        code += `
        };`;
    }

    code += `
    }
}`;
    return { classCode: code, typesToImport };
}