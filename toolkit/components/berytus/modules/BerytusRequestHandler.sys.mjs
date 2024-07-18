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
            getSigningKey(context, args) {
                self.preCall("manager", "getSigningKey", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("manager", "getSigningKey", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("manager", "getSigningKey", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.manager.getSigningKey({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            getCredentialsMetadata(context, args) {
                self.preCall("manager", "getCredentialsMetadata", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("manager", "getCredentialsMetadata", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("manager", "getCredentialsMetadata", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.manager.getCredentialsMetadata({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
        };
        this.channel = {
            generateKeyExchangeParameters(context, args) {
                self.preCall("channel", "generateKeyExchangeParameters", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("channel", "generateKeyExchangeParameters", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("channel", "generateKeyExchangeParameters", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.channel.generateKeyExchangeParameters({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            enableEndToEndEncryption(context, args) {
                self.preCall("channel", "enableEndToEndEncryption", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("channel", "enableEndToEndEncryption", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("channel", "enableEndToEndEncryption", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.channel.enableEndToEndEncryption({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            closeChannel(context) {
                self.preCall("channel", "closeChannel", { context });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("channel", "closeChannel", { context });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("channel", "closeChannel", { context });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.channel.closeChannel({
                    ...context,
                    ...wrappedResponseCtx
                });
            },
        };
        this.login = {
            approveOperation(context, args) {
                self.preCall("login", "approveOperation", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("login", "approveOperation", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("login", "approveOperation", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.login.approveOperation({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            closeOpeation(context) {
                self.preCall("login", "closeOpeation", { context });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("login", "closeOpeation", { context });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("login", "closeOpeation", { context });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.login.closeOpeation({
                    ...context,
                    ...wrappedResponseCtx
                });
            },
            getRecordMetadata(context) {
                self.preCall("login", "getRecordMetadata", { context });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("login", "getRecordMetadata", { context });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("login", "getRecordMetadata", { context });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.login.getRecordMetadata({
                    ...context,
                    ...wrappedResponseCtx
                });
            },
            updateMetadata(context, args) {
                self.preCall("login", "updateMetadata", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("login", "updateMetadata", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("login", "updateMetadata", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.login.updateMetadata({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
        };
        this.accountCreation = {
            approveTransitionToAuthOp(context, args) {
                self.preCall("accountCreation", "approveTransitionToAuthOp", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountCreation", "approveTransitionToAuthOp", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountCreation", "approveTransitionToAuthOp", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountCreation.approveTransitionToAuthOp({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            getUserAttributes(context) {
                self.preCall("accountCreation", "getUserAttributes", { context });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountCreation", "getUserAttributes", { context });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountCreation", "getUserAttributes", { context });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountCreation.getUserAttributes({
                    ...context,
                    ...wrappedResponseCtx
                });
            },
            addField(context, args) {
                self.preCall("accountCreation", "addField", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountCreation", "addField", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountCreation", "addField", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountCreation.addField({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            rejectFieldValue(context, args) {
                self.preCall("accountCreation", "rejectFieldValue", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountCreation", "rejectFieldValue", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountCreation", "rejectFieldValue", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountCreation.rejectFieldValue({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
        };
        this.accountAuthentication = {
            approveChallengeRequest(context, args) {
                self.preCall("accountAuthentication", "approveChallengeRequest", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountAuthentication", "approveChallengeRequest", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountAuthentication", "approveChallengeRequest", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountAuthentication.approveChallengeRequest({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            abortChallenge(context, args) {
                self.preCall("accountAuthentication", "abortChallenge", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountAuthentication", "abortChallenge", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountAuthentication", "abortChallenge", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountAuthentication.abortChallenge({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            closeChallenge(context, args) {
                self.preCall("accountAuthentication", "closeChallenge", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountAuthentication", "closeChallenge", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountAuthentication", "closeChallenge", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountAuthentication.closeChallenge({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
            respondToChallengeMessage(context, args) {
                self.preCall("accountAuthentication", "respondToChallengeMessage", { context, args });
                const wrappedResponseCtx = {
                    response: {
                        resolve(val) {
                            self.preResolve("accountAuthentication", "respondToChallengeMessage", { context, args });
                            context.response.resolve(val);
                        },
                        reject(val) {
                            self.preReject("accountAuthentication", "respondToChallengeMessage", { context, args });
                            context.response.reject(val);
                        }
                    }
                };
                self.#impl.accountAuthentication.respondToChallengeMessage({
                    ...context,
                    ...wrappedResponseCtx
                }, args);
            },
        };
    }
    preCall(group, method, args) { }
    preResolve(group, method, value) { }
    preReject(group, method, value) { }
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
        if (!(impl instanceof ValidatedRequestHandler)) {
            throw new Error('Cannot construct PublicRequestHandler '
                + 'Expecting request handler to be an '
                + 'instance of ValidatedRequestHandler; '
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
