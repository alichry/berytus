/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

export const generateValidatedHandler = () => {
    let code = `interface PreCallInput {
    context: Record<string, unknown>;
    args?: unknown
}
const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    Schemas: "resource://gre/modules/Schemas.sys.mjs"
});


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
                choicesMessage = \`.\${choicePath} must \${choicesMessage}\`;
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

export class ValidatedRequestHandler extends IsolatedRequestHandler {
    #schema: any;

    constructor(impl: IUnderlyingRequestHandler) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
    }

    #validateInput(
        group: string,
        method: string,
        parameterType: any,
        input: unknown
    ) {
        const { error } = parameterType.normalize(
            input,
            new ValidationContext()
        );
        if (error) {
            throw new Error(
                \`Malformed input passed to the request handler's \`
                + \`\${group}:\${method} method. Reason: \${typeof error === "function" ? error() : error}\`
            );
        }
    }

    protected async preCall(group: string, method: string, input: PreCallInput) {
        const methodType = await this.#getMethodType(group, method);
        const { parameters } = methodType;
        if (parameters.length === 0) {
            throw new Error(
                \`Berytus Schema's RequestHandler type did not \'
                + \'contain parameters for \${group}:\${method}.\`
            );
        }
        this.#validateInput(
            group,
            method,
            parameters[0].type,
            input.context
        );
        if (parameters[1]) {
            this.#validateInput(
                group,
                method,
                parameters[1].type,
                input.args
            );
        }
        await super.preCall(group, method, input);
    }

    protected async preResolve(group: string, method: string, value: unknown) {
        // TODO(berytus): validate output
        await super.preResolve(group, method, value);
    }

    protected async preReject(group: string, method: string, value: unknown) {
        // TODO(berytus): validate error value
        await super.preReject(group, method, value);
    }

    async #getMethodType(group: string, method: string) {
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
    return code;
}