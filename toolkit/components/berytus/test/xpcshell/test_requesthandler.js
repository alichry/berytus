/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 "use strict";

 add_task(async function test_calls_requesthandler() {
    const promise = new PromiseReference();

    const handlerProxy = createRequestHandlerProxy(
        (group, method, cx, args) => {
            Assert.equal(group, "manager");
            Assert.equal(method, "getCredentialsMetadata");
            Assert.deepEqual(cx.document, { id: 4 });
            Assert.deepEqual(args.webAppActor, {
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
            });
            Assert.deepEqual(args.accountConstraints, {});
            cx.response.resolve(7);
            promise.resolve();
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
        {
            document: { id: 4 }
        },
        {
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
    );
    Assert.equal(res, 7);

    await promise.finished;
 });