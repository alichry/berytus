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
                    self.preCall("manager", "getSigningKey", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("manager", "getSigningKey", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("manager", "getSigningKey", { context, args });
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
                    self.preCall("manager", "getCredentialsMetadata", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("manager", "getCredentialsMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("manager", "getCredentialsMetadata", { context, args });
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
                    self.preCall("channel", "generateKeyExchangeParameters", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("channel", "generateKeyExchangeParameters", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("channel", "generateKeyExchangeParameters", { context, args });
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
                    self.preCall("channel", "enableEndToEndEncryption", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("channel", "enableEndToEndEncryption", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("channel", "enableEndToEndEncryption", { context, args });
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
                    self.preCall("channel", "closeChannel", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("channel", "closeChannel", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("channel", "closeChannel", { context });
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
                    self.preCall("login", "approveOperation", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("login", "approveOperation", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("login", "approveOperation", { context, args });
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
                    self.preCall("login", "closeOpeation", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("login", "closeOpeation", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("login", "closeOpeation", { context });
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
                    self.preCall("login", "getRecordMetadata", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("login", "getRecordMetadata", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("login", "getRecordMetadata", { context });
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
                    self.preCall("login", "updateMetadata", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("login", "updateMetadata", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("login", "updateMetadata", { context, args });
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
                    self.preCall("accountCreation", "approveTransitionToAuthOp", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountCreation", "approveTransitionToAuthOp", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountCreation", "approveTransitionToAuthOp", { context, args });
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
                    self.preCall("accountCreation", "getUserAttributes", { context });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountCreation", "getUserAttributes", { context });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountCreation", "getUserAttributes", { context });
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
                    self.preCall("accountCreation", "addField", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountCreation", "addField", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountCreation", "addField", { context, args });
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
                    self.preCall("accountCreation", "rejectFieldValue", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountCreation", "rejectFieldValue", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountCreation", "rejectFieldValue", { context, args });
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
                    self.preCall("accountAuthentication", "approveChallengeRequest", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountAuthentication", "approveChallengeRequest", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountAuthentication", "approveChallengeRequest", { context, args });
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
                    self.preCall("accountAuthentication", "abortChallenge", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountAuthentication", "abortChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountAuthentication", "abortChallenge", { context, args });
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
                    self.preCall("accountAuthentication", "closeChallenge", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountAuthentication", "closeChallenge", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountAuthentication", "closeChallenge", { context, args });
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
                    self.preCall("accountAuthentication", "respondToChallengeMessage", { context, args });
                }
                catch (e) {
                    context.response.reject(e);
                    return;
                }
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            try {
                                self.preResolve("accountAuthentication", "respondToChallengeMessage", { context, args });
                            }
                            catch (e) {
                                context.response.reject(e);
                                throw e;
                            }
                            context.response.resolve(val);
                        },
                        reject(val) {
                            try {
                                self.preReject("accountAuthentication", "respondToChallengeMessage", { context, args });
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
        response.reject(new Components.Exception("Error sending request to secret manager. "
            + "The secret manager unexpectedly threw an exception "
            + "instead of using the reject callback.", Cr.NS_ERROR_FAILURE));
    }
}
export class ValidatedRequestHandler extends IsolatedRequestHandler {
    constructor(impl) {
        // TODO(berytus): ensure impl is conformant
        super(impl);
    }
    preCall(group, method, args) {
        // TODO(berytus): validate input
    }
    preResolve(group, method, value) {
        // TODO(berytus): validate output
    }
    preReject(group, method, value) {
        // TODO(berytus): validate error value
    }
}
export class SequentialRequestHandler extends ValidatedRequestHandler {
    busy = false;
    preCall(group, method, args) {
        if (this.busy) {
            throw new Components.Exception('Refusing to send request to secret manager, '
                + 'an existing request is still pending. '
                + 'make sure to send one request at a time '
                + 'to the secret manager.', Cr.NS_ERROR_FAILURE);
        }
        super.preCall(group, method, args);
        this.busy = true;
    }
    preResolve(group, method, value) {
        this.busy = false;
        super.preResolve(group, method, value);
    }
    preReject(group, method, value) {
        this.busy = false;
        super.preReject(group, method, value);
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
