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
    await operation.save();
    await channel.close();
}, "BerytusAccountCreationOperation succeeds at closing operation");

promise_test(async (t) => {
    const { operation, channel } = await operationCtx();
    await operation.save();
    const field = new BerytusForeignIdentityField("email", {
        kind: "EmailAddress",
        private: false
    });
    const prom = operation.addFields(field);
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom, Promise.resolve()]),
        'addFields should have returned an already-rejected promise.');

    await channel.close();
}, "BerytusAccountCreationOperation disables creation of fields after closure.");

promise_test(async (t) => {
    const { operation, channel } = await operationCtx();

    const createdField = new BerytusIdentityField("username", {
        private: false,
        humanReadable: true,
        maxLength: 16
    });
    await operation.addFields(createdField);
    await operation.save();

    const prom = operation.rejectAndReviseFields({
        field: createdField,
        reason: "IdentityAlreadyExists"
    });
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom, Promise.resolve()]),
        'rejectAndReviseFields should have returned an already-rejected promise.');

    await channel.close();
}, "BerytusAccountCreationOperation disables rejection of fields after closure.");

promise_test(async (t) => {
    const { operation, channel } = await operationCtx();
    await operation.save();

    const prom =  operation.setUserAttributes([
        {
            id: "givenName",
            value: "Cherry"
        }
    ]);
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom, Promise.resolve()]),
        'setUserAttributes should have returned an already-rejected promise.');

    await channel.close();
}, "BerytusAccountCreationOperation disables update of user attributes after closure.");

promise_test(async (t) => {
    const { operation, channel } = await operationCtx();
    await operation.save();

    const prom1 = operation.setCategory("admin");
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom1, Promise.resolve()]),
        'setCategory should have returned an already-rejected promise.');

    const prom2 = operation.setStatus("Created");
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom2, Promise.resolve()]),
        'setStatus should have returned an already-rejected promise.');

    const prom3 = operation.setVersion(10);
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom3, Promise.resolve()]),
        'setVersion should have returned an already-rejected promise.');

    const prom4 = operation.setChangePasswordUrl("https://example.tld");
    await promise_rejects_dom(
        t, 'InvalidStateError', Promise.race([prom4, Promise.resolve()]),
        'setChangePasswordUrl should have returned an already-rejected promise.');

    await channel.close();
}, "BerytusAccountCreationOperation disables update of account metadata");

// TODO(berytus): Test transition to auth operation.