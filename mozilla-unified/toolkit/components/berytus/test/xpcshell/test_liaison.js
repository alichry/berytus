/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

add_task(async function test_register_manager_stores_passed_data() {
    /**
     * @type {import('../../src/RequestHandler.mjs')}
     */
    const { PublicRequestHandler } = ChromeUtils.importESModule(
        "resource://gre/modules/BerytusRequestHandler.sys.mjs"
    );

    const defaultManagers = 1;
    Assert.equal(liaison.managers.length, defaultManagers);

    const handlerProxy = createRequestHandlerProxy();
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1
        },
        handlerProxy
    );
    Assert.equal(liaison.managers.length, defaultManagers + 1);

    const handlerProxy2 = createRequestHandlerProxy();
    liaison.registerManager(
        {
            id: "alichry@2nd-sample-manager",
            name: "2ndSampleManager",
            type: 0
        },
        handlerProxy2
    );
    Assert.equal(liaison.managers.length, defaultManagers + 2);

    Assert.equal(liaison.managers[1].id, "alichry@sample-manager");
    Assert.equal(liaison.managers[1].type, 1);
    Assert.equal(liaison.managers[1].name, "SampleManager");
    Assert.equal(
        liaison.getRequestHandler("alichry@sample-manager")
            instanceof PublicRequestHandler,
        true
    );

    Assert.equal(liaison.managers[2].id, "alichry@2nd-sample-manager");
    Assert.equal(liaison.managers[2].type, 0);
    Assert.equal(liaison.managers[2].name, "2ndSampleManager");
    Assert.equal(
        liaison.getRequestHandler("alichry@2nd-sample-manager")
            instanceof PublicRequestHandler,
        true
    );

    Assert.notStrictEqual(
        liaison.getRequestHandler("alichry@sample-manager"),
        liaison.getRequestHandler("alichry@2nd-sample-manager"),
    );

    liaison.ereaseManager("alichry@sample-manager");
    Assert.equal(liaison.managers.length, defaultManagers + 1);

    Assert.equal(liaison.managers[1].id, "alichry@2nd-sample-manager");
    Assert.equal(liaison.managers[1].type, 0);
    Assert.equal(liaison.managers[1].name, "2ndSampleManager");
    Assert.equal(
        liaison.getRequestHandler("alichry@2nd-sample-manager")
            instanceof PublicRequestHandler,
        true
    );

    liaison.ereaseManager("alichry@2nd-sample-manager");
    Assert.equal(liaison.managers.length, defaultManagers);
});

add_task(async function test_register_manager_validates_input() {
    const handlerProxy = createRequestHandlerProxy();

    // ID:
    Assert.throws(() => {
        liaison.registerManager({ id: "", name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "   ", name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(   \\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: undefined, name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: null, name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: {}, name: "A Label", label: 1  }, handlerProxy);
    }, new RegExp(`ID \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: 5, name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(${5}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: true, name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: () => {}, name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(\\(\\) => \\{\\}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: Symbol.for("B"), name: "A Label", type: 1 }, handlerProxy);
    }, new RegExp(`ID \\(Symbol\\(B\\)\\) is invalid`));

    // Label:
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "", type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "  ", type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(  \\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: undefined, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: null, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: {}, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: 5, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(${5}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: true, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: Symbol.for("S"), type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(Symbol\\(S\\)\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: () => {}, type: 1 }, handlerProxy);
    }, new RegExp(`Name \\(\\(\\) => \\{\\}\\) is invalid`));

    // Type:
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: -1 }, handlerProxy);
    }, new RegExp(`Type \\(${-1}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: 2 }, handlerProxy);
    }, new RegExp(`Type \\(${2}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: "" }, handlerProxy);
    }, new RegExp(`Type \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: undefined }, handlerProxy);
    }, new RegExp(`Type \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: null }, handlerProxy);
    }, new RegExp(`Type \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: {} }, handlerProxy);
    }, new RegExp(`Type \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: true }, handlerProxy);
    }, new RegExp(`Type \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: () => {} }, handlerProxy);
    }, new RegExp(`Type \\(\\(\\) => \\{\\}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({ id: "alichry@sample-manager", name: "A Label", type: Symbol.for("V") }, handlerProxy);
    }, new RegExp(`Type \\(Symbol\\(V\\)\\) is invalid`));
});

add_task(async function test_isolation() {
    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            do_timeout(0, () => {
                cx.response.resolve(7);
            });
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1
        },
        handlerProxy
    );
    const h1 = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const h2 = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    Assert.notStrictEqual(h1, h2);

    h1.manager.getCredentialsMetadata = () => {};

    Assert.notStrictEqual(
        h1.manager.getCredentialsMetadata,
        h2.manager.getCredentialsMetadata,
    );
});