/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { project } from './Project.js';
import { Type, EnumMember } from 'ts-morph';

type Atom = "any" | "string" | "number" | "boolean" | "undefined"
    | "null" | "ArrayBuffer" | "Uint8Array";

export type ParsedType = {
    type: Atom;
} | {
    type: "object";
    alias?: string;
    properties: {
        [x: string]: ParsedType;
    }
} | {
    type: "Array";
    itemType: ParsedType
} | {
    type: "union";
    options: ParsedType[]
} | {
    type: "enum";
    choices: string[] | number[]
} | {
    type: "record";
    keyType: "string" | "number";
    keyChoices?: string[] | number[]
} | {
    type: "literal";
    value: string | number | boolean;
} | {
    type: "pattern";
    regexp: RegExp
} | {
    type: "promise";
    resolveType: ParsedType
};


const typeChecker = project.getTypeChecker();

/**
 * A simple TypeScript type parser. This is primarily
 * used to parse the RequestHandler methods and
 * generate code based on the parsed types.
 * This parser is meant to be used in our use cases,
 * and not a generic type parser. It explicitly
 * does not support the following:
 *  - Promises
 *  - Classes
 *  - Intersections
 *  - Index signatures
 *
 * It supports:
 *  - String/Number/Boolean (+Literals)
 *  - Null/Undefined
 *  - Enum
 *  - Interfaces
 *  - Objects (e.g. type X = { a: hi })
 *  - Unions
 *  - "object" (e.g. type X = { a: object })
 *  - A Berytus-specific pattern defined as `custom:${string}`
 *          i.e. the glob custom:*
 *
 * any/unknown/void/never is mapped to any
 */
export const parseType = (int: Type): ParsedType => {
    if (int.isNumberLiteral() || int.isStringLiteral() || int.isBooleanLiteral()) {
        return {
            type: 'literal',
            value: int.getText()
        }
    }
    if (int.isString()) {
        return {
            type: 'string'
        };
    }
    if (int.isNumber()) {
        return {
            type: 'number'
        };
    }
    if (int.isNull()) {
        return {
            type: 'null'
        };
    }
    if (int.isUndefined()) {
        return {
            type: 'undefined'
        };
    }
    if (int.isArray()) {
        const itemType = int.getTypeArguments()[0]
        return {
            type: 'Array',
            itemType: parseType(itemType)
        }
    }
    if (int.getText() === 'Uint8Array') {
        return {
            type: 'Uint8Array'
        }
    }
    if (int.getText() === 'ArrayBuffer') {
        return {
            type: 'ArrayBuffer'
        }
    }
    if (int.getTargetType()?.getSymbol()?.getName() === 'Promise') {
        throw new Error('Promises are not supported in type parser');
    }
    if (int.getText() === 'object') {
        return {
            type: "object",
            properties: {}
        }
    }
    if (int.getText() === '`custom:${string}`') {
        return {
            type: "pattern",
            regexp: /^custom:[-_a-zA-Z0-9]+$/
        }
    }
    if (int.isObject() || int.isInterface()) {
        if (typeChecker.compilerObject.getIndexInfosOfType(int.compilerType).length > 0) {
            throw new Error(
                'Object types with an index are not supporetd. '
                + `Caught in "${int.getAliasSymbol()?.getName() || int.getText()}"`
            );
        }
        const properties: Record<string, ParsedType> = {};
        int.getProperties().forEach(prop => {
            properties[prop.getName()] =
                parseType(prop.getValueDeclarationOrThrow().getType());
        })
        return {
            type: 'object',
            alias: int.getAliasSymbol()?.getName() || int.getSymbol()?.getName(),
            properties
        }
    }
    if (int.isBoolean()) {
        return {
            type: "boolean"
        }
    }
    if (int.isEnum()) {
        const members = int.getSymbolOrThrow()
            .getValueDeclarationOrThrow() // compilerNode
            // @ts-ignore: TODO(beytus): How to get enum members from a ts-morph type?
            .getMembers() as EnumMember[];
        const choices: Array<string | number> = [];
        for (let i = 0; i < members.length; i++) {
            const val = members[i].getValue();
            if (typeof val === 'undefined') {
                throw new Error(
                    'Expecting Enum value for type to be either ' +
                    'a string or a number. Got undefined. ' +
                    'Enum: ' + int.getText()
                );
            }
            if (i > 0 && typeof val !== typeof choices[0]) {
                throw new Error(
                    'Expecting Enum value types to be uniform. '
                    + 'Got otherwise. '
                    + 'typeof 1st enum: ' + typeof choices[0]
                    + ` typeof ${i}th enum: ${typeof val}. `
                    + 'Eum: ' + int.getText()
                )
            }
            choices.push(val);
        }
        return {
            type: "enum",
            choices: choices as Array<string> | Array<number>
        }
    }
    if (int.isUnion()) {
        const options: ParsedType[] = [];
        for (let i = 0; i < int.compilerType.types.length; i++) {
            // @ts-ignore Type constructor:
            // https://github.com/dsherret/ts-morph/blob/5e208c51877b14aa05bea3ae04a4b283cf0ace60/packages/ts-morph/src/compiler/types/Type.ts#L27
            // TODO(berytus): find a better way of wrapping the native type
            const type = new Type(project._context, int.compilerType.types[i]) as Type;
            options.push(
                parseType(
                    type
                )
            );
        }
        return {
            type: 'union',
            options
        };
    }
    if (int.isIntersection()) {
        throw new Error('Intersection types not supported');
    }
    if (int.isClass()) {
        throw new Error("Class types not supported");
    }
    if (int.isAny() || int.isUnknown() || int.isVoid() || int.isNever()) {
        return { type: "any" };
    }
    throw new Error(
        'Unrecognised/Unsupportted type for '
        + (int.getAliasSymbol()?.getName() || int.getText())
    );
}