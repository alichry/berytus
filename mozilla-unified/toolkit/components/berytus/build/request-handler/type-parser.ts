/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { project } from './Project.js';
import { Type, EnumMember, ts } from 'ts-morph';

type Atom = "any" | "string" | "number" | "boolean" | "undefined"
    | "null" | "ArrayBuffer" | "ArrayBufferView";

export type ParsedType = {
    type: Atom;
    optional?: true;
} | {
    type: "object";
    optional?: true;
    alias: string;
    properties: {
        [x: string]: ParsedType;
    }
} | {
    type: "Array";
    optional?: true;
    itemType: ParsedType
} | {
    type: "union";
    optional?: true;
    alias: string; // each union must be defined using an alias, e.g. type XUnion = 1 | 2 | 3 ... interface Q { x: XUnion }
    options: ParsedType[]
} | {
    type: "enum";
    optional?: true;
    alias: string;
    choices: Array<{ name: string; value: string | number}>;
} | {
    type: "record";
    optional?: true;
    keyType: "string" | "number";
    keyChoices?: string[] | number[]
} | {
    type: "literal";
    // TODO(beytus): I don't think optional is appropriate here.
    optional?: true;
    value: string | number | boolean;
} | {
    type: "pattern";
    optional?: true;
    regexp: RegExp
} | {
    type: "promise";
    optional?: true;
    resolveType: ParsedType
};

export interface ParseOptions {
    unionAliasGenerator?(options: ParsedType[]): string;
}

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
 *  - "object" (e.g. type X = { a: object })
 *
 * It supports:
 *  - String/Number/Boolean (+Literals)
 *  - Null/Undefined
 *  - Enum
 *  - Interfaces
 *  - Objects (e.g. type X = { a: hi })
 *  - Unions
 *  - A Berytus-specific pattern defined as `custom:${string}`
 *          i.e. the glob custom:*
 *
 * any/unknown/void/never is mapped to any
 */
export const parseType = (int: Type, opts: ParseOptions = {}): ParsedType => {
    if (int.isNumberLiteral() || int.isStringLiteral() || int.isBooleanLiteral()) {
        const value = int.getLiteralValueOrThrow(); //int.getText()
        if (typeof value !== "string" && typeof value !== "number") {
            throw new Error("Bad literal value; bigints are not supported");
        }
        return {
            type: 'literal',
            value
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
            itemType: parseType(itemType, opts)
        }
    }
    // if (int.getText() === 'Uint8Array') {
    //     return {
    //         type: 'Uint8Array'
    //     }
    // }
    if (int.getText() === 'ArrayBuffer') {
        return {
            type: 'ArrayBuffer'
        }
    }
    if (int.getText() === 'ArrayBufferView') {
        return { type: 'ArrayBufferView' }
    }
    if (int.getTargetType()?.getSymbol()?.getName() === 'Promise') {
        throw new Error('Promises are not supported in type parser');
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
        const alias = int.getAliasSymbol()?.getName() || int.getSymbol()?.getName()
        if (! alias) {
            throw new Error('Missing alias for object! Ensure every object type is defined using an alias.');
        }
        const properties: Record<string, ParsedType> = {};
        int.getProperties().forEach(prop => {
            properties[prop.getName()] =
                parseType(prop.getValueDeclarationOrThrow().getType()!, opts);
        })
        return {
            type: 'object',
            alias,
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
        const choices: Array<{ name: string; value: string | number; }> = [];
        for (let i = 0; i < members.length; i++) {
            const name = members[i].getName();
            const val = members[i].getValue();
            if (typeof val === 'undefined') {
                throw new Error(
                    'Expecting Enum value for type to be either ' +
                    'a string or a number. Got undefined. ' +
                    'Enum: ' + int.getText()
                );
            }
            if (i > 0 && typeof val !== typeof choices[0].value) {
                throw new Error(
                    'Expecting Enum value types to be uniform. '
                    + 'Got otherwise. '
                    + 'typeof 1st enum: ' + typeof choices[0]
                    + ` typeof ${i}th enum: ${typeof val}. `
                    + 'Eum: ' + int.getText()
                )
            }
            choices.push({ name, value: val });
        }
        const alias = int.getAliasSymbol()?.getName()!;
        return {
            type: "enum",
            alias,
            choices: choices
        }
    }
    if (int.isUnion()) {
        const options: ParsedType[] = [];
        /**
         * When using int.compilerType.origin.types,
         *  the alias symbol is set in the following instance:
         * interface Test {
         *    value?: Value
         * }
         * type Value = ArrayBuffer | string;
         *
         * ... but not when using int.compilerType.types
         */
        const compilerUnionMembers: ts.Type[] = ((int.compilerType as any).origin?.types)
            || int.compilerType.types;
        for (let i = 0; i < compilerUnionMembers.length; i++) {
            // @ts-ignore Type constructor:
            // https://github.com/dsherret/ts-morph/blob/5e208c51877b14aa05bea3ae04a4b283cf0ace60/packages/ts-morph/src/compiler/types/Type.ts#L27
            // TODO(berytus): find a better way of wrapping the native type
            const type = new Type(project._context, compilerUnionMembers[i]) as Type;
            options.push(
                parseType(
                    type,
                    opts
                )
            );
        }
        const undefinedIndex = options.findIndex(
            o => o.type === "undefined"
        );
        if (options.length === 2 && undefinedIndex !== -1) {
            return {
                optional: true,
                ...options[undefinedIndex + 1 % 2]
            };
        }
        let alias = int.getAliasSymbol()?.getName() || int.getSymbol()?.getName();
        if (! alias) {
            if (! opts.unionAliasGenerator) {
                throw new Error(
                    'Missing alias for union! Ensure every union type is defined using an alias.'
                    + 'Type:' + int.getText()
                );
            }
            alias = opts.unionAliasGenerator(options);
        }
        return {
            type: 'union',
            alias,
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