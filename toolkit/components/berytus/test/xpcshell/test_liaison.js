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

    Assert.equal(liaison.managers.length, 0);

    const handlerProxy = createRequestHandlerProxy();
    liaison.registerManager(
        "alichry@sample-manager",
        "SampleManager",
        1,
        handlerProxy
    );
    Assert.equal(liaison.managers.length, 1);

    const handlerProxy2 = createRequestHandlerProxy();
    liaison.registerManager(
        "alichry@2nd-sample-manager",
        "2ndSampleManager",
        0,
        handlerProxy2
    );
    Assert.equal(liaison.managers.length, 2);

    Assert.equal(liaison.managers[0].id, "alichry@sample-manager");
    Assert.equal(liaison.managers[0].type, 1);
    Assert.equal(liaison.managers[0].label, "SampleManager");
    Assert.equal(
        liaison.getRequestHandler("alichry@sample-manager")
            instanceof PublicRequestHandler,
        true
    );

    Assert.equal(liaison.managers[1].id, "alichry@2nd-sample-manager");
    Assert.equal(liaison.managers[1].type, 0);
    Assert.equal(liaison.managers[1].label, "2ndSampleManager");
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
    Assert.equal(liaison.managers.length, 1);

    Assert.equal(liaison.managers[0].id, "alichry@2nd-sample-manager");
    Assert.equal(liaison.managers[0].type, 0);
    Assert.equal(liaison.managers[0].label, "2ndSampleManager");
    Assert.equal(
        liaison.getRequestHandler("alichry@2nd-sample-manager")
            instanceof PublicRequestHandler,
        true
    );

    liaison.ereaseManager("alichry@2nd-sample-manager");
    Assert.equal(liaison.managers.length, 0);
});

add_task(async function test_register_manager_validates_input() {
    const handlerProxy = createRequestHandlerProxy();

    // ID:
    Assert.throws(() => {
        liaison.registerManager("", "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("   ", "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(   \\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(undefined, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(null, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager({}, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(5, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(${5}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(true, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(() => {}, "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(\\(\\) => \\{\\}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager(Symbol.for("B"), "A Label", 1, handlerProxy);
    }, new RegExp(`ID \\(Symbol\\(B\\)\\) is invalid`));

    // Label:
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "", 1, handlerProxy);
    }, new RegExp(`Label \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "  ", 1, handlerProxy);
    }, new RegExp(`Label \\(  \\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", undefined, 1, handlerProxy);
    }, new RegExp(`Label \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", null, 1, handlerProxy);
    }, new RegExp(`Label \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", {}, 1, handlerProxy);
    }, new RegExp(`Label \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", 5, 1, handlerProxy);
    }, new RegExp(`Label \\(${5}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", true, 1, handlerProxy);
    }, new RegExp(`Label \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", Symbol.for("S"), 1, handlerProxy);
    }, new RegExp(`Label \\(Symbol\\(S\\)\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", () => {}, 1, handlerProxy);
    }, new RegExp(`Label \\(\\(\\) => \\{\\}\\) is invalid`));

    // Type:
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", -1, handlerProxy);
    }, new RegExp(`Type \\(${-1}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", 2, handlerProxy);
    }, new RegExp(`Type \\(${2}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", "", handlerProxy);
    }, new RegExp(`Type \\(\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", undefined, handlerProxy);
    }, new RegExp(`Type \\(${undefined}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", null, handlerProxy);
    }, new RegExp(`Type \\(${null}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", {}, handlerProxy);
    }, new RegExp(`Type \\(\\[object Object\\]\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", true, handlerProxy);
    }, new RegExp(`Type \\(${true}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", () => {}, handlerProxy);
    }, new RegExp(`Type \\(\\(\\) => \\{\\}\\) is invalid`));
    Assert.throws(() => {
        liaison.registerManager("alichry@sample-manager", "A Label", Symbol.for("V"), handlerProxy);
    }, new RegExp(`Type \\(Symbol\\(V\\)\\) is invalid`));
});