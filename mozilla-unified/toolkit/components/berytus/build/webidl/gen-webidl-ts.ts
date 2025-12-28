#!/usr/bin/env node --loader ts-node/esm
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { resolve } from "node:path";
import { readFile, writeFile } from "node:fs/promises";
import { InterfaceDeclaration, Type as tsType, SourceFile, Project as TSProject } from 'ts-morph';
import { ParsedType, ParseOptions, parseType, retrieveRecordArgumentTypes } from '../request-handler/type-parser.js';

const createProject = () => new TSProject({
    tsConfigFilePath: resolve('./tsconfig.json')
});

const project = createProject();

const dtsFile = resolve("./src/generated/berytus.web.d.ts");

const fieldValueTypes = new Set<string>();

const listFields = (
    typesFile: SourceFile = project.getSourceFileOrThrow(dtsFile)
) => {
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
        if (propType.isObject() && propType.getAliasSymbol()?.getName() === 'Record') {
            const { keyType, valueType } = retrieveRecordArgumentTypes(propType);
            [keyType, valueType].forEach(tt => {
                if (tt.isInterface() && !shouldSkipType(tt)) {
                    interfaceEnsureUnionsAreAliasd(
                        typesFile,
                        unionAliasGenerator,
                        typesFile.getInterfaceOrThrow(
                            tt.getAliasSymbol()?.getName()
                            || tt.getSymbol()?.getName()!
                        ),
                        updater
                    );
                }
            });
            const parsedKeyType = parseType(keyType, { unionAliasGenerator });
            let keyTypeText = keyType.getText();
            if ("alias" in parsedKeyType) {
                keyTypeText = parsedKeyType.alias;
            }
            const parsedValueType = parseType(valueType, { unionAliasGenerator });
            let valueTypeText = valueType.getText();
            if ("alias" in parsedValueType) {
                valueTypeText = parsedValueType.alias;
            }
            propSig.set({
                name: propSig.getName(),
                type: `Record<${keyTypeText}, ${valueTypeText}>`
            });
            return;
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
        if (propTypeText === parsedType.alias) {
            // no change needed.
            return;
        }
        propSig.set({
            name: propSig.getName(),
            type: parsedType.alias
        });
    });
    const newText = int.getText();
    updater(oldText, newText);
}

