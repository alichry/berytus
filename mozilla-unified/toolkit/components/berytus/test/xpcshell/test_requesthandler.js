/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

"use strict";

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
        "alichry@sample-manager",
        "SampleManager",
        1,
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
        "alichry@sample-manager",
        "SampleManager",
        1,
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
        "alichry@sample-manager",
        "SampleManager",
        1,
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
        "alichry@sample-manager",
        "SampleManager",
        1,
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