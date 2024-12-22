/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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

const requestIs = <RT extends RequestType>(requestType: RT, d: { context: PreliminaryRequestContext; args?: unknown; output: unknown}): d is { context: PreliminaryRequestContext; args: RequestHandlerFunctionParameters<RT>[1]; output: RequestHandlerFunctionReturnType<RT> } => {
    return d.context.request.type === requestType;
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

export class ValidatedRequestHandler extends IsolatedRequestHandler {
    #schema: any;

    constructor(impl: IUnderlyingRequestHandler) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
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
        await super.preCall(group, method, input);
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
        const data = { ...input, output };
        if (requestIs("AccountCreation_AddField", data)) {
            const fieldTypeEntry = await this.#getFieldTypeEntry(data.args.field.type);
            this.#validateValue(
                fieldTypeEntry.properties.value.type,
                output,
                errorPrefix
            );
        }
        await super.preResolve(group, method, input, output);
    }

    protected async preReject(group: string, method: string, input: PreCallInput, value: unknown) {
        // TODO(berytus): validate error value
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

    async #getFieldTypeEntry(fieldType: AddFieldArgs['field']['type']) {
        const schema = await this.#getSchema();
        const id = \`Berytus\${fieldType}Field\`;
        const fieldTypeEntry = schema.get(id);
        if (! fieldTypeEntry) {
            throw new Error(
                \`Berytus Schema did not contain a "\${id}" type.\`
            );
        }
        return fieldTypeEntry;
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