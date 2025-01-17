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
        intent: "Authenticate"
    });
    return { channel, operation };
}


promise_test(async () => {
    const { operation, channel } = await operationCtx();
    await operation.finish();
    await channel.close();
}, "BerytusAccountAuthenticationOperation succeeds at closing operation");

promise_test(async (t) => {
    const { operation, channel } = await operationCtx();
    await operation.finish();
    const challenge = new BerytusIdentificationChallenge("identification");
    const prom = operation.createChallenge(challenge);
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom, Promise.resolve()]),
        'createChallenge should have returned an already-rejected promise.');

    await channel.close();
}, "BerytusAccountAuthenticationOperation disables creation of challenges after closure.");