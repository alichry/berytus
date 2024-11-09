/* -*- Mode: indent-tabs-mode: nil; js-indent-level: 2 -*- */
/* vim: set sts=2 sw=2 et tw=80: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

var { ExtensionError } = ExtensionUtils;

/** @type {Lazy} */
const lazy = {};

ChromeUtils.defineESModuleGetters(lazy, {
  Schemas: "resource://gre/modules/Schemas.sys.mjs"
});

/**
 * @type {import('../../berytus/src/Liaison.sys.mjs')}
 */
const { liaison } = ChromeUtils.importESModule(
  "resource://gre/modules/BerytusLiaison.sys.mjs"
);


/**
 * @typedef {import("../../berytus/src/types.ts").PreliminaryRequestContext} PreliminaryRequestContext
 * @typedef {{ group: string; method: string; key: string; }} RequestTypeEntry
 */

/**
 * @typedef {import("../../berytus/src/types.ts").RequestHandler} RequestHandler
 * @typedef {import("../../berytus/src/types.ts").IUnderlyingRequestHandler} IUnderlyingRequestHandler
 * @typedef {import("../../berytus/src/types.ts").ResponseContext<keyof RequestHandler, keyof RequestHandler[keyof RequestHandler]>} ResponseContext
 * @typedef {import("../../berytus/src/types.ts").PreliminaryRequestContext & ResponseContext} Context
 */

this.berytus = class BerytusExtensionAPI extends ExtensionAPIPersistent {
  /**
   * @type {Array<RequestTypeEntry>}
   */
  static #requestTypes;

  /**
   * @type {Record<string, Context>}
   */
  #requests = {};

  /**
   * @type {Record<string, Map<<(...args: any[]) => any, { fire: Fire, context: any }>>}
   */
  listeners;

  /**
   * @type {IUnderlyingRequestHandler}
   */
  #liaisonHandler;

  /**
   * @typedef {Object} Fire
   * @property {(...args: any[]) => any} async
   *
   * @typedef {Object} PersistentEventHandlerArg
   * @property {Fire} fire
   * @property {any} context
   *
   * @typedef {Object} PersistentEventHandlerResult
   * @property {() => void} unregister
   * @property {(fire: Fire, context: any) => void} convert
   *
   * @type {Record<string, (arg: { fire: Fire, context: any })}
   */
  PERSISTENT_EVENTS;

  constructor(extension) {
    super(extension);
    this.#initListeners();
    this.#initPersistentEvents();
    this.#initLiaisonHandler();
    extension.callOnClose({
      close() {
        console.log("ParentAuthRealmAPI Extension closing (0)");
      }
    });
  }

  #initListeners() {
    const types = this.#getRequestTypes();
    this.listeners = {};
    types.forEach(type => {
      this.listeners[type.key] = new Map();
    });
  }

  #initPersistentEvents() {
    const types = this.#getRequestTypes();
    this.PERSISTENT_EVENTS = {};
    types.forEach(requestType => {
      /**
       * @param {PersistentEventHandlerArg} param0
       * @returns {PersistentEventHandlerResult}
       */
      const handler = ({ fire, context }) => {
        const requestTypeListeners = this.listeners[requestType.key];
        requestTypeListeners.set(fire, { fire, context });
        return {
          unregister: () => {
            requestTypeListeners.delete(fire);
          },
          convert: (_fire, _context) => {
            requestTypeListeners.delete(fire);
            requestTypeListeners.set(_fire, {
              fire: _fire,
              context: _context
            });
          }
        }
      };
      this.PERSISTENT_EVENTS[requestType.key] = handler;
    });
  }

  #initLiaisonHandler() {
    this.#liaisonHandler = {};
    const types = this.#getRequestTypes();
    types.forEach(requestType => {
      let groupHandler = this.#liaisonHandler[requestType.group];
      if (!groupHandler) {
        this.#liaisonHandler[requestType.group] = groupHandler = {};
      }
      const methodHandler = (...args) => {
        this.#fireListeners(requestType, ...args);
      }
      groupHandler[requestType.method] = methodHandler;
    });
  }

  /**
   *
   * @param {RequestTypeEntry} requestType
   * @param {Context} cx
   * @param  {...any} args
   */
  #fireListeners(requestType, cx, ...args) {
    const requestTypeListeners = this.listeners[requestType.key];
    let { group, method } = requestType;
    group = group.charAt(0).toLowerCase() + group.substring(1);
    method = method.charAt(0).toLowerCase() + method.substring(1);
    if (group !== requestType.group) {
      throw new ExtensionError('Request Group did not match with that of the Listener\'s Group');
    }
    if (method !== requestType.method) {
      throw new ExtensionError('Request Method did not match with that of the Listener\'s Method');
    }
    if (requestTypeListeners.size === 0) {
      cx.response.reject("GeneralError2");
      return;
    }
    this.#requests[cx.request.id] = {
      ...cx,
      response: {
        resolve: (value) => {
          cx.response.resolve(value);
        },
        reject: (err) => {
          cx.response.reject(err);
        }
      }
    }
    requestTypeListeners.forEach(({ fire: cb, context }) => {
      const cx2 = { ...cx };
      delete cx2.response;
      cb.async(cx2, ...args);
    });
  }

  /**
   * @returns {Array<RequestTypeEntry>}
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

  getAPI(context) {

    /**
     * @type {Record<string, any>}
     */
    const eventManagers = {};
    /**
     * @type {Record<string, any>}
     */
    const events = {};
    this.#getRequestTypes().forEach(type => {
      const eventName = type.key;
      const manager = new EventManager({
        context,
        module: "berytus",
        event: eventName,
        name: `berytus.${eventName}`,
        extensionApi: this
      });
      eventManagers[eventName] = manager;
      events[eventName] = manager.api();
    });

    return {
      berytus: {
        ...events,
        resolveRequest: (requestId, value) => {
          const cx = this.#requests[requestId];
          if (!cx) {
            throw new Error('Invalid request id; cannot resolve request.');
          }
          cx.response.resolve(value);
        },
        rejectRequest: (requestId, reason) => {
          const cx = this.#requests[requestId];
          if (!cx) {
            throw new Error('Invalid request id; cannot reject request.');
          }
          cx.response.reject(reason);
        },
        register: () => {
          if (liaison.isManagerRegistered(this.extension.id)) {
            throw new Error('Extension already registered; cannot register.');
          }
          liaison.registerManager(
            this.extension.id,
            'Test',
            1,
            this.#liaisonHandler
          );
        },
        unregister: () => {
          if (!liaison.isManagerRegistered(this.extension.id)) {
            throw new Error('Extension is not registered; cannot unregister.');
          }
          liaison.ereaseManager(this.extension.id);
        },
        isListenerRegistered: (eventName) => {
          const eventManager = eventManagers[eventName];
          if (!eventManager) {
            throw new Error("Unknown event name");
          }
          return eventManager.unregister.size > 0;
        },
      }
    }
  }
}