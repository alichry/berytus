/* -*- Mode: indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* vim: set sts=2 sw=2 et tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

var { ExtensionError } = ExtensionUtils;

/**
 * @typedef {import("../../berytus/src/types.ts").PreliminaryRequestContext} PreliminaryRequestContext
 */

/** @type {Lazy} */
const lazy = {};

ChromeUtils.defineESModuleGetters(lazy, {
    Schemas: "resource://gre/modules/Schemas.sys.mjs"
});


this.berytus = class BerytusExtensionAPI extends ExtensionAPI {
    /**
     * @type {Array<{ group: string; method: string; key: string; }>}
     */
    static #requestTypes;

    constructor(extension) {
        super(extension);
        this.extensionRegistered = false;
        this.listeners = null;
        this.requestContexts = {};

    }

    /**
     * The parent's onShutdown handles deregisteration with
     * the Liaison.
     */
    onShutdown() {
        this.extensionRegistered = false;
        this.listeners = null;
    }

    getAPI(context) {
        /**
         * callOnClose is invoked when a non-persistent
         * background script is terminated.
         */
        context.callOnClose({
            close: () => {
                this.listeners = null;
            }
        });
        return {
            berytus: {
                registerRequestHandler: async (handler) => {

                    if (this.listeners) {
                        throw new ExtensionError(
                            "Cannot register another handler! Please make sure you call unregister before registering again."
                        );
                    }
                    this.listeners = this.createListeners(context, handler);
                    this.#addEventListeners(context, this.listeners);

                    if (this.extensionRegistered) {
                        // already registered with liaison, we just
                        // connected the listeners above.
                        return;
                    }
                    try {
                        await context.childManager.callParentAsyncFunction("berytus.register", []);
                        this.extensionRegistered = true;
                    } catch (e) {
                        this.#removeEventListeners(context, this.listeners);
                        this.listeners = null;
                        this.extensionRegistered = false;

                        throw e;
                    }
                },
                unregisterRequestHandler: async () => {
                    if (this.listeners) {
                        this.#removeEventListeners(this.listeners);
                        this.listeners = null;
                    }

                    context.childManager.callParentFunctionNoReturn("berytus.unregister", []);
                    this.extensionRegistered = false;
                },
            }
        }
    }

    createListeners(context, handler) {
        /**
         * @type {Record<string, (...args: any[]) => any}
         */
        const listeners = {};
        this.#getRequestTypes().forEach(({ group, method, key }) => {
            /**
             * @param {PreliminaryRequestContext} ctx
             * @param  {...any} args
             */
            const listener = (ctx, ...args) => {
                this.requestContexts[ctx.request.requestId] = ctx;
                const { request } = ctx;
                const { type: requestType } = request;
                let [requestGroup, requestMethod] = requestType.split("_", 2);
                requestGroup = requestGroup.charAt(0).toLowerCase() + requestGroup.substring(1);
                requestMethod = requestMethod.charAt(0).toLowerCase() + requestMethod.substring(1);
                if (requestGroup !== group) {
                    throw new ExtensionError('Request Group did not match with that of the Listener\'s Group');
                }
                if (requestMethod !== method) {
                    throw new ExtensionError('Request Method did not match with that of the Listener\'s Method');
                }
                const promise = ExtensionCommon.withHandlingUserInput(context.contentWindow, () => {
                    const reject = (e) => {
                        context.childManager.callParentFunctionNoReturn("berytus.rejectRequest", [request.id, e || "GeneralError"]);
                    }

                    let result;
                    try {
                        result = handler[group][method](ctx, ...args);
                    } catch (e) {
                        reject(e);
                        return;
                    }
                    return Promise.resolve(result)
                        .then((result) => {
                            context.childManager.callParentFunctionNoReturn("berytus.resolveRequest", [request.id, result]);
                        })
                        .catch((e) => {
                            reject(e);
                        });
                });
                // delete the requestContext to indicate that *WE* no longer welcome calls
                // to `openPageActionPopupIfNecessary`.
                delete this.requestContexts[ctx.request.requestId];
                return promise;
            }
            listeners[key] = listener;
        });
        return listeners;
    }

    #getParentEvent(context, eventName) {
        return context.childManager
            .getParentEvent(`berytus.${eventName}`);
    }

    /**
     * This routine assumes the context is still open/active
     * @param {object} eventListeners { [eventName: string]: Function } keys are event names and values are functions/callbacks
     */
    #addEventListeners(context, eventListeners) {
        Object.keys(eventListeners).forEach(eventName => {
            this.#getParentEvent(context, eventName)
                .addListener(eventListeners[eventName]);
        });
    }

    /**
     * This routine assumes the context is still open/active
     * @param {object} eventListeners { [eventName: string]: Function } keys are event names and values are functions/callbacks
     */
    #removeEventListeners(context, eventListeners) {
        Object.keys(eventListeners).forEach(eventName => {
            this.#getParentEvent(context, eventName)
                .removeListener(eventListeners[eventName]);
        });
    }

    /**
     * @returns {typeof BerytusExtensionAPI.requestTypes}
     */
    #getRequestTypes() {
        if (BerytusExtensionAPI.#requestTypes) {
            return BerytusExtensionAPI.#requestTypes;
        }
        const schema = this.#getSchema();
        // essentially, we want to get the group names
        // that are the properties of ReqeustHandler,
        // and for each group that is an object,
        // read its properties as method names
        const mainNamespace = schema
            .find(s => s.namespace === "berytus");
        const requestHandler = mainNamespace.types
            .find(t => t.id === "RequestHandler");
        const res = [];
        Object.keys(requestHandler.properties).forEach((group) => {
            const { $ref } = requestHandler.properties[group];
            const groupTypeDef = mainNamespace.types
                .find(t => t.id === $ref);
            Object.keys(groupTypeDef.properties).forEach(method => {
                res.push({ group, method, key: `${group}:${method}` });
            });
        });
        BerytusExtensionAPI.#requestTypes = res;
        return res;
    }

    #getSchema() {
        let schema = lazy.Schemas.schemaJSON.get(
            "chrome://extensions/content/schemas/berytus.json"
        );
        if (!schema) {
            throw new ExtensionError(
                'Unable to retrieve own schema'
            );
        }
        if (StructuredCloneHolder.isInstance(schema)) {
            schema = schema.deserialize(globalThis, true);
        }
        return schema;
    }
}