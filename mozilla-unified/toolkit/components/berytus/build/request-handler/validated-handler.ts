/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

import { project } from "./Project.js";
import type { EnumDeclaration } from "ts-morph";

const generateFieldIdValidator = () => {
    const code = `\
class FieldIdValidator implements ILogicalValidator {
    /**
     * op id -> field id -> true
     * TODO(berytus): There should be a cleanup mechanism.
     */
    #fields: Record<string, Record<string, true>>;

    constructor() {
        this.#fields = {};
    }

    #fieldExists(operationId: string, fieldId: string) {
        if (!(operationId in this.#fields)) {
            return false;
        }
        if (!(fieldId in this.#fields[operationId])) {
            return false;
        }
        return true;
    }

    #addField(operationId: string, fieldId: string) {
        if (!(operationId in this.#fields)) {
            this.#fields[operationId] = {};
        }
        if (!(fieldId in this.#fields[operationId])) {
            this.#fields[operationId][fieldId] = true;
        }
    }

    #removeField(operationId: string, fieldId: string) {
        if (!(operationId in this.#fields)) {
            return;
        }
        if (!(fieldId in this.#fields[operationId])) {
            return;
        }
        delete this.#fields[operationId][fieldId];
    }

    async consume(group: string, method: string, input: PreCallInput): Promise<void> {
        if (inputIs("AccountCreation_AddField", input)) {
            const { operation } = input.context as RequestContextWithOperation;
            const { field } = input.args;
            if (this.#fieldExists(operation.id, field.id)) {
                throw new Error(\`Illegal field creation request; Passed field id (\${field.id}) already exists.\`);
            }
            this.#addField(operation.id, field.id);
            return;
        }
        if (inputIs("AccountCreation_RejectFieldValue", input)) {
            const { operation } = input.context as RequestContextWithOperation;
            const { fieldId } = input.args;
            if (! this.#fieldExists(operation.id, fieldId)) {
                throw new Error(\`Illegal field creation request; Passed field id (\${fieldId}) does not exist.\`);
            }
            return;
        }
    }

    async digest(group: string, method: string, input: PreCallInput, output: unknown): Promise<void> {}

    async rollback(group: string, method: string, input: PreCallInput): Promise<void> {
        if (inputIs("AccountCreation_AddField", input)) {
            const { operation } = input.context as RequestContextWithOperation;
            const { field } = input.args;
            this.#removeField(operation.id, field.id);
            return;
        }
    }
}`;
    return code;
}
const generateChallangeSequenceValidator = () => {
    // Note(berytus): Depends on the generation of message name enums
    // Berytus${chName}ChallengeMessageName
    // implemented in gen-webidl-ts.ts
    const typesFile = project.getSourceFileOrThrow(
        './src/generated/berytus.web.d.ts'
    );
    const infos = typesFile.getInterfaces()
        .filter(f => /^Berytus[a-zA-Z0-9]+ChallengeInfo$/.test(f.getName()));
    const msgNameEnums: Record<string, EnumDeclaration> = {};
    typesFile.getEnums()
        .forEach(e => {
            const res = e.getName().match(/^EBerytus([a-zA-Z0-9]+)ChallengeMessageName+$/);
            if (res === null) {
                return;
            }
            const [_, chName] = res;
            msgNameEnums[chName] = e;
        });

    const code = `\
class ChallangeMessagingSequenceValidator implements ILogicalValidator {
    /**
     * op id -> ch id -> nb of messages sent
     * TODO(berytus): There should be a cleanup mechanism.
     */
    #counter: Record<string, Record<string, number>>;

    constructor() {
        this.#counter = {};
    }

    #defineRecord(operationId: string, challengeId: string) {
        if (!(operationId in this.#counter)) {
            this.#counter[operationId] = {};
        }
        if (!(challengeId in this.#counter[operationId])) {
            this.#counter[operationId][challengeId] = 0;
        }
    }

    #count(operationId: string, challengeId: string): number {
        this.#defineRecord(operationId, challengeId);
        return this.#counter[operationId][challengeId];
    }

    #increment(operationId: string, challengeId: string) {
        this.#defineRecord(operationId, challengeId);
        this.#counter[operationId][challengeId] += 1;
    }

    #decrement(operationId: string, challengeId: string) {
        this.#defineRecord(operationId, challengeId);
        this.#counter[operationId][challengeId] -= 1;
    }

    async consume(group: string, method: string, input: PreCallInput): Promise<void> {
        if (!inputIs("AccountAuthentication_RespondToChallengeMessage", input)) {
            return;
        }
        const { operation } = input.context as RequestContextWithOperation;
        const { challenge } = input.args;
        switch (challenge.type) {
            ${infos.map(info => {
                const chName = info.getPropertyOrThrow("type").getType().getLiteralValue() as string;
                const enumDecl = msgNameEnums[chName];
                return `case "${chName}": {
                const names = [${enumDecl.getMembers().map(m => JSON.stringify(m.getValue())).join(", ")}];
                const currentCount = this.#count(operation.id, challenge.id);
                if (currentCount >= names.length) {
                    throw new Error("Illegal message; maximum amount of messages reached.");
                }
                const nextMessageName = names[currentCount];
                if (input.args.name !== nextMessageName) {
                    throw new Error("Illegal message; Unexpected message name. Expected " + nextMessageName + " instead of " + input.args.name);
                }
                this.#increment(operation.id, challenge.id);
                break;
            }`
            }).join("\n\t\t\t")}
        }
    }

    async digest(group: string, method: string, input: PreCallInput, output: unknown): Promise<void> {}

    async rollback(group: string, method: string, input: PreCallInput): Promise<void> {
        if (!inputIs("AccountAuthentication_RespondToChallengeMessage", input)) {
            return;
        }
        const { challenge } = input.args;
        const { operation } = input.context as RequestContextWithOperation;
        if (!(operation.id in this.#counter)) {
            console.warn("Expected operation to be set in #counter");
            return;
        }
        if (!(challenge.id in this.#counter[operation.id])) {
            console.warn("Expected challenge to be set in #counter[...]");
            return;
        }
        this.#decrement(operation.id, challenge.id);
    }
}
`;
    return code;
}

