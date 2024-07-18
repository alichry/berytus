/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { project } from './Project.js';
import { InterfaceDeclaration, Node } from 'ts-morph';
import { ParsedType, parseType } from './type-parser.js';

interface MethodParameters {
    name: string;
    type: ParsedType;
}

interface Method {
    name: string;
    parameters: MethodParameters[];
    returnType: ParsedType
}

export class RequestHandlerParser {
    intf: InterfaceDeclaration;

    constructor() {
        const typesFile = project.getSourceFileOrThrow(
            './src/types.ts'
        );
        this.intf =  typesFile.getInterfaceOrThrow('RequestHandler');
    }

    getGroups(): string[] {
        const props = this.intf.getProperties();
        return props.map(p => p.getName());
    }

    getMethods(group: string): string[] {
        return this.intf
            .getPropertyOrThrow(group)
            .getType()
            .getProperties()
            .map(p => p.getName());
    }

    parseMethod(groupName: string, methodName: string): Method {
        const methodSig = this.intf
            .getPropertyOrThrow(groupName)
            .getType()
            .getPropertyOrThrow(methodName)
            .getValueDeclarationOrThrow();
        if (! Node.isMethodSignature(methodSig)) {
            throw new Error(
                'Expecting ' + methodName + ' to be a ' +
                'MethodSignature, got otherwise.'
            );
        }
        const params = methodSig
            .getParameters()
            .map(pr => {
                return {
                    name: pr.getName(),
                    type: parseType(pr.getType())
                }
            });
        const parsedReturnType = parseType(methodSig.getReturnType());
        return {
            name: methodName,
            parameters: params,
            returnType: parsedReturnType
        };
    }

    *iterator() {
        const groups = this.getGroups();
        for (let i = 0; i < groups.length; i++) {
            const group = groups[i];
            const methods = this.getMethods(group);
            for (let j = 0; j < methods.length; j++) {
                const method = this.parseMethod(
                    group,
                    methods[j]
                );
                yield {
                    group,
                    method
                };
            }
        }
    }
}