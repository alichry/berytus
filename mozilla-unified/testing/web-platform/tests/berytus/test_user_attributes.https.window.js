/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});

const operationCtx = async () => {
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
        intent: "Register"
    });
    return { channel, operation };
}

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    await operation.setUserAttributes([
        {
            id: "familyName",
            value: "Cherry"
        },
        {
            id: "name",
            value: "Ali"
        }
    ]);
    assert_equals(typeof operation.userAttributes.get("familyName"), "object");
    assert_equals(typeof operation.userAttributes.get("name"), "object");
    assert_not_equals(operation.userAttributes.get("familyName"), null)
    assert_not_equals(operation.userAttributes.get("name"), null);
    assert_equals(operation.userAttributes.get("familyName").value, "Cherry");
    assert_equals(operation.userAttributes.get("name").value, "Ali");
    await channel.close();
}, "BerytusAccountCreationOperation facilitates update of user attributes");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    assert_throws_js(
        TypeError,
        async () => {
            await operation.setUserAttributes([
                {
                    id: "dummy",
                    value: "Cherry"
                }
            ]);
        }
    )
    assert_equals(operation.userAttributes.get("dummy"), undefined);
    await channel.close();
}, "BerytusAccountCreationOperation rejects id-invalid user attribute definition ");