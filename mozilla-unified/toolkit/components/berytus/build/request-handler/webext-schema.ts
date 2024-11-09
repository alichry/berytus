import { RequestHandlerParser } from "./RequestHandlerParser.js";
import type { ParsedType } from "./type-parser.js";
import { capitalise, UniqueArray } from "./dom-proxy.js";

const decaptialise = (str: string) => {
    return str.charAt(0).toLowerCase() + str.substring(1);
}

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
    });
    const requestHandlerPropertiesDef: Record<string, RefSchemaEntry> =  {};
    apiGenerator.groups.forEach(group => {
        requestHandlerPropertiesDef[decaptialise(group.id).replace('RequestHandler', '')] = {
            $ref: group.id
        }
    });
    apiGenerator.addTypeDef({
        id: "RequestHandler",
        schema: {
            type: "object",
            properties: requestHandlerPropertiesDef
        }
    });
    apiGenerator.addApiMethod(
        'registerRequestHandler',
        [
            {
                name: "requestHandler",
                $ref: 'RequestHandler'
            }
        ],
        true
    );
    apiGenerator.addApiMethod(
        'unregisterRequestHandler',
        [],
        true
    );
    apiGenerator.addApiMethod(
        'resolveRequest',
        [
            {
                name: "requestId",
                type: "string"
            },
            {
                name: "value",
                type: "any"
            }
        ],
        true
    );
    apiGenerator.addApiMethod(
        'rejectRequest',
        [
            {
                name: "requestId",
                type: "string"
            },
            {
                name: "reason",
                type: "any"
            }
        ],
        true
    );
    return [
        {
            "namespace": "manifest",
            "types": [
                {
                    "$extend": "OptionalPermission",
                    "choices": [
                        {
                            "type": "string",
                            "enum": ["berytus"]
                        }
                    ]
                }
            ]
        },
        apiGenerator.schema,
        ...enumGenerator.schema
    ];
}

export interface ValueSchemaEntry {
    value: string | number | boolean;
}

export interface TypeSchemaEntry {
    type: "object" | "string" | "number" | "boolean" | string;
    optional?: true;
    description?: string;
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
    | TypeSchemaEntry
    | ArraySchemaEntry;

export type ObjectFunctionParameterEntry = {
    name: string; type: string; optional?: true
} | {
    name: string, $ref: string; optional?: true
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

export type SchemaDef =
    TypeSchemaEntry
    | ArrayBufferSchemaEntry
    | RefSchemaEntry
    | ObjectFunctionSchemaEntry
    | ObjectSchemaEntry
    | ChoicesSchemaEntry
    | EnumSchemaEntry;

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
                const { $ref, optional } = attr;
                entry = {
                    $ref: $ref,
                    ...(optional ? { optional } : undefined)
                };
            } else if ("value" in attr) {
                entry = {
                    value: attr.value
                };
            } else if ("enum" in attr) {
                const {
                    type,
                    enum: _enum,
                    optional,
                    description
                } = attr;
                entry = {
                    type: type,
                    enum: _enum,
                    ...(optional ? { optional } : undefined),
                    ...(description ? { description } : undefined)
                };
                if (attr.optional !== undefined) {
                    entry.optional = attr.optional;
                }
            } else if ("items" in attr) {
                const { type, items, optional, description } = attr;
                entry = {
                    type,
                    ...(optional ? { optional } : undefined),
                    ...(description ? { description } : undefined),
                    items
                };
            } else {
                const { type, optional, description } = attr;
                entry = {
                    type,
                    ...(optional ? { optional } : undefined),
                    ...(description ? { description } : undefined)
                };
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
    async: boolean;

    constructor(
        id: string,
        parameters: Array<ObjectFunctionParameterEntry>,
        async: boolean
    ) {
        this.name = id;
        this.parameters = parameters;
        this.async = async;
    }

    get schema() {
        return {
            type: "function",
            ...(this.async ? { async: this.async } : undefined ),
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

    addTypeDef(def: IDef) {
        this.defs.push(def);
    }

    addApiMethod(
        method: string,
        parameters: Array<ObjectFunctionParameterEntry>,
        async: boolean
    ) {
        this.methods.push(new ApiFunctionDef(
            method, parameters, async
        ));
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
        const [literalsAsEnum, otherSubTypes] = subTypes.reduce((res, curr) => {
            let [literalEnum, otherSubTypes] = res;
            if (!("value" in curr)) {
                otherSubTypes.push(curr);
                return res;
            }
            const valueType = typeof curr.value;
            switch (valueType) {
                case "boolean":
                case "number":
                    if (! otherSubTypes.find(s => "type" in s &&  s.type === valueType)) {
                        otherSubTypes.push({ type: valueType });
                    }
                    console.warn(
                        `Non-string Literal Value (${curr.value}) encountered when defining a Union`
                    );
                    break;
                case "string":
                    if (! literalEnum) {
                        res[0] = literalEnum = {
                            type: valueType,
                            enum: []
                        };
                    }
                    literalEnum.enum.push(curr.value as string | number);
                    break;
                default:
                    throw new Error("Invalid literal type.");
            }
            return res;
        }, [undefined, []] as [EnumSchemaEntry | undefined, Array<TypeSchemaEntry | RefSchemaEntry>]);
        const unionDef = new UnionDef(parsedType.alias, ! literalsAsEnum ? otherSubTypes : [literalsAsEnum, ...otherSubTypes]);
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
        // @ts-ignore JSON.parse accepts a number/boolean type
        const value = JSON.parse(parsedType.value);
        switch (typeof value) {
            case "number":
            case "string":
            case "boolean":
                return {
                    value
                };
            default:
                throw new Error(
                    "Unrecognised type literal "
                    + parsedType.value + " -- " + value
                );
        }
    }

    getArraySchemaEntry(parsedType: ParsedType): ArraySchemaEntry {
        if (parsedType.type !== "Array") {
            throw new Error(
                "Wrong type passed to getLiteralSchemaEntry"
            );
        }
        const itemType = this.defineType(parsedType.itemType);
        if ("value" in itemType) {
            throw new Error("Literals cannot be in arrays.");
        }
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
        const description = `See berytus.enums.${parsedType.alias} for mappings`;
        return WebExtsApiSchemaGenerator.#maybeIncludeOptional<EnumSchemaEntry>(
            {
                enum: enumValues,
                type: enumType,
                description,
            },
            parsedType.optional
        );
    }

    getPrimitiveEntry(parsedType: ParsedType): TypeSchemaEntry {
        switch (parsedType.type) {
            case "boolean":
            case "number":
            case "string":
            case "pattern":
                let typeName = parsedType.type;
                return WebExtsApiSchemaGenerator.#maybeIncludeOptional<TypeSchemaEntry>(
                    {
                        type: typeName === "pattern" ? "string" : typeName,
                        ...(
                            parsedType.type === "pattern"
                            ? { pattern: parsedType.regexp.source }
                            : undefined
                        )
                    },
                    parsedType.optional
                );
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
            permissions: ["berytus"],
            types: [...this.defs, ...this.groups].map(d => {
                return {
                    id: d.id,
                    ...d.schema
                }
            }),
            properties: {},
            functions: this.methods.map(m => {
                return {
                    name: m.name,
                    ...m.schema
                }
            })
        };
    }

    static #maybeIncludeOptional<T extends object>(obj: T, optional?: boolean): T | T & { optional: boolean } {
        if (optional) {
            return {
                ...obj,
                optional
            } as const;
        }
        return obj;
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