const generateFieldCreationValidator = () => {
    const code = `\
class FieldCreationHandler implements ILogicalValidator {
    async consume(group: string, method: string, input: PreCallInput): Promise<void> {}

    async rollback(group: string, method: string, input: PreCallInput): Promise<void> {}

    async digest(group: string, method: string, input: PreCallInput, output: unknown): Promise<void> {
        const errorPrefix = \`Malformed output passed from the request handler's \`
            + \`\${group}:\${method} method.\`;
        if (inputIs("AccountCreation_AddField", input)) {
            const { value: dictatedValue } = input.args.field;
            if (dictatedValue === null) {
                if (output === null) {
                    throw new ResolutionError(errorPrefix, "resolved value must not be null since the web app did not dictate a field value.");
                }
            } else {
                if (output !== null) {
                    throw new ResolutionError(errorPrefix, "resolved value must be null since the web app has dictated a field value.");
                }
            }
            return;
        }
        if (inputIs("AccountCreation_RejectFieldValue", input)) {
            const { optionalNewValue: dictatedValue } = input.args;
            if (dictatedValue === undefined) {
                if (output === null) {
                    throw new ResolutionError(errorPrefix, "resolved value must not be null since the web app did not dictate a revised field value.");
                }
            } else {
                if (output !== null) {
                    throw new ResolutionError(errorPrefix, "resolved value must be null since the web app has dictated a revised field value.");
                }
            }
            return;
        }
    }
}`;
    return code;
}

