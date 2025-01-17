/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

 setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});

const operationCtx = async (intent = "Register") => {
    const actor = new BerytusAnonymousWebAppActor();
    const constraints = {
        secretManagerPublicKey: [],
        enableEndToEndEncryption: true,
    };
    const channel = await BerytusChannel.create({
        webApp: actor,
        constraints
    });
    const operation = await channel.login({
        intent
    });
    return { channel, operation };
}

promise_test(async () => {
    const { operation, channel } = await operationCtx("Authenticate");
    assert_equals(typeof operation.category, "string");
    assert_equals(typeof operation.version, "number");
    assert_equals(typeof operation.status, "string");
    assert_equals(typeof operation.changePasswordUrl, "string");
    await channel.close();
}, "BerytusAccountAuthenticationOperation provides account metadata");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const value = "admin";
    await operation.setCategory(value);
    assert_equals(operation.category, value);
    await channel.close();
}, "BerytusAccountCreationOperation facilitates update of account category metadata");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const value = "Created";
    await operation.setStatus(value);
    assert_equals(operation.status, value);
    await channel.close();
}, "BerytusAccountCreationOperation facilitates update of account status metadata");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const value = 10;
    await operation.setVersion(value);
    assert_equals(operation.version, value);
    await channel.close();
}, "BerytusAccountCreationOperation facilitates update of account version metadata");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const value = "https://example.tld";
    await operation.setChangePasswordUrl(value);
    assert_equals(operation.changePasswordUrl, value);
    await channel.close();
}, "BerytusAccountCreationOperation facilitates update of account change password url metadata");