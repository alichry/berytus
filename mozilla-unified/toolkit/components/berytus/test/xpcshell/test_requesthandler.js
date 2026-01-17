/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

/**
 * @var {import('../../src/Liaison.sys.mjs')} liaison
 */

add_task(async function test_calls_requesthandler() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            Assert.equal(group, "manager");
            Assert.equal(method, "getCredentialsMetadata");
            Assert.deepEqual(
                cx.document,
                sampleRequests.getCredentialsMetadata().context.document
            );
            Assert.deepEqual(
                args,
                sampleRequests.getCredentialsMetadata().args
            );
            cx.response.resolve(7);
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
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const res = await publicHandler.manager.getCredentialsMetadata(
        sampleRequests.getCredentialsMetadata().context,
        sampleRequests.getCredentialsMetadata().args
    );
    Assert.equal(res, 7);

    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_noconcurrent_requests() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

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
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const credPromise = publicHandler.manager.getCredentialsMetadata(
        sampleRequests.getCredentialsMetadata().context,
        sampleRequests.getCredentialsMetadata().args
    );
    await Assert.rejects(
        publicHandler.manager.getCredentialsMetadata(
            sampleRequests.getCredentialsMetadata().context,
            sampleRequests.getCredentialsMetadata().args
        ), /an existing request is still pending/i
    );
    Assert.equal(await credPromise, 7);

    // send another request to test if the 3rd request is fulfilledd
    Assert.equal(
        await publicHandler.manager.getCredentialsMetadata(
            sampleRequests.getCredentialsMetadata().context,
            sampleRequests.getCredentialsMetadata().args
        ),
        7
    );

    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_unexcepted_exception() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            throw new Error("This is bad");
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
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    await Assert.rejects(
        publicHandler.manager.getCredentialsMetadata(
            sampleRequests.getCredentialsMetadata().context,
            sampleRequests.getCredentialsMetadata().args
        ), /secret manager unexpectedly threw an exception/i
    );
    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_invalid_input() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            throw new Error("Should not be reached");
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
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    await Assert.rejects(
        publicHandler.manager.getCredentialsMetadata(
            null, //{ document: "Should not be a String" },
            { webAppActor: "Should not be a String" },
        ), /Malformed input passed to the request handler/i
    );
    liaison.ereaseManager("alichry@sample-manager");
})

add_task(async function test_handle_invalid_output() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const promises = [];
    const handlerProxy = createRequestHandlerProxy(
        async (group, method, cx, args) => {
            promises.push(
                Assert.rejects(
                    cx.response.resolve("Bla Bla"),
                    /^Error: Expected number instead of "Bla Bla"/i
                )
            )
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
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    await Assert.rejects(
        publicHandler.manager.getCredentialsMetadata(
            sampleRequests.getCredentialsMetadata().context,
            sampleRequests.getCredentialsMetadata().args
        ), /Malformed output passed from the request handler/i
    );
    await Promise.all(promises);
    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_invalid_output_addFields() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const promises = [];
    const handlerProxy = createRequestHandlerProxy(
        async (group, method, cx, args) => {
            /**
             * @type {import("../../src/types").AddFieldArgs}
             */
            const { field } = args;
            Assert.equal(field.value, null);
            promises.push(
                Assert.rejects(
                    cx.response.resolve(null),
                    /Value must not be null/i
                ).then(() => Assert.rejects(
                    cx.response.resolve(field),
                    /Value must either/i
                ))
            )
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1,
        },
        handlerProxy
    );
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    await Assert.rejects(
        publicHandler.accountCreation.addField(
            sampleRequests.addField().context,
            sampleRequests.addField().args
        ), /Malformed output passed from the request handler/i
    );
    await Promise.all(promises);
    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_ciphertext_input_when_e2ee_false() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();
    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            throw new Error("Should not be reached");
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1,
        },
        handlerProxy
    );
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const context = sampleRequests.addField().context;
    await Assert.rejects(
        publicHandler.accountCreation.addField(
            {
                ...context,
                channel: {
                    ...context.channel,
                    e2eeEnabled: false
                }
            },
            {
                field: {
                    ...sampleRequests.addField().args.field,
                    value: {
                        type: "JWE",
                        value: ""
                    }
                }
            }
        ), /Malformed input passed into the request handler/i
    );
    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_ciphertext_output_when_e2ee_false() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const promises = [];
    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            promises.push(
                Assert.rejects(
                    cx.response.resolve({
                        type: "JWE",
                        value: ""
                    }),
                    /resolved value must not be encrypted/i
                )
            );
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1,
        },
        handlerProxy
    );
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const context = sampleRequests.addField().context;
    await Assert.rejects(
        publicHandler.accountCreation.addField(
            {
                ...context,
                channel: {
                    ...context.channel,
                    e2eeEnabled: false
                }
            },
            sampleRequests.addField().args
        ), /Malformed output passed from the request handler/i
    );
    await Promise.all(promises);
    liaison.ereaseManager("alichry@sample-manager");
});


add_task(async function test_handle_plaintext_input_when_e2ee_true() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();
    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            throw new Error("Should not be reached");
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1,
        },
        handlerProxy
    );
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const context = sampleRequests.addField().context;
    await Assert.rejects(
        publicHandler.accountCreation.addField(
            {
                ...context,
                channel: {
                    ...context.channel,
                    e2eeEnabled: true
                }
            },
            {
                field: {
                    ...sampleRequests.addField().args.field,
                    value: "myUsername"
                }
            }
        ), /Malformed input passed into the request handler/i
    );
    liaison.ereaseManager("alichry@sample-manager");
});

add_task(async function test_handle_plaintext_output_when_e2ee_true() {
    // Need a profile to be setup; otherwise ValidatedRequestHandler
    // would not be able to retrieve the Schema.
    do_get_profile();

    const promises = [];
    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            promises.push(
                Assert.rejects(
                    cx.response.resolve("myUsername"),
                    /resolved value must be encrypted/i
                )
            )
        }
    );
    liaison.registerManager(
        {
            id: "alichry@sample-manager",
            name: "SampleManager",
            type: 1,
        },
        handlerProxy
    );
    const publicHandler = liaison.getRequestHandler(
        "alichry@sample-manager"
    );
    const context = sampleRequests.addField().context;
    await Assert.rejects(
        publicHandler.accountCreation.addField(
            {
                ...context,
                channel: {
                    ...context.channel,
                    e2eeEnabled: true
                }
            },
            sampleRequests.addField().args
        ), /Malformed output passed from the request handler/i
    );
    await Promise.all(promises);
    liaison.ereaseManager("alichry@sample-manager");
});

// TODO(berytus): other methods expecting ciphertext should be tested
// for ciphertext input/output when e2e false --> error
//     ciphertext input/output when e2e true --> OK
//     plaintext input/output when e2e true --> error
//     plaintext input/output when e2e false --> OK

// TODO(berytus): Add test that the request handler
// can solve addFields with null when the web app dictates
// a field value.