const generateE2EEMessagingValidator = () => {
    const code = `\
// TODO(berytus): Also reject requests containing encrypted packets
// when channel e2ee is false.
class E2EEMessagingValidator implements ILogicalValidator {
    #isE2EEEnabled(ctx: PreliminaryRequestContext) {
        if (! ("channel" in ctx)) {
            return false;
        }
        return (ctx as RequestContext).channel.e2eeEnabled;
    }

    #isEncrypted(valueOrDict: unknown): number {
        if (valueOrDict === null || valueOrDict === undefined) {
            return 1;
        }
        if (typeof valueOrDict !== "object") {
            return 0;
        }
        if ("type" in valueOrDict &&
            valueOrDict.type === "JWE" &&
            "value" in valueOrDict &&
            typeof valueOrDict.value === "string") {
            return 2;
        }
        let result = 0;
        for (const key in valueOrDict) {
            const entry = (valueOrDict as Record<string, unknown>)[key];
            result |= this.#isEncrypted(entry);
        }
        return result;
    }
    async consume(group: string, method: string, input: PreCallInput): Promise<void> {
        const errorPrefix = \`Malformed input passed into the request handler's \`
            + \`\${group}:\${method} method.\`;
        if (! this.#isE2EEEnabled(input.context)) {
            if (this.#isEncrypted(input.args) >= 2) {
                throw new ResolutionError(
                    errorPrefix,
                    "input value must not be encrypted."
                );
            }
            return;
        }
        if (inputIs("AccountCreation_AddField", input)) {
            if (! this.#isEncrypted(input.args.field.value)) {
                throw new ResolutionError(
                    errorPrefix,
                    "input value must be encrypted."
                );
            }
        }
        if (inputIs("AccountCreation_RejectFieldValue", input)) {
            if (undefined === input.args.optionalNewValue) {
                return;
            }
            if (! this.#isEncrypted(input.args.optionalNewValue)) {
                throw new ResolutionError(
                    errorPrefix,
                    "input value must be encrypted."
                );
            }
        }
        if (inputIs("AccountAuthentication_RespondToChallengeMessage", input)) {
            if (! this.#isEncrypted(input.args.payload)) {
                throw new ResolutionError(
                    errorPrefix,
                    "input value must be encrypted."
                );
            }
        }
    }

    async rollback(group: string, method: string, input: PreCallInput): Promise<void> {}

    async digest(group: string, method: string, input: PreCallInput, output: unknown): Promise<void> {
        const errorPrefix = \`Malformed output passed from the request handler's \`
            + \`\${group}:\${method} method.\`;
        if (! this.#isE2EEEnabled(input.context)) {
            if (this.#isEncrypted(output) >= 2) {
                throw new ResolutionError(
                    errorPrefix,
                    "resolved value must not be encrypted."
                );
            }
            return;
        }
        const data = {
            context: input.context,
            args: input.args,
            output
        };
        if (
            requestIs("AccountCreation_AddField", data) ||
            requestIs("AccountAuthentication_RespondToChallengeMessage", data) ||
            requestIs("AccountCreation_RejectFieldValue", data)
        ) {
            if (! this.#isEncrypted(data.output)) {
                throw new ResolutionError(
                    errorPrefix,
                    "resolved value must be encrypted."
                );
            }
        }
        if (requestIs("AccountCreation_GetUserAttributes", data)) {
            for (const attr of data.output) {
                if (! this.#isEncrypted(attr.value)) {
                    throw new ResolutionError(
                        errorPrefix,
                        "resolved value must be encrypted."
                    );
                }
            }
        }
    }
}
`;
    return code;
}