const ensureUnionsAreAliases = async (list: Array<InterfaceDeclaration | string | RegExp>) => {
    const typesFile = createProject().getSourceFileOrThrow(
        dtsFile
    );
    const unionAliases: Array<{ alias: string, def: string }> = [];
    const unionAliasGenerator = (options: ParsedType[]): string => {
        const getAliasAndDef = (opt: ParsedType) => {
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
            } else if (opt.type === "record") {
                const valueOpt = getAliasAndDef(opt.valueType);
                optAlias = `Record_${opt.keyType}_${valueOpt.optAlias}`;
                optDef = `Record<${opt.keyType}, ${valueOpt.optDef}>`;
            } else {
                optAlias = opt.type;
                optDef = opt.type;
            }
            return { optAlias, optDef };
        }

        const aliasesAndDefs = options.map(opt => {
            return getAliasAndDef(opt);
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


    const intfList: Array<InterfaceDeclaration> = [];
    list.forEach(f => {
        if (typeof f === "string") {
            intfList.push(typesFile.getInterfaceOrThrow(f));
            return;
        }
        if (f instanceof RegExp) {
            typesFile.getInterfaces().filter(intf => {
                return f.test(intf.getName());
            }).forEach(intf => intfList.push(intf));
            return;
        }
        intfList.push(f);
    });
    intfList.forEach(int => {
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

const generateEnumFromLiteralUnion = async (typeName: string) => {
    const typesFile = createProject().getSourceFileOrThrow(
        dtsFile
    );
    const unionType = typesFile.getTypeAlias(typeName);
    if (! unionType) {
        throw new Error(`Missing ${typeName} enum in WebIDL`);
    }
    const parsedUnionType = parseType(unionType.getType());
    if (!("options" in parsedUnionType)) {
        throw new Error(`Expeccted ${typeName} to contain literal options`);
    }
    const enumName = 'E' + typeName;
    const typeEnum: string = `export enum ${enumName} {
${parsedUnionType.options.map(o => {
    if (!("value" in o)) {
        throw new Error("Option should be a literal value");
    }
    return `\t${o.value} = ${JSON.stringify(o.value)}`
}).join(",\n")}
}\n`;
    const dts: string = await readFile(dtsFile, { encoding: "utf8" });
    await writeFile(dtsFile, dts + typeEnum);
    return enumName;
}

const generateFieldTypeEnum = async () => {
    await generateEnumFromLiteralUnion("BerytusFieldType");
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

const generateFieldOptionsUnion = async () => {
    const typesFile = createProject().getSourceFileOrThrow(
        dtsFile
    );
    const optionsIfNames: Array<string> = [];
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    listFields(typesFile).forEach(intf => {
        const options = intf.getPropertyOrThrow("options");
        optionsIfNames.push(options.getType().getSymbolOrThrow().getName());
    });
    await writeFile(
        dtsFile,
        dts
        + "\n"
        + `export type BerytusFieldOptionsUnion = ${optionsIfNames.join("\n\t| ")};`
        + "\n");
}

const deleteFunctions = async () => {
    const typesFile = createProject().getSourceFileOrThrow(
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
 * BerytusEncryptedPacket does not define any attributes.
 * Therefore, the generator will not embed an appropriate
 * placeholder for the JWE. We define one here and remove
 * the inheritance from Blob.
 */
const correctEncryptedPacketInterface = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    const intf = typesFile.getInterfaceOrThrow("BerytusEncryptedPacket");
    const origText = intf.getText();
    intf.removeExtends(0); // we do not want it to inherit from Blob
    intf.addProperty({
        name: "type",
        type: "\"JWE\"",
    })
    intf.addProperty({
        name: "value",
        type: "string"
    });
    dts = dts.replace(origText, intf.getText());
    await writeFile(
        dtsFile,
        dts
    );
}

const generateChallengeTypeEnum = async () => {
    await generateEnumFromLiteralUnion("BerytusChallengeType");
}

const generateChallengeMessagingTypes = async () => {
    const typesFile = project.getSourceFileOrThrow(
        dtsFile
    );
    let dts: string = await readFile(dtsFile, { encoding: "utf8" });
    const newIntfs: Array<string> = [];
    const patt = /^Berytus(.*)Challenge$/;
    const sendMessageArgsUnionMembers: Array<string> = [];
    const receiveMessageUnionMembers: Array<string> = [];
    const messageInfoUnionMembers: Array<string> = [];
    typesFile.getInterfaces()
        .filter(f => patt.test(f.getName()))
        .filter(f => f.getName() !== 'BerytusChallenge') // base interface
        .forEach(intf => {
            const [_, chName] = intf.getName().match(patt)!;
            const paramsIntf = typesFile.getInterface(`Berytus${chName}ChallengeParameters`);
            newIntfs.push(
                `\
interface Berytus${chName}ChallengeInfo {
    id: string;
    type: EBerytusChallengeType.${chName};
    parameters${paramsIntf ? `: ${paramsIntf.getName()}` : `: null`};
}`
            );
            const messageNameEnum = `EBerytus${chName}ChallengeMessageName`;
            const messagesMethods = intf.getMethods()
                .filter(m => !m.getName().startsWith("abort"));
            const messageNames: Array<string> = [];
            messagesMethods.forEach(m => {
                const returnType = m.getReturnType();
                const messageName = m.getName().charAt(0).toUpperCase() + m.getName().substring(1);
                messageNames.push(messageName);
                const returnTypeName = returnType.getSymbol()?.getName()
                if (returnTypeName !== 'Promise') {
                    throw new Error(`Message method ${m.getName()} defined in ${chName} must have a return type of a 'Promise' type. Got ${returnTypeName}`);
                }
                const [_, innerReturnTypeName] = returnType.getText().match(/^Promise<(.*)>$/)!;
                if ( innerReturnTypeName !== `BerytusChallenge${messageName}MessageResponse`) {
                    throw new Error(`Message method ${m.getName()} defined in ${chName} must have a return type of a Promise-resolve type of a dictionary whose name must be equal to  BerytusChallenge${messageName}MessageResponse. Got ${innerReturnTypeName}`);
                }
                const arg = m.getParameters()[0];
                newIntfs.push(
                    `\
interface BerytusChallenge${messageName}MessageRequest {
    payload: ${arg.getType().getText()}
}`
                );
                newIntfs.push(
                    `\
interface BerytusSend${messageName}Message extends BerytusChallenge${messageName}MessageRequest {
    challenge: Berytus${chName}ChallengeInfo;
    name: ${messageNameEnum}.${messageName};
}
`
                );
                messageInfoUnionMembers.push(`Berytus${chName}ChallengeInfo`);
                receiveMessageUnionMembers.push(innerReturnTypeName);
                sendMessageArgsUnionMembers.push(`BerytusSend${messageName}Message`);
            });
            newIntfs.push(`\
export enum ${messageNameEnum} {
    ${messageNames.map(s => `${s} = ${JSON.stringify(s)}`).join(",\n\t")}
}`);
        });
    await writeFile(
        dtsFile,
        dts + "\n" + newIntfs.join("\n") + "\n" +
        `export type BerytusChallengeInfoUnion = ${messageInfoUnionMembers.join("\n\t| ")};` + "\n" +
        `export type BerytusSendMessageUnion = ${sendMessageArgsUnionMembers.join("\n\t| ")};` + "\n" +
        `export type BerytusReceiveMessageUnion = ${receiveMessageUnionMembers.join("\n\t| ")};` + "\n"
    );
};

const generate = async () => {
    await generateFieldTypeEnum();
    await generateFieldProperties();
    await ensureUnionsAreAliases(listFields());
    await generateFieldOptionsUnion();
    await ensureUnionsAreAliases(["BerytusUserAttributeDefinition"]);
    await correctEncryptedPacketInterface();
    await generateChallengeTypeEnum();
    await generateChallengeMessagingTypes();
    await deleteFunctions();
    await generateFieldUnion();
    await generateFieldValueUnion();
    await ensureUnionsAreAliases([/^Berytus.*?Challenge/]);
}

generate();