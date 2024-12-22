#!/usr/bin/env node --loader ts-node/esm
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { resolve } from "node:path";
import { readFile, writeFile } from "node:fs/promises";
import { InterfaceDeclaration, Type as tsType, SourceFile, Project as TSProject } from 'ts-morph';
import { ParsedType, ParseOptions, parseType } from '../request-handler/type-parser.js';

const project = new TSProject({
    tsConfigFilePath: resolve('./tsconfig.json'),
});

const dtsFile = resolve("./src/generated/berytus.web.d.ts");

const fieldValueTypes = new Set<string>();

const listFields = (typesFile: SourceFile) => {
    const baseFieldName = "BerytusField";
    return typesFile.getInterfaces().map(intf => {
        if (! intf.getExtends().some(f => f.getText() === baseFieldName)) {
            return null;
        }
        return intf;
    }).filter(d => !!d);
}

const interfaceEnsureUnionsAreAliasd = (
    typesFile: SourceFile,
    unionAliasGenerator: ParseOptions['unionAliasGenerator'],
    int: InterfaceDeclaration,
    updater: (oldIntText: string, newIntText: string) => void
) => {
    const shouldSkipType = (type: tsType) => {
        const name = type.getAliasSymbol()?.getName()
            || type.getSymbol()?.getName();
        return name === 'ArrayBuffer' ||
            name === 'ArrayBufferView';
    }

    const oldText = int.getText();
    int.getProperties().forEach((propSig) => {
        const propType = propSig.getType();
        if (propType.isInterface() && !shouldSkipType(propType)) {
            interfaceEnsureUnionsAreAliasd(
                typesFile,
                unionAliasGenerator,
                typesFile.getInterfaceOrThrow(
                    propType.getAliasSymbol()?.getName()
                    || propType.getSymbol()?.getName()!
                ),
                updater
            );
        }
        if (propType.isIntersection()) {
            throw new Error("Intersection types are not allowed.");
        }
        if (! propType.isUnion()) {
            return;
        }
        propType.getUnionTypes().forEach(ut => {
            if (!ut.isInterface() || shouldSkipType(ut)) {
                return;
            }
            interfaceEnsureUnionsAreAliasd(
                typesFile,
                unionAliasGenerator,
                typesFile.getInterfaceOrThrow(
                    ut.getAliasSymbol()?.getName()
                    || ut.getSymbol()?.getName()!
                ),
                updater
            );
        });
        const parsedType = parseType(propType, { unionAliasGenerator });
        if (!("alias" in parsedType)) {
            // alias is not needed, skip this prop.
            return;
        }
        const propTypeText = propSig.getType().getText();
        propSig.set({
            name: propSig.getName(),
            type: propTypeText !== parsedType.alias
                ? parsedType.alias : propTypeText
        });
    });
    const newText = int.getText();
    updater(oldText, newText);
}

const ensureUnionsAreAliases = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    const unionAliases: Array<{ alias: string, def: string }> = [];
    const unionAliasGenerator = (options: ParsedType[]): string => {
        const aliasesAndDefs = options.map(opt => {
            let optAlias: string;
            let optDef: string;
            if ("alias" in opt) {
                optAlias = opt.alias;
                optDef = opt.alias;
            } else if (opt.type === "literal") {
                optAlias = `Literal_${typeof opt.value}_${opt.value}`;
                optDef = String(opt.value);
            } else if (opt.type === "pattern") {
                optAlias = `Pattern`;
                optDef = "string";
            } else {
                optAlias = opt.type;
                optDef = opt.type;
            }
            return { optAlias, optDef };
        });
        const alias = aliasesAndDefs.map(a => a.optAlias).join("Or");
        const def = `export type ${alias} = ` + aliasesAndDefs.map(d => d.optDef).join(" |\n\t") + ";\n";
        if (! unionAliases.some(uA => uA.alias === alias)) {
            unionAliases.push({
                alias,
                def
            });
        }
        return alias;
    }
    let dts = await readFile(dtsFile, { encoding: "utf8" });

    [
        ...listFields(typesFile),
        typesFile.getInterfaceOrThrow("BerytusUserAttributeDefinition")
    ].forEach(int => {
        interfaceEnsureUnionsAreAliasd(
            typesFile,
            unionAliasGenerator,
            int,
            (oldText, newText) => {
                dts =  dts.replace(oldText, newText);
            }
        );
    });

    await writeFile(
        dtsFile,
        dts + unionAliases.map(a => a.def).join("\n")
    );
}