export const generateValidatedHandler = () => {
    const typesToImport: Record<string, true> = {
        "RequestType": true,
        "RequestHandlerFunctionParameters": true,
        "RequestHandlerFunctionReturnType": true
    }

    let code = `interface PreCallInput {
    context: PreliminaryRequestContext;
    args?: unknown
}
const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    Schemas: "resource://gre/modules/Schemas.sys.mjs"
});

const isPrelimContext = (context: unknown): context is Pick<PreliminaryRequestContext, 'request'> => {
    return typeof context === "object" &&
        context !== null &&
        "request" in context &&
        typeof context.request === "object" &&
        context.request !== null &&
        "type" in context.request &&
        typeof context.request.type === "string" &&
        "id" in context.request &&
        typeof context.request.id === "string";
}

const requestIs = <RT extends RequestType>(requestType: RT, d: { context: unknown; args?: unknown; output: unknown}): d is { context: RequestHandlerFunctionParameters<RT>[0]; args: RequestHandlerFunctionParameters<RT>[1]; output: RequestHandlerFunctionReturnType<RT> } => {
    if (isPrelimContext(d.context)) {
        return d.context.request.type === requestType;
    }
    return false;
}

const inputIs = <RT extends RequestType>(requestType: RT, input: { context: unknown; args?: unknown; }): input is { context: RequestHandlerFunctionParameters<RT>[0]; args: RequestHandlerFunctionParameters<RT>[1]; } => {
    if (isPrelimContext(input.context)) {
        return input.context.request.type === requestType;
    }
    return false;
}

/**
 * Implementation copied from Schemas.sys.mjs's Context
 */
class ValidationContext {
    path: Array<string> = [];
    manifestVersion = 2;
    currentChoices = new Set();
    choicePathIndex = 0;

    matchManifestVersion() {
        return true;
    }
    checkLoadURL(url: string) {
        throw new Error("Method not implemented");
    }
    hasPermission(perm: string) {
        throw new Error("Method not implemented");
    }
    logError(err: string | Error) {
        console.error(err);
    }
    withPath(component: string, callback: (...args: any[]) => any) {
        this.path.push(component);
        try {
            return callback();
        } finally {
            this.path.pop();
        }
    }

    get choicePath() {
        let path = this.path.slice(this.choicePathIndex);
        return path.join(".");
    }


    get currentTarget() {
        return this.path.join(".");
    }

    error(
        errorMessage: string | Function | null,
        choicesMessage: string | Function | undefined = undefined,
        warning: boolean = false
    ): object {
        if (choicesMessage !== null) {
            let { choicePath } = this;
            if (choicePath) {
                choicesMessage = \`.\${choicePath} must \${typeof choicesMessage === "function" ? choicesMessage() : choicesMessage}\`;
            }

            this.currentChoices.add(choicesMessage);
        }

        if (this.currentTarget) {
            let { currentTarget } = this;
            return {
                error: () =>
                    \`\${warning ? "Warning" : "Error"
                    } processing \${currentTarget}: \${typeof errorMessage === "function" ? errorMessage() : errorMessage}\`,
            };
        }
        return { error: errorMessage };
    }

    withChoices(callback: Function): object {
        let { currentChoices, choicePathIndex } = this;

        let choices = new Set();
        this.currentChoices = choices;
        this.choicePathIndex = this.path.length;

        try {
            let result = callback();

            return { result, choices };
        } finally {
            this.currentChoices = currentChoices;
            this.choicePathIndex = choicePathIndex;

            if (choices.size == 1) {
                for (let choice of choices) {
                    currentChoices.add(choice);
                }
            } else if (choices.size) {
                this.error(null, () => {
                    let array = Array.from(choices, (m) => {
                        if (typeof m === "function") {
                            return m();
                        }
                        return m;
                    });
                    let n = array.length - 1;
                    array[n] = \`or \${array[n]}\`;

                    return \`must either [\${array.join(", ")}]\`;
                });
            }
        }
    }
}

class ResolutionError extends Error {
    reason: string;

    constructor(msg: string, reason: string) {
        super(msg + " Reason: " + reason);
        this.reason = reason;
    }

    get name() {
        return "ResolutionError";
    }
}

interface ILogicalValidator {
    consume(group: string, method: string, input: PreCallInput): Promise<void>;
    rollback(group: string, method: string, input: PreCallInput): Promise<void>;
    digest(group: string, method: string, input: PreCallInput, output: unknown): Promise<void>;
}

${generateFieldIdValidator()}
${generateChallangeSequenceValidator()}
${generateFieldCreationValidator()}
${generateE2EEMessagingValidator()}

export class ValidatedRequestHandler extends IsolatedRequestHandler {
    #schema: any;
    #validators: ILogicalValidator[];

    constructor(impl: IUnderlyingRequestHandler) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
        this.#validators = [];
        this.#validators.push(
            new FieldIdValidator(),
            new ChallangeMessagingSequenceValidator(),
            new FieldCreationHandler(),
            new E2EEMessagingValidator()
        );
    }

    #validateValue(
        typeEntry: any,
        value: unknown,
        message: string
    ) {
        const { error } = typeEntry.normalize(
            value,
            new ValidationContext()
        );
        if (error) {
            throw new ResolutionError(
                message,
                (typeof error === "function" ? error() : error)
            );
        }
    }

    async #consumeValidators(group: string, method: string, input: PreCallInput) {
        await Promise.all(this.#validators.map(val => val.consume(group, method, input)));
    }

    async #digestValidators(group: string, method: string, input: PreCallInput, output: unknown) {
        await Promise.all(this.#validators.map(val => val.digest(group, method, input, output)));
    }

    async #rollbackValidators(group: string, method: string, input: PreCallInput) {
        await Promise.all(this.#validators.map(val => val.rollback(group, method, input)));
    }

    protected async preCall(group: string, method: string, input: PreCallInput) {
        const methodType = await this.#getMethodTypeEntry(group, method);
        const { parameters } = methodType;
        if (parameters.length === 0) {
            throw new Error(
                \`Berytus Schema's RequestHandler type did not \'
                + \'contain parameters for \${group}:\${method}.\`
            );
        }
        const message = \`Malformed input passed to the request handler's \`
            + \`\${group}:\${method} method.\`;

        this.#validateValue(
            parameters[0].type,
            input.context,
            message
        );
        if (parameters[1]) {
            this.#validateValue(
                parameters[1].type,
                input.args,
                message
            );
        }
        await this.#consumeValidators(group, method, input);
        try {
            await super.preCall(group, method, input);
        } catch (e) {
            await this.#rollbackValidators(group, method, input);
            throw e;
        }
    }

    protected async preResolve(group: string, method: string, input: PreCallInput, output: unknown) {
        const resultType = await this.#getMethodResultTypeEntry(group, method);
        const errorPrefix = \`Malformed output passed from the request handler's \`
            + \`\${group}:\${method} method.\`;
        this.#validateValue(
            resultType,
            output,
            errorPrefix
        );
        await this.#digestValidators(group, method, input, output);
        await super.preResolve(group, method, input, output);
    }

    protected async preReject(group: string, method: string, input: PreCallInput, value: unknown) {
        // TODO(berytus): validate error value
        await this.#rollbackValidators(group, method, input);
        await super.preReject(group, method, input, value);
    }

    async #getMethodResultTypeEntry(group: string, method: string) {
        group = group.charAt(0).toUpperCase() + group.substring(1);
        method = method.charAt(0).toUpperCase() + method.substring(1);
        const schema = await this.#getSchema();
        const id = group + method + "Result"
        let resultType = schema.get(id);
        if (! resultType) {
            throw new Error(
                \`Berytus Schema did not contain a "\${id}" type.\`
            );
        }
        return resultType;
    }

    async #getMethodTypeEntry(group: string, method: string) {
        const schema = await this.#getSchema();
        const requestHandlerType = schema.get("RequestHandler");
        if (! requestHandlerType) {
            throw new Error(
                \`Berytus Schema did not contain a "RequestHandler" type.\`
            );
        }
        if (! (group in requestHandlerType.properties)) {
            throw new Error(
                \`Berytus Schema's RequestHandler type did not contain \`
                + \` a property for (group:) \${group}\`
            );
        }
        let groupHandlerType = requestHandlerType.properties[group].type;
        if ("reference" in groupHandlerType) {
            groupHandlerType = groupHandlerType.targetType;
        }
        if (! (method in groupHandlerType.properties)) {
            throw new Error(
                \`Berytus Schema's RequestHandler.\${group} type did not contain \`
                + \` a property for (method:) \${method}\`
            );
        }
        const methodType = groupHandlerType.properties[method].type;
        return methodType;
    }

    async #getSchema() {
        if (this.#schema) {
            return this.#schema;
        }
        // @ts-ignore: TS did not catch assertion for "lazy"
        await lazy.Schemas.load(
            "chrome://extensions/content/schemas/berytus.json"
        );
        // @ts-ignore: TS did not catch assertion for "lazy"
        this.#schema = lazy.Schemas.getNamespace("berytus");
        return this.#schema;
    }
}`;
    return {
        code,
        typesToImport
    };
}