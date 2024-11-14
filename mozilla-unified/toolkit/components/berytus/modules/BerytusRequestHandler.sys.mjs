/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
export class IsolatedRequestHandler {
    #impl;
    manager;
    channel;
    login;
    accountCreation;
    accountAuthentication;
    constructor(impl) {
        this.#impl = impl;
        const self = this;
        this.manager = {
            async getSigningKey(context, args) {
                try {
                    await self.preCall("manager", "getSigningKey", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("manager", "getSigningKey", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("manager", "getSigningKey", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.manager.getSigningKey({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("manager", "getSigningKey", context.response, e);
                    return;
                }
            },
            async getCredentialsMetadata(context, args) {
                try {
                    await self.preCall("manager", "getCredentialsMetadata", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("manager", "getCredentialsMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("manager", "getCredentialsMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.manager.getCredentialsMetadata({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("manager", "getCredentialsMetadata", context.response, e);
                    return;
                }
            },
        };
        this.channel = {
            async generateKeyExchangeParameters(context, args) {
                try {
                    await self.preCall("channel", "generateKeyExchangeParameters", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("channel", "generateKeyExchangeParameters", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "generateKeyExchangeParameters", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.channel.generateKeyExchangeParameters({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("channel", "generateKeyExchangeParameters", context.response, e);
                    return;
                }
            },
            async enableEndToEndEncryption(context, args) {
                try {
                    await self.preCall("channel", "enableEndToEndEncryption", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("channel", "enableEndToEndEncryption", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "enableEndToEndEncryption", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.channel.enableEndToEndEncryption({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("channel", "enableEndToEndEncryption", context.response, e);
                    return;
                }
            },
            async closeChannel(context) {
                try {
                    await self.preCall("channel", "closeChannel", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("channel", "closeChannel", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "closeChannel", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.channel.closeChannel({
                        ...context,
                        ...wrappedResponseCtx
                    });
                }
                catch (e) {
                    self.handleUnexpectedException("channel", "closeChannel", context.response, e);
                    return;
                }
            },
        };
        this.login = {
            async approveOperation(context, args) {
                try {
                    await self.preCall("login", "approveOperation", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("login", "approveOperation", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "approveOperation", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.login.approveOperation({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("login", "approveOperation", context.response, e);
                    return;
                }
            },
            async closeOpeation(context) {
                try {
                    await self.preCall("login", "closeOpeation", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("login", "closeOpeation", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "closeOpeation", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.login.closeOpeation({
                        ...context,
                        ...wrappedResponseCtx
                    });
                }
                catch (e) {
                    self.handleUnexpectedException("login", "closeOpeation", context.response, e);
                    return;
                }
            },
            async getRecordMetadata(context) {
                try {
                    await self.preCall("login", "getRecordMetadata", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("login", "getRecordMetadata", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "getRecordMetadata", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.login.getRecordMetadata({
                        ...context,
                        ...wrappedResponseCtx
                    });
                }
                catch (e) {
                    self.handleUnexpectedException("login", "getRecordMetadata", context.response, e);
                    return;
                }
            },
            async updateMetadata(context, args) {
                try {
                    await self.preCall("login", "updateMetadata", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("login", "updateMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "updateMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.login.updateMetadata({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("login", "updateMetadata", context.response, e);
                    return;
                }
            },
        };
        this.accountCreation = {
            async approveTransitionToAuthOp(context, args) {
                try {
                    await self.preCall("accountCreation", "approveTransitionToAuthOp", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountCreation", "approveTransitionToAuthOp", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "approveTransitionToAuthOp", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountCreation.approveTransitionToAuthOp({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountCreation", "approveTransitionToAuthOp", context.response, e);
                    return;
                }
            },
            async getUserAttributes(context) {
                try {
                    await self.preCall("accountCreation", "getUserAttributes", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountCreation", "getUserAttributes", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "getUserAttributes", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountCreation.getUserAttributes({
                        ...context,
                        ...wrappedResponseCtx
                    });
                }
                catch (e) {
                    self.handleUnexpectedException("accountCreation", "getUserAttributes", context.response, e);
                    return;
                }
            },
            async addField(context, args) {
                try {
                    await self.preCall("accountCreation", "addField", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountCreation", "addField", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "addField", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountCreation.addField({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountCreation", "addField", context.response, e);
                    return;
                }
            },
            async rejectFieldValue(context, args) {
                try {
                    await self.preCall("accountCreation", "rejectFieldValue", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountCreation", "rejectFieldValue", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "rejectFieldValue", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountCreation.rejectFieldValue({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountCreation", "rejectFieldValue", context.response, e);
                    return;
                }
            },
        };
        this.accountAuthentication = {
            async approveChallengeRequest(context, args) {
                try {
                    await self.preCall("accountAuthentication", "approveChallengeRequest", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountAuthentication", "approveChallengeRequest", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "approveChallengeRequest", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountAuthentication.approveChallengeRequest({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountAuthentication", "approveChallengeRequest", context.response, e);
                    return;
                }
            },
            async abortChallenge(context, args) {
                try {
                    await self.preCall("accountAuthentication", "abortChallenge", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountAuthentication", "abortChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "abortChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountAuthentication.abortChallenge({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountAuthentication", "abortChallenge", context.response, e);
                    return;
                }
            },
            async closeChallenge(context, args) {
                try {
                    await self.preCall("accountAuthentication", "closeChallenge", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountAuthentication", "closeChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "closeChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountAuthentication.closeChallenge({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountAuthentication", "closeChallenge", context.response, e);
                    return;
                }
            },
            async respondToChallengeMessage(context, args) {
                try {
                    await self.preCall("accountAuthentication", "respondToChallengeMessage", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountAuthentication", "respondToChallengeMessage", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "respondToChallengeMessage", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.reject(val);
                        }
                    }
                };
                try {
                    await self.#impl.accountAuthentication.respondToChallengeMessage({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountAuthentication", "respondToChallengeMessage", context.response, e);
                    return;
                }
            },
        };
    }
    preCall(group, method, args) { }
    preResolve(group, method, value) { }
    preReject(group, method, value) { }
    handleUnexpectedException(group, method, response, excp) {
        // TODO(berytus): Define what would be the
        // accepted values to reject a request with.
        console.error("Unexpected exception from secret manager:", excp);
        const err = new Components.Exception("Error sending request to secret manager. "
            + "The secret manager unexpectedly threw an exception "
            + "instead of using the reject callback (RequestType: "
            + `${group}:${String(method)}). Exception: ${excp}`, Cr.NS_ERROR_FAILURE);
        response.reject(err);
    }
}
const lazy = {};
ChromeUtils.defineESModuleGetters(lazy, {
    Schemas: "resource://gre/modules/Schemas.sys.mjs"
});
/**
 * Implementation copied from Schemas.sys.mjs's Context
 */
class ValidationContext {
    path = [];
    manifestVersion = 2;
    currentChoices = new Set();
    choicePathIndex = 0;
    matchManifestVersion() {
        return true;
    }
    checkLoadURL(url) {
        throw new Error("Method not implemented");
    }
    hasPermission(perm) {
        throw new Error("Method not implemented");
    }
    logError(err) {
        console.error(err);
    }
    withPath(component, callback) {
        this.path.push(component);
        try {
            return callback();
        }
        finally {
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
    error(errorMessage, choicesMessage = undefined, warning = false) {
        if (choicesMessage !== null) {
            let { choicePath } = this;
            if (choicePath) {
                choicesMessage = `.${choicePath} must ${choicesMessage}`;
            }
            this.currentChoices.add(choicesMessage);
        }
        if (this.currentTarget) {
            let { currentTarget } = this;
            return {
                error: () => `${warning ? "Warning" : "Error"} processing ${currentTarget}: ${typeof errorMessage === "function" ? errorMessage() : errorMessage}`,
            };
        }
        return { error: errorMessage };
    }
    withChoices(callback) {
        let { currentChoices, choicePathIndex } = this;
        let choices = new Set();
        this.currentChoices = choices;
        this.choicePathIndex = this.path.length;
        try {
            let result = callback();
            return { result, choices };
        }
        finally {
            this.currentChoices = currentChoices;
            this.choicePathIndex = choicePathIndex;
            if (choices.size == 1) {
                for (let choice of choices) {
                    currentChoices.add(choice);
                }
            }
            else if (choices.size) {
                this.error(null, () => {
                    let array = Array.from(choices, (m) => {
                        if (typeof m === "function") {
                            return m();
                        }
                        return m;
                    });
                    let n = array.length - 1;
                    array[n] = `or ${array[n]}`;
                    return `must either [${array.join(", ")}]`;
                });
            }
        }
    }
}
export class ValidatedRequestHandler extends IsolatedRequestHandler {
    #schema;
    constructor(impl) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
    }
    #validateInput(group, method, parameterType, input) {
        const { error } = parameterType.normalize(input, new ValidationContext());
        if (error) {
            throw new Error(`Malformed input passed to the request handler's `
                + `${group}:${method} method. Reason: ${typeof error === "function" ? error() : error}`);
        }
    }
    async preCall(group, method, input) {
        const methodType = await this.#getMethodType(group, method);
        const { parameters } = methodType;
        if (parameters.length === 0) {
            throw new Error(`Berytus Schema's RequestHandler type did not '
                + 'contain parameters for ${group}:${method}.`);
        }
        this.#validateInput(group, method, parameters[0].type, input.context);
        if (parameters[1]) {
            this.#validateInput(group, method, parameters[1].type, input.args);
        }
        await super.preCall(group, method, input);
    }
    async preResolve(group, method, value) {
        // TODO(berytus): validate output
        await super.preResolve(group, method, value);
    }
    async preReject(group, method, value) {
        // TODO(berytus): validate error value
        await super.preReject(group, method, value);
    }
    async #getMethodType(group, method) {
        const schema = await this.#getSchema();
        const requestHandlerType = schema.get("RequestHandler");
        if (!requestHandlerType) {
            throw new Error(`Berytus Schema did not contain a "RequestHandler" type.`);
        }
        if (!(group in requestHandlerType.properties)) {
            throw new Error(`Berytus Schema's RequestHandler type did not contain `
                + ` a property for (group:) ${group}`);
        }
        let groupHandlerType = requestHandlerType.properties[group].type;
        if ("reference" in groupHandlerType) {
            groupHandlerType = groupHandlerType.targetType;
        }
        if (!(method in groupHandlerType.properties)) {
            throw new Error(`Berytus Schema's RequestHandler.${group} type did not contain `
                + ` a property for (method:) ${method}`);
        }
        const methodType = groupHandlerType.properties[method].type;
        return methodType;
    }
    async #getSchema() {
        if (this.#schema) {
            return this.#schema;
        }
        // @ts-ignore: TS did not catch assertion for "lazy"
        await lazy.Schemas.load("chrome://extensions/content/schemas/berytus.json");
        // @ts-ignore: TS did not catch assertion for "lazy"
        this.#schema = lazy.Schemas.getNamespace("berytus");
        return this.#schema;
    }
}
export class SequentialRequestHandler extends ValidatedRequestHandler {
    busy = false;
    async preCall(group, method, input) {
        if (this.busy) {
            throw new Components.Exception('Refusing to send request to secret manager, '
                + 'an existing request is still pending. '
                + 'make sure to send one request at a time '
                + 'to the secret manager.', Cr.NS_ERROR_FAILURE);
        }
        this.busy = true;
        await super.preCall(group, method, input);
    }
    async preResolve(group, method, value) {
        this.busy = false;
        await super.preResolve(group, method, value);
    }
    async preReject(group, method, value) {
        this.busy = false;
        await super.preReject(group, method, value);
    }
    handleUnexpectedException(group, method, response, excp) {
        this.busy = false;
        super.handleUnexpectedException(group, method, response, excp);
    }
}
function uuid() {
    // @ts-ignore: TODO(berytus): add to index.d.ts
    return Services.uuid
        .generateUUID()
        .toString();
}
export class PublicRequestHandler {
    #impl;
    manager;
    channel;
    login;
    accountCreation;
    accountAuthentication;
    constructor(impl) {
        if (!(impl instanceof SequentialRequestHandler)) {
            throw new Error('Cannot construct PublicRequestHandler '
                + 'Expecting request handler to be an '
                + 'instance of SequentialRequestHandler; '
                + 'got otherwise.');
        }
        this.#impl = impl;
        const self = this;
        this.manager = {
            getSigningKey(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Manager_GetSigningKey"
                        }
                    };
                    self.#impl.manager.getSigningKey({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            getCredentialsMetadata(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Manager_GetCredentialsMetadata"
                        }
                    };
                    self.#impl.manager.getCredentialsMetadata({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
        };
        this.channel = {
            generateKeyExchangeParameters(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Channel_GenerateKeyExchangeParameters"
                        }
                    };
                    self.#impl.channel.generateKeyExchangeParameters({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            enableEndToEndEncryption(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Channel_EnableEndToEndEncryption"
                        }
                    };
                    self.#impl.channel.enableEndToEndEncryption({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            closeChannel(context) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Channel_CloseChannel"
                        }
                    };
                    self.#impl.channel.closeChannel({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    });
                });
            },
        };
        this.login = {
            approveOperation(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Login_ApproveOperation"
                        }
                    };
                    self.#impl.login.approveOperation({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            closeOpeation(context) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Login_CloseOpeation"
                        }
                    };
                    self.#impl.login.closeOpeation({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    });
                });
            },
            getRecordMetadata(context) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Login_GetRecordMetadata"
                        }
                    };
                    self.#impl.login.getRecordMetadata({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    });
                });
            },
            updateMetadata(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "Login_UpdateMetadata"
                        }
                    };
                    self.#impl.login.updateMetadata({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
        };
        this.accountCreation = {
            approveTransitionToAuthOp(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountCreation_ApproveTransitionToAuthOp"
                        }
                    };
                    self.#impl.accountCreation.approveTransitionToAuthOp({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            getUserAttributes(context) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountCreation_GetUserAttributes"
                        }
                    };
                    self.#impl.accountCreation.getUserAttributes({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    });
                });
            },
            addField(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountCreation_AddField"
                        }
                    };
                    self.#impl.accountCreation.addField({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            rejectFieldValue(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountCreation_RejectFieldValue"
                        }
                    };
                    self.#impl.accountCreation.rejectFieldValue({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
        };
        this.accountAuthentication = {
            approveChallengeRequest(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountAuthentication_ApproveChallengeRequest"
                        }
                    };
                    self.#impl.accountAuthentication.approveChallengeRequest({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            abortChallenge(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountAuthentication_AbortChallenge"
                        }
                    };
                    self.#impl.accountAuthentication.abortChallenge({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            closeChallenge(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountAuthentication_CloseChallenge"
                        }
                    };
                    self.#impl.accountAuthentication.closeChallenge({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
            respondToChallengeMessage(context, args) {
                return new Promise((_resolve, _reject) => {
                    const responseCtx = {
                        response: {
                            resolve(val) {
                                _resolve(val);
                            },
                            reject(val) {
                                _reject(val);
                            }
                        }
                    };
                    const requestCtx = {
                        request: {
                            id: uuid(),
                            type: "AccountAuthentication_RespondToChallengeMessage"
                        }
                    };
                    self.#impl.accountAuthentication.respondToChallengeMessage({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
                });
            },
        };
    }
}