const generateFieldTypeEnum = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    const fieldType = typesFile.getTypeAlias("BerytusFieldType");
    if (! fieldType) {
        throw new Error("Missing BerytusFieldType enum in WebIDL");
    }
    const parsedFieldType = parseType(fieldType.getType());
    if (!("options" in parsedFieldType)) {
        throw new Error("Missing BerytusFieldType enum in WebIDL");
    }
    const enumName = 'EBerytusFieldType'
    const fieldTypeEnum: string = `export enum ${enumName} {
${parsedFieldType.options.map(o => {
    if (!("value" in o)) {
        throw new Error("Option should be a literal value");
    }
    return `\t${o.value} = ${JSON.stringify(o.value)}`
}).join(",\n")}
}\n`
    const dts: string = await readFile(dtsFile, { encoding: "utf8" });
    await writeFile(dtsFile, dts + fieldTypeEnum);
    return enumName;
}

const generateFieldUnion = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    const fieldNames = listFields(typesFile).map(intf => {
        return intf.getSymbol()!.getName()!;
    });
    const dts: string = await readFile(dtsFile, { encoding: "utf8" });
    await writeFile(
        dtsFile,
        dts + `export type BerytusFieldUnion = ${fieldNames.join("\n\t| ")};\n`
    );
}

const generateFieldValueUnion = async () => {
    const dts: string = await readFile(dtsFile, { encoding: "utf8" });
    await writeFile(
        dtsFile,
        dts + `export type BerytusFieldValueUnion = ${Array.from(fieldValueTypes).join("\n\t| ")};\n`
    );
}

const generateFieldProperties = async () => {
    fieldValueTypes.clear();
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    listFields(typesFile).forEach(intf => {
        const oldIntfText = intf.getText();
        const fieldName = intf.getSymbol()?.getName();
        const [_, fieldType] = fieldName!.match(/^Berytus(.*)Field$/)!;
        const ctor = intf.getConstructSignatureOrThrow(() => true);
        const options = ctor.getParameter("options");
        const value = ctor.getParameter("desiredValue");

        const optionsTypeText: string = options!.getType().getText();
        let valueTypeText: string;

        if (! value) {
            // SecurePassword does not define a value argument,
            // but we infer its vallue type.
            valueTypeText = `${fieldName}Value`;
        } else {
            const valueType = value.getType();
            if (! valueType.isUnion()) {
                valueTypeText = value.getType().getText();
            } else {
                valueTypeText = valueType.getUnionTypes().map(t => {
                    if (t.isUndefined()) {
                        // remove undefined, we want to extract field value
                        // type, not whether it's optional or not.
                        // @ts-ignore
                        return;
                    }
                    return t.getText();
                }).filter(d => !!d).join(" | ");
            }
        }
        fieldValueTypes.add(valueTypeText);

        intf.set({
            properties: [
                {
                    name: "type",
                    type: `EBerytusFieldType.${fieldType}`
                },
                {
                    name: "options",
                    type: optionsTypeText
                },
                {
                    name: "value",
                    type: valueTypeText + " | null"
                }
            ]
        });
        dts = dts.replace(oldIntfText, intf.getText());
    });
    await writeFile(dtsFile, dts);
}

const deleteFunctions = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    typesFile.getInterfaces().forEach(intf => {
        const oldIntfText = intf.getText();
        intf.getConstructSignatures().forEach(c => c.remove());
        intf.getCallSignatures().forEach(c => c.remove());
        intf.getMethods().forEach(m => m.remove());
        dts = dts.replace(oldIntfText, intf.getText());
    });
    await writeFile(dtsFile, dts);
}

/**
 * WebIDL defines "object" as the type for packet's parameters.
 * This gets mapped to "any" by webidl-dts-gen which is no good.
 */
const correctPacketParametersAttribute = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    const intf = typesFile.getInterfaceOrThrow("BerytusEncryptedPacket");
    const origText = intf.getText();
    const parameterType = intf.getConstructSignatures()[0].getParameters()[0].getType();
    intf.getProperty("parameters")!.set({
        name: "parameters",
        type: parameterType.getAliasSymbol()?.getName()
            || parameterType.getSymbol()?.getName()
    });

    dts = dts.replace(origText, intf.getText());
    await writeFile(
        dtsFile,
        dts
    );
}

const generate = async () => {
    await generateFieldTypeEnum();
    await generateFieldProperties();
    await ensureUnionsAreAliases();
    await correctPacketParametersAttribute();
    await deleteFunctions();
    await generateFieldUnion();
    await generateFieldValueUnion();

}

generate();