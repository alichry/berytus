import { RequestHandlerParser } from "./RequestHandlerParser.js";
import type { ParsedType } from "./type-parser.js";
import { capitalise, UniqueArray } from "./dom-proxy.js";

export const generateWebExtsSchema = async () => {
    const h = new RequestHandlerParser();
    const typeIterator = h.typeIterator();
    let item = typeIterator.next();
    let parameters: Array<{ name: string; parsedType: ParsedType }> = [];
    const apiGenerator = new WebExtsApiSchemaGenerator();
    while (!item.done) {
        const {
            parsedType,
            source,
            method,
            group,
            paramName
        } = item.value;

        if (source === "parameter") {
            parameters.push({
                name: paramName,
                parsedType: parsedType
            });
        } else {
            apiGenerator.addHandlerMethod(
                group,
                method.name,
                parameters
            );
            parameters.splice(0, parameters.length);
        }
        item = typeIterator.next();
    }
    const enumGenerator = new WebExtsEnumSchemaGenerator();
    apiGenerator.enums.forEach(e => {
        enumGenerator.defineEnum(e.parsedType);
    })

    return [apiGenerator.schema, ...enumGenerator.schema];
}

export interface ValueSchemaEntry {
    value: string | number | boolean;
}

export interface TypeSchemaEntry {
    type: "object" | "string" | "number" | "boolean" | string;
    optional?: true;
}

export interface RefSchemaEntry {
    $ref: string;
    optional?: true;
}

export interface ArraySchemaEntry {
    type: "array";
    optional?: true;
    description?: string;
    items: SchemaDef;
}

export interface ArrayBufferSchemaEntry {
    type: "object";
    isInstanceOf: "ArrayBuffer"
    optional?: true;
}

export type ObjectAttributeSchemaEntry = ValueSchemaEntry
    | RefSchemaEntry
    | EnumSchemaEntry
    | TypeSchemaEntry;

export type ObjectFunctionParameterEntry = {
    name: string, type: string
} | {
    name: string, $ref: string
};

export interface ObjectFunctionSchemaEntry {
    type: "function";
    async?: boolean;
    parameters?: Array<ObjectFunctionParameterEntry>;
}

export interface ObjectSchemaEntry {
    type: "object";
    optional?: true;
    properties?: Record<string, SchemaDef>;
    isInstanceOf?: string;
}

export interface EnumSchemaEntry extends TypeSchemaEntry {
    type: "number" | "string";
    enum: Array<string | number>;
}

export interface ChoicesSchemaEntry {
    choices: SchemaDef[];
}

export type SchemaDef = ValueSchemaEntry
    | TypeSchemaEntry
    | ArrayBufferSchemaEntry
    | RefSchemaEntry
    | ObjectFunctionSchemaEntry
    | ObjectSchemaEntry
    | ChoicesSchemaEntry;

interface IDef {
    id: string;
    schema: SchemaDef;
}

type ObjectAttribute = ObjectAttributeSchemaEntry & {
    name: string;
}

interface ObjectFunction extends ObjectFunctionSchemaEntry {
    name: string;
}

class ObjectDef implements IDef {
    id: string;
    attrs: Array<ObjectAttribute> = [];
    fns: Array<ObjectFunction> = [];

    constructor(id: string) {
        this.id = id;
    }

    get schema() {
        return {
            id: this.id,
            type: "object",
            properties: this.properties
        }
    }

    get properties() {
        const res: Record<
            string,
            ObjectFunctionSchemaEntry | ObjectAttributeSchemaEntry
        > = {};
        this.fns.forEach(fn => {
            const entry: ObjectFunctionSchemaEntry = {
                type: "function",
                async: fn.async
            };
            if (fn.parameters) {
                entry.parameters = fn.parameters.map(p => {
                    if ("type" in p) {
                        return {
                            name: p.name,
                            type: p.type
                        };
                    }
                    return {
                        name: p.name,
                        $ref: p.$ref
                    };
                })
            }
            res[fn.name] = entry;
        });
        this.attrs.forEach(attr => {
            let entry: ObjectAttributeSchemaEntry;
            if ("$ref" in attr) {
                entry = {
                    "$ref": attr.$ref
                };
            } else if ("value" in attr) {
                entry = {
                    "value": attr.value
                };
            } else if ("enum" in attr) {
                entry = {
                    type: attr.type,
                    enum: attr.enum
                };
                if (attr.optional !== undefined) {
                    entry.optional = attr.optional;
                }
            } else {
                entry = {
                    type: attr.type
                };
                if (attr.optional !== undefined) {
                    entry.optional = attr.optional;
                }
            }
            res[attr.name] = entry;
        });
        return res;
    }

