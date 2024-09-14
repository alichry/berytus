/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

const lazy = {};

/**
 * @type {import('../../src/Liaison.sys.mjs')}
 */
const { liaison } = ChromeUtils.importESModule(
    "resource://gre/modules/BerytusLiaison.sys.mjs"
);

ChromeUtils.defineESModuleGetters(lazy, {
  setTimeout: "resource://gre/modules/Timer.sys.mjs",
});

registerCleanupFunction(() => {
    for (let i = 0; i < liaison.managers.length; i++) {
        liaison.ereaseManager(liaison.managers[i].id);
    }
})

/**
 * @param {(groupName: string, methodName: string, ...args: any[]) => void} cb
 * @returns {import('../../src/types').IUnderlyingRequestHandler}
 */
const createRequestHandlerProxy = (cb = () => {}) => {
    return new Proxy({}, {
        get(_, groupNameProp) {
            return new Proxy({}, {
                get(_, methodNameProp) {
                    return (...args) => {
                        cb(groupNameProp, methodNameProp, ...args);
                    }
                }
            })
        }
    });
};

class PromiseReference {
    resolve;
    reject;
    finished;
    timeoutInteveral;

    constructor(timeout = null) {
        this.finished = new Promise((resolve, reject) => {
            this.resolve = (val) => {
                if (this.timeoutInteveral) {
                    lazy.clearTimeout(this.timeoutInteveral);
                    this.timeoutInteveral = null;
                }
                resolve(val);
            };
            this.reject = (val) => {
                if (this.timeoutInteveral) {
                    lazy.clearTimeout(this.timeoutInteveral);
                    this.timeoutInteveral = null;
                }
                reject(val);
            };
        });
        if (timeout) {
            this.timeoutInteveral = lazy.setTimeout(() => {
                this.reject(new Error('Promise killed, timeout exceeded ' + timeout + ' seconds.'));
            }, 1000 * timeout);
        }
    }
}

const sampleRequests = {
    getCredentialsMetadata() {
        return {
            context: {
                document: { id: 4 }
            },
            args: {
                webAppActor: {
                    originalUri: {
                        hostname: "example.tld",
                        path: "/",
                        port: 443,
                        scheme: 'https:',
                        uri: 'https://example.tld/'
                    },
                    currentUri: {
                        hostname: "example.tld",
                        path: "/login",
                        port: 443,
                        scheme: 'https:',
                        uri: 'https://example.tld/login'
                    },
                },
                accountConstraints: {}
            }
        }
    }
}