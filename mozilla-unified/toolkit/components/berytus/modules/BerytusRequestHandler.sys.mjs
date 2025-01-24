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
                                await self.preResolve("manager", "getSigningKey", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("manager", "getSigningKey", { context, args }, val);
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
                                await self.preResolve("manager", "getCredentialsMetadata", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("manager", "getCredentialsMetadata", { context, args }, val);
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
                                await self.preResolve("channel", "generateKeyExchangeParameters", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "generateKeyExchangeParameters", { context, args }, val);
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
                                await self.preResolve("channel", "enableEndToEndEncryption", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "enableEndToEndEncryption", { context, args }, val);
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
                                await self.preResolve("channel", "closeChannel", { context }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("channel", "closeChannel", { context }, val);
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
                                await self.preResolve("login", "approveOperation", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "approveOperation", { context, args }, val);
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
            async closeOperation(context) {
                try {
                    await self.preCall("login", "closeOperation", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("login", "closeOperation", { context }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "closeOperation", { context }, val);
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
                    await self.#impl.login.closeOperation({
                        ...context,
                        ...wrappedResponseCtx
                    });
                }
                catch (e) {
                    self.handleUnexpectedException("login", "closeOperation", context.response, e);
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
                                await self.preResolve("login", "getRecordMetadata", { context }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "getRecordMetadata", { context }, val);
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
                                await self.preResolve("login", "updateMetadata", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("login", "updateMetadata", { context, args }, val);
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
                                await self.preResolve("accountCreation", "approveTransitionToAuthOp", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "approveTransitionToAuthOp", { context, args }, val);
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
                                await self.preResolve("accountCreation", "getUserAttributes", { context }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "getUserAttributes", { context }, val);
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
            async updateUserAttributes(context, args) {
                try {
                    await self.preCall("accountCreation", "updateUserAttributes", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        async resolve(val) {
                            try {
                                await self.preResolve("accountCreation", "updateUserAttributes", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "updateUserAttributes", { context, args }, val);
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
                    await self.#impl.accountCreation.updateUserAttributes({
                        ...context,
                        ...wrappedResponseCtx
                    }, args);
                }
                catch (e) {
                    self.handleUnexpectedException("accountCreation", "updateUserAttributes", context.response, e);
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
                                await self.preResolve("accountCreation", "addField", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "addField", { context, args }, val);
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
                                await self.preResolve("accountCreation", "rejectFieldValue", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountCreation", "rejectFieldValue", { context, args }, val);
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
                                await self.preResolve("accountAuthentication", "approveChallengeRequest", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "approveChallengeRequest", { context, args }, val);
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
                                await self.preResolve("accountAuthentication", "abortChallenge", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "abortChallenge", { context, args }, val);
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
                                await self.preResolve("accountAuthentication", "closeChallenge", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "closeChallenge", { context, args }, val);
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
                                await self.preResolve("accountAuthentication", "respondToChallengeMessage", { context, args }, val);
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw new Error(e.reason || "ResolutionFailure");
                            }
                            context.response.resolve(val);
                        },
                        async reject(val) {
                            try {
                                await self.preReject("accountAuthentication", "respondToChallengeMessage", { context, args }, val);
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
    preCall(group, method, input) { }
    preResolve(group, method, input, value) { }
    preReject(group, method, input, value) { }
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
const requestIs = (requestType, d) => {
    return d.context.request.type === requestType;
};
const inputIs = (requestType, input) => {
    return input.context.request.type === requestType;
};
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
                choicesMessage = `.${choicePath} must ${typeof choicesMessage === "function" ? choicesMessage() : choicesMessage}`;
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
class ResolutionError extends Error {
    reason;
    constructor(msg, reason) {
        super(msg + " Reason: " + reason);
        this.reason = reason;
    }
    get name() {
        return "ResolutionError";
    }
}
class FieldIdValidator {
    /**
     * op id -> field id -> true
     * TODO(berytus): There should be a cleanup mechanism.
     */
    #fields;
    constructor() {
        this.#fields = {};
    }
    #fieldExists(operationId, fieldId) {
        if (!(operationId in this.#fields)) {
            return false;
        }
        if (!(fieldId in this.#fields[operationId])) {
            return false;
        }
        return true;
    }
    #addField(operationId, fieldId) {
        if (!(operationId in this.#fields)) {
            this.#fields[operationId] = {};
        }
        if (!(fieldId in this.#fields[operationId])) {
            this.#fields[operationId][fieldId] = true;
        }
    }
    #removeField(operationId, fieldId) {
        if (!(operationId in this.#fields)) {
            return;
        }
        if (!(fieldId in this.#fields[operationId])) {
            return;
        }
        delete this.#fields[operationId][fieldId];
    }
    async consume(group, method, input) {
        if (inputIs("AccountCreation_AddField", input)) {
            const { operation } = input.context;
            const { field } = input.args;
            if (this.#fieldExists(operation.id, field.id)) {
                throw new Error(`Illegal field creation request; Passed field id (${field.id}) already exists.`);
            }
            this.#addField(operation.id, field.id);
            return;
        }
        if (inputIs("AccountCreation_RejectFieldValue", input)) {
            const { operation } = input.context;
            const { fieldId } = input.args;
            if (!this.#fieldExists(operation.id, fieldId)) {
                throw new Error(`Illegal field creation request; Passed field id (${fieldId}) does not exist.`);
            }
            return;
        }
    }
    async digest(group, method, input, output) { }
    async rollback(group, method, input) {
        if (inputIs("AccountCreation_AddField", input)) {
            const { operation } = input.context;
            const { field } = input.args;
            this.#removeField(operation.id, field.id);
            return;
        }
    }
}
class ChallangeMessagingSequenceValidator {
    /**
     * op id -> ch id -> nb of messages sent
     * TODO(berytus): There should be a cleanup mechanism.
     */
    #counter;
    constructor() {
        this.#counter = {};
    }
    #defineRecord(operationId, challengeId) {
        if (!(operationId in this.#counter)) {
            this.#counter[operationId] = {};
        }
        if (!(challengeId in this.#counter[operationId])) {
            this.#counter[operationId][challengeId] = 0;
        }
    }
    #count(operationId, challengeId) {
        this.#defineRecord(operationId, challengeId);
        return this.#counter[operationId][challengeId];
    }
    #increment(operationId, challengeId) {
        this.#defineRecord(operationId, challengeId);
        this.#counter[operationId][challengeId] += 1;
    }
    #decrement(operationId, challengeId) {
        this.#defineRecord(operationId, challengeId);
        this.#counter[operationId][challengeId] -= 1;
    }
    async consume(group, method, input) {
        if (!inputIs("AccountAuthentication_RespondToChallengeMessage", input)) {
            return;
        }
        const { operation } = input.context;
        const { challenge } = input.args;
        switch (challenge.type) {
            case "Identification": {
                const names = ["GetIdentityFields"];
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
            }
            case "Password": {
                const names = ["GetPasswordFields"];
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
            }
            case "DigitalSignature": {
                const names = ["SelectKey", "SignNonce"];
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
            }
            case "SecureRemotePassword": {
                const names = ["SelectSecurePassword", "ExchangePublicKeys", "ComputeClientProof", "VerifyServerProof"];
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
            }
            case "OffChannelOtp": {
                const names = ["GetOtp"];
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
            }
        }
    }
    async digest(group, method, input, output) { }
    async rollback(group, method, input) {
        if (!inputIs("AccountAuthentication_RespondToChallengeMessage", input)) {
            return;
        }
        const { challenge } = input.args;
        const { operation } = input.context;
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
class FieldCreationHandler {
    async consume(group, method, input) { }
    async rollback(group, method, input) { }
    async digest(group, method, input, output) {
        const errorPrefix = `Malformed output passed from the request handler's `
            + `${group}:${method} method.`;
        if (inputIs("AccountCreation_AddField", input)) {
            const { value: dictatedValue } = input.args.field;
            if (dictatedValue === null) {
                if (output === null) {
                    throw new ResolutionError(errorPrefix, "resolved value must not be null since the web app did not dictate a field value.");
                }
            }
            else {
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
            }
            else {
                if (output !== null) {
                    throw new ResolutionError(errorPrefix, "resolved value must be null since the web app has dictated a revised field value.");
                }
            }
            return;
        }
    }
}
export class ValidatedRequestHandler extends IsolatedRequestHandler {
    #schema;
    #validators;
    constructor(impl) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
        this.#validators = [];
        this.#validators.push(new FieldIdValidator(), new ChallangeMessagingSequenceValidator(), new FieldCreationHandler());
    }
    #validateValue(typeEntry, value, message) {
        const { error } = typeEntry.normalize(value, new ValidationContext());
        if (error) {
            throw new ResolutionError(message, (typeof error === "function" ? error() : error));
        }
    }
    async #consumeValidators(group, method, input) {
        await Promise.all(this.#validators.map(val => val.consume(group, method, input)));
    }
    async #digestValidators(group, method, input, output) {
        await Promise.all(this.#validators.map(val => val.digest(group, method, input, output)));
    }
    async #rollbackValidators(group, method, input) {
        await Promise.all(this.#validators.map(val => val.rollback(group, method, input)));
    }
    async preCall(group, method, input) {
        const methodType = await this.#getMethodTypeEntry(group, method);
        const { parameters } = methodType;
        if (parameters.length === 0) {
            throw new Error(`Berytus Schema's RequestHandler type did not '
                + 'contain parameters for ${group}:${method}.`);
        }
        const message = `Malformed input passed to the request handler's `
            + `${group}:${method} method.`;
        this.#validateValue(parameters[0].type, input.context, message);
        if (parameters[1]) {
            this.#validateValue(parameters[1].type, input.args, message);
        }
        await this.#consumeValidators(group, method, input);
        try {
            await super.preCall(group, method, input);
        }
        catch (e) {
            await this.#rollbackValidators(group, method, input);
            throw e;
        }
    }
    async preResolve(group, method, input, output) {
        const resultType = await this.#getMethodResultTypeEntry(group, method);
        const errorPrefix = `Malformed output passed from the request handler's `
            + `${group}:${method} method.`;
        this.#validateValue(resultType, output, errorPrefix);
        const data = { ...input, output };
        if (requestIs("AccountCreation_AddField", data)) {
            const fieldTypeEntry = await this.#getFieldTypeEntry(data.args.field.type);
            this.#validateValue(fieldTypeEntry.properties.value.type, output, errorPrefix);
            if (data.args.field.value === null) {
                if (data.output === null) {
                    throw new ResolutionError(errorPrefix, "resolved value must not be null since the web app did not dictate a field value.");
                }
            }
            else {
                if (data.output !== null) {
                    throw new ResolutionError(errorPrefix, "resolved value must be null since the web app has dictated a field value.");
                }
            }
        }
        await super.preResolve(group, method, input, output);
    }
    async preReject(group, method, input, value) {
        // TODO(berytus): validate error value
        await this.#rollbackValidators(group, method, input);
        await super.preReject(group, method, input, value);
    }
    async #getMethodResultTypeEntry(group, method) {
        group = group.charAt(0).toUpperCase() + group.substring(1);
        method = method.charAt(0).toUpperCase() + method.substring(1);
        const schema = await this.#getSchema();
        const id = group + method + "Result";
        let resultType = schema.get(id);
        if (!resultType) {
            throw new Error(`Berytus Schema did not contain a "${id}" type.`);
        }
        return resultType;
    }
    async #getMethodTypeEntry(group, method) {
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
    async #getFieldTypeEntry(fieldType) {
        const schema = await this.#getSchema();
        const id = `Berytus${fieldType}Field`;
        const fieldTypeEntry = schema.get(id);
        if (!fieldTypeEntry) {
            throw new Error(`Berytus Schema did not contain a "${id}" type.`);
        }
        return fieldTypeEntry;
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
        try {
            await super.preCall(group, method, input);
        }
        catch (e) {
            this.busy = false;
            throw e;
        }
    }
    async preResolve(group, method, input, value) {
        this.busy = false;
        await super.preResolve(group, method, input, value);
    }
    async preReject(group, method, input, value) {
        this.busy = false;
        await super.preReject(group, method, input, value);
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
            closeOperation(context) {
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
                            type: "Login_CloseOperation"
                        }
                    };
                    self.#impl.login.closeOperation({
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
            updateUserAttributes(context, args) {
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
                            type: "AccountCreation_UpdateUserAttributes"
                        }
                    };
                    self.#impl.accountCreation.updateUserAttributes({
                        ...context,
                        ...responseCtx,
                        ...requestCtx
                    }, args);
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