    addFunction(fn: ObjectFunction) {
        this.fns.push(fn);
    }

    addAttribute(attr: ObjectAttribute) {
        this.attrs.push(attr);
    }
}

class UnionDef implements IDef {
    id: string;
    choices: Array<SchemaDef>;

    constructor(
        id: string,
        choices: Array<SchemaDef> = []
    ) {
        this.id = id;
        this.choices = choices;
    }

    addChoice(choice: SchemaDef) {
        this.choices.push(choice);
    }

    get schema(): ChoicesSchemaEntry {
        return {
            choices: this.choices
        };
    }
}

class ApiFunctionDef {
    name: string;
    parameters: Array<ObjectFunctionParameterEntry>;

    constructor(
        id: string,
        parameters: Array<ObjectFunctionParameterEntry>
    ) {
        this.name = id;
        this.parameters = parameters;
    }

    get schema() {
        return {
            id: this.name,
            type: "function",
            parameters: this.parameters
        }
    }
}

interface ParameterArg {
    name: string;
    parsedType: ParsedType
}

class WebExtsApiSchemaGenerator {
    defs: UniqueArray<IDef> = new UniqueArray<IDef>();
    groups: Array<ObjectDef> = [];
    methods: Array<ApiFunctionDef> = []
    enums: UniqueArray<{ id: string; parsedType: ParsedType}> = new UniqueArray();

    getOrCreateHandlerGroup(group: string): ObjectDef {
        const groupDefId = `${group}RequestHandler`;
        let groupDef = this.groups.find(d => d.id === groupDefId);
        if (! groupDef) {
            groupDef = new ObjectDef(groupDefId);
            this.groups.push(groupDef);
        }
        return groupDef;
    }

    addApiMethod(
        method: string,
        parsedParameters: Array<ParameterArg>
    ) {
        const parameters = this.defineParameters(parsedParameters);
        this.methods.push(new ApiFunctionDef(method, parameters));
    }

    addHandlerMethod(
        group: string,
        method: string,
        parsedParameters: Array<ParameterArg>
    ) {
        group = capitalise(group);
        let groupDef = this.getOrCreateHandlerGroup(group);
        const parameters = this.defineParameters(parsedParameters);
        groupDef.addFunction({
            name: method,
            type: "function",
            parameters
        });
    }

    defineParameters(
        parsedParameters: Array<ParameterArg>
    ): ObjectFunctionParameterEntry[] {
        return parsedParameters.map(p => {
            const paramDef = this.defineType(p.parsedType);
            if ("value" in paramDef) {
                throw new Error(
                    'Literal values cannot be in parameter defs'
                );
            }
            return {
                name: p.name,
                ...paramDef
            }
        });
    }

    defineType(parsedType: ParsedType): ValueSchemaEntry | TypeSchemaEntry | RefSchemaEntry {
        if (parsedType.type === "object") {
            return this.defineObject(parsedType);
        }
        if (parsedType.type === "enum") {
            return this.getEnumSchemaEntry(parsedType);
        }
        if (parsedType.type === "literal") {
            return this.getLiteralSchemaEntry(parsedType);
        }
        if (parsedType.type === "union") {
            return this.defineUnion(parsedType);
        }
        if (parsedType.type === "Array") {
            return this.getArraySchemaEntry(parsedType);
        }
        if (
            parsedType.type === "undefined"
            || parsedType.type === "null"
            || parsedType.type === "any"
        ) {
            // TODO(berytus): Think about null and undefined?
            return this.getAnyEntry(parsedType);
        }
        if (parsedType.type === "ArrayBuffer") {
            return this.getArrayBufferEntry(parsedType);
        }
        if (
            parsedType.type === "boolean" ||
            parsedType.type === "number" ||
            parsedType.type === "string" ||
            parsedType.type === "pattern"
        ) {
            return this.getPrimitiveEntry(parsedType);
        }
        throw new Error('Unsupported type ' + parsedType.type);
    }

    defineObject(parsedType: ParsedType): RefSchemaEntry {
        if (parsedType.type !== "object") {
            throw new Error(
                "Wrong type passed to defineStruct"
            );
        }
        const { properties, alias } = parsedType;
        const objectDef = new ObjectDef(
            alias
        );
        for (const prop in properties) {
            const parsedType = properties[prop];
            const attributeDef = this.defineType(parsedType);
            objectDef.addAttribute({
                ...attributeDef,
                name: prop,
            });
        }
        this.defs.push(objectDef);
        return {
            $ref: objectDef.id,
            optional: parsedType.optional
        };
    }

    defineUnion(parsedType: ParsedType): RefSchemaEntry {
        if (parsedType.type !== "union") {
            throw new Error(
                "Wrong type passed to defineUnion"
            );
        }
        const subTypes = parsedType.options.map(pt => this.defineType(pt));
        const unionDef = new UnionDef(parsedType.alias, subTypes);
        this.defs.push(unionDef);
        if (parsedType.optional === undefined) {
            return { $ref: unionDef.id };
        }
        return {
            $ref: unionDef.id,
            optional: parsedType.optional
        };
    }

    getLiteralSchemaEntry(parsedType: ParsedType): ValueSchemaEntry {
        if (parsedType.type !== 'literal') {
            throw new Error(
                "Wrong type passed to getLiteralSchemaEntry"
            );
        }
        switch (typeof parsedType.value) {
            case "number":
            case "string":
            case "boolean":
                return {
                    value: parsedType.value
                };
            default:
                throw new Error("Unrecognised type literal " + parsedType.value);
        }
    }

    getArraySchemaEntry(parsedType: ParsedType): ArraySchemaEntry {
        if (parsedType.type !== "Array") {
            throw new Error(
                "Wrong type passed to getLiteralSchemaEntry"
            );
        }
        const itemType = this.defineType(parsedType.itemType);
        if (parsedType.optional === undefined) {
            return {
                type: "array",
                items: itemType
            };
        }
        return {
            type: "array",
            items: itemType,
            optional: parsedType.optional
        };
    }

    getEnumSchemaEntry(parsedType: ParsedType): EnumSchemaEntry {
        if (parsedType.type !== "enum") {
            throw new Error(
                "Wrong type passed to getEnumSchemaEntry"
            );
        }
        const enumValues = parsedType.choices.map(a => a.value);
        const enumType = parsedType.choices.reduce<"string" | "number">((prev, curr) => {
            const type = typeof curr.value;
            if (type !== "number" && type !== "string") {
                throw new Error('Invalid enum value: ' + curr.value);
            }
            if (type !== prev) {
                throw new Error('Invalid enum value: ' + curr.value);
            }
            return type;
        }, typeof parsedType.choices[0].value as "string" | "number");
        this.enums.push({ id: parsedType.alias, parsedType });
        if (parsedType.optional === undefined) {
            return {
                enum: enumValues,
                type: enumType
            };
        }
        return {
            enum: enumValues,
            type: enumType,
            optional: parsedType.optional
        };
    }

    getPrimitiveEntry(parsedType: ParsedType): TypeSchemaEntry {
        switch (parsedType.type) {
            case "boolean":
            case "number":
            case "string":
            case "pattern":
                const typeName = parsedType.type === "pattern"
                    ? "string" : parsedType.type;
                if (parsedType.optional === undefined) {
                    return {
                        type: parsedType.type
                    };
                }
                return {
                    type: parsedType.type,
                    optional: parsedType.optional
                };
            default:
                throw new Error("Wrong type passed to getPrimitiveEntry");
        }
    }

    getAnyEntry(parsedType: ParsedType): TypeSchemaEntry {
        if (parsedType.optional === undefined) {
            return {
                type: "any"
            };
        }
        return {
            type: "any",
            optional: parsedType.optional
        }
    }

    getArrayBufferEntry(parsedType: ParsedType): ArrayBufferSchemaEntry {
        if (parsedType.optional === undefined) {
            return {
                type: "object",
                isInstanceOf: "ArrayBuffer"
            };
        }
        return {
            type: "object",
            isInstanceOf: "ArrayBuffer",
            optional: parsedType.optional
        }
    }

    get schema() {
        return {
            namespace: "berytus",
            types: [...this.defs, ...this.groups].map(d => {
                return {
                    id: d.id,
                    ...d.schema
                }
            }),
            properties: [],
            functions: this.methods.map(m => {
                return {
                    name: m.name,
                    ...m.schema
                }
            })
        };
    }
}

class WebExtsEnumSchemaGenerator {
    enums: UniqueArray<{ id: string; choices: Array<{ name: string; value: string | number }> }>

    constructor() {
        this.enums = new UniqueArray();
    }

    defineEnum(parsedType: ParsedType) {
        if (parsedType.type !== 'enum') {
            throw new Error('Invalid parsed type passed to defineEnum');
        }

        this.enums.push({
            id: parsedType.alias,
            choices: [...parsedType.choices, ...parsedType.choices.map(c => {
                return {
                    name: String(c.value),
                    value: c.name
                };
            })]
        });
    }

    get schema() {
        return this.enums.map(e => {
            const properties: Record<string, ValueSchemaEntry> = {};
            e.choices.forEach(c => {
                properties[c.name] = { value: c.value };
            });
            return {
                namespace: `berytus.enum.${e.id}`,
                properties
            }
        })
    }
}