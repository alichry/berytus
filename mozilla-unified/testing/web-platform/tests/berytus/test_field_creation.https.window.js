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
    const { channel, operation } = await operationCtx();
    const field = new BerytusIdentityField(
        "username",
        {
            private: false,
            humanReadable: true,
            maxLength: 16,
            allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
        }
    );
    const { username } = await operation.addFields(field);
    assert_equals(username, field);
    assert_equals(typeof field.value, "string");
    await channel.close();
}, "BerytusIdentityField creation");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusIdentityField(
        "username",
        {
            private: false,
            humanReadable: true,
            maxLength: 16,
            allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
        },
        "jacksparrow"
    );
    const { username } = await operation.addFields(field);
    assert_equals(username, field);
    assert_equals(field.value, "jacksparrow");
    await channel.close();
}, "BerytusIdentityField insertion");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusIdentityField(
        "username",
        {
            private: false,
            humanReadable: true,
            maxLength: 16,
            allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
        }
    );
    const { username } = await operation.addFields(field);
    assert_equals(username, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { username: username2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IdentityAlreadyExists"
    })
    assert_equals(username2, field);
    assert_equals(typeof field.value, "string");
    // NOTE(berytus): This might seem like we are testing
    // the secret manager -- the implementation should not relay
    // the same value back to Berytus.
    assert_not_equals(field.value, oldValue);
    await channel.close();
}, "BerytusIdentityField revision");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusIdentityField(
        "username",
        {
            private: false,
            humanReadable: true,
            maxLength: 16,
            allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
        }
    );
    const { username } = await operation.addFields(field);
    assert_equals(username, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { username: username2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IdentityAlreadyExists",
        newValue: "webAppRevisedUsername"
    })
    assert_equals(username2, field);
    assert_equals(typeof field.value, "string");
    assert_not_equals(field.value, oldValue);
    assert_equals(field.value, "webAppRevisedUsername");
    await channel.close();
}, "BerytusIdentityField revision with dictated value");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusForeignIdentityField(
        "email",
        {
            private: false,
            kind: "EmailAddress"
        }
    );
    const { email } = await operation.addFields(field);
    assert_equals(email, field);
    assert_equals(typeof field.value, "string");
    await channel.close();
}, "BerytusForeignIdentityField creation");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusForeignIdentityField(
        "email",
        {
            private: false,
            kind: "EmailAddress",
        },
        "jack@sparrow.tld"
    );
    const { email } = await operation.addFields(field);
    assert_equals(email, field);
    assert_equals(field.value, "jack@sparrow.tld");
    await channel.close();
}, "BerytusForeignIdentityField insertion");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusForeignIdentityField(
        "email",
        {
            private: false,
            kind: "EmailAddress"
        }
    );
    const { email } = await operation.addFields(field);
    assert_equals(email, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { email: email2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IdentityAlreadyExists"
    })
    assert_equals(email2, field);
    assert_equals(typeof field.value, "string");
    assert_not_equals(field.value, oldValue);
    await channel.close();
}, "BerytusForeignIdentityField revision");


promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusForeignIdentityField(
        "email",
        {
            private: false,
            kind: "EmailAddress"
        }
    );
    const { email } = await operation.addFields(field);
    assert_equals(email, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { email: email2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IdentityAlreadyExists",
        newValue: "webAppRevisedEmail@example.tld"
    })
    assert_equals(email2, field);
    assert_equals(typeof field.value, "string");
    assert_not_equals(field.value, oldValue);
    assert_equals(field.value, "webAppRevisedEmail@example.tld");
    await channel.close();
}, "BerytusForeignIdentityField revision with dictated value");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusPasswordField(
        "password"
    );
    const { password } = await operation.addFields(field);
    assert_equals(password, field);
    assert_equals(typeof field.value, "string");
    await channel.close();
}, "BerytusPasswordField creation");


promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusPasswordField(
        "password",
        {},
        "123456789"
    );
    const { password } = await operation.addFields(field);
    assert_equals(password, field);
    assert_equals(field.value, "123456789");
    await channel.close();
}, "BerytusPasswordField insertion");


promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusPasswordField(
        "password"
    );
    const { password } = await operation.addFields(field);
    assert_equals(password, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { password: password2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IncompatiblePassword"
    });
    assert_equals(password2, field);
    assert_equals(typeof field.value, "string");
    assert_not_equals(field.value, oldValue);
    await channel.close();
}, "BerytusPasswordField revision");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusPasswordField(
        "password"
    );
    const { password } = await operation.addFields(field);
    assert_equals(password, field);
    assert_equals(typeof field.value, "string");
    const oldValue = field.value;
    const { password: password2 } = await operation.rejectAndReviseFields({
        field,
        reason: "IncompatiblePassword",
        newValue: "webAppDictatedPassword"
    });
    assert_equals(password2, field);
    assert_equals(typeof field.value, "string");
    assert_not_equals(field.value, oldValue);
    assert_equals(field.value, "webAppDictatedPassword");
    await channel.close();
}, "BerytusPasswordField revision with dictated value");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusKeyField(
        "key",
        { alg: -51 }
    );
    const { key } = await operation.addFields(field);
    assert_equals(key, field);
    assert_true(field.value.publicKey instanceof ArrayBuffer);
    await channel.close();
}, "BerytusKeyField creation");


promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusKeyField(
        "key",
        { alg: -51 }
    );
    const { key } = await operation.addFields(field);
    assert_equals(key, field);
    assert_true(field.value.publicKey instanceof ArrayBuffer);
    const oldValue = field.value;
    const { key: key2 } = await operation.rejectAndReviseFields({
        field,
        reason: "PublicKeyAlreadyExists"
    });
    assert_equals(key2, field);
    assert_true(field.value.publicKey instanceof ArrayBuffer);
    assert_not_equals(field.value.publicKey, oldValue.publicKey);
    const oldView = new Uint8Array(oldValue.publicKey);
    const newView = new Uint8Array(field.value.publicKey);
    for (let i = 0; i < oldView.length; i++) {
        if (i >= newView.length) {
            break;
        }
        const oldByte = oldView[i];
        const newByte = newView[i];
        assert_not_equals(newByte, oldByte);
    }
    await channel.close();
}, "BerytusKeyField revision");

promise_test(async (t) => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusKeyField(
        "key",
        { alg: -51 }
    );
    const { key } = await operation.addFields(field);
    assert_equals(key, field);
    assert_true(field.value.publicKey instanceof ArrayBuffer);
    const prom = operation.rejectAndReviseFields({
        field,
        reason: "PublicKeyAlreadyExists",
        newValue: "dum"
    });
    await promise_rejects_js(
        t, TypeError, Promise.race([prom, Promise.resolve()]),
        'rejectAndReviseFields should have returned an already-rejected promise.');
    await channel.close();
}, "BerytusKeyField unsuccessful revision with dictated value");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusSharedKeyField(
        "sharedKey",
        { alg: -51 }
    );
    const { sharedKey } = await operation.addFields(field);
    assert_equals(sharedKey, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    await channel.close();
}, "BerytusSharedKeyField creation");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const privateKey = new Uint8Array([1,2,3,4,5,6,7,8,9]);
    const privateKeyBuffer = privateKey.buffer;
    const field = new BerytusSharedKeyField(
        "sharedKey",
        { alg: -51 },
        new BerytusSharedKeyFieldValue(privateKeyBuffer)
    );
    const { sharedKey } = await operation.addFields(field);
    assert_equals(sharedKey, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    assert_array_equals(new Uint8Array(field.value.privateKey), privateKey);
    await channel.close();
}, "BerytusSharedKeyField insertion");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusSharedKeyField(
        "sharedKey",
        { alg: -51 }
    );
    const { sharedKey } = await operation.addFields(field);
    assert_equals(sharedKey, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    const oldValue = field.value;
    const { sharedKey: sharedKey2 } = await operation.rejectAndReviseFields({
        field,
        // TODO(berytus): Perhaps define a new error code for BerytusSharedKey
        reason: "PublicKeyAlreadyExists"
    });
    assert_equals(sharedKey2, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    assert_not_equals(field.value.privateKey, oldValue.privateKey);
    const oldView = new Uint8Array(oldValue.privateKey);
    const newView = new Uint8Array(field.value.privateKey);
    for (let i = 0; i < oldView.length; i++) {
        if (i >= newView.length) {
            break;
        }
        const oldByte = oldView[i];
        const newByte = newView[i];
        assert_not_equals(newByte, oldByte);
    }
    await channel.close();
}, "BerytusSharedKeyField revision");

promise_test(async () => {
    const { channel, operation } = await operationCtx();
    const field = new BerytusSharedKeyField(
        "sharedKey",
        { alg: -51 }
    );
    const { sharedKey } = await operation.addFields(field);
    assert_equals(sharedKey, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    const oldValue = field.value;
    const newPrivateKeyBuffer = new Uint8Array([10,20,30,40,50,60,70,80,90]).buffer;
    const { sharedKey: sharedKey2 } = await operation.rejectAndReviseFields({
        field,
        // TODO(berytus): Perhaps define a new error code for BerytusSharedKey
        reason: "PublicKeyAlreadyExists",
        newValue: new BerytusSharedKeyFieldValue(newPrivateKeyBuffer)
    });
    assert_equals(sharedKey2, field);
    assert_true(field.value.privateKey instanceof ArrayBuffer);
    assert_not_equals(field.value.privateKey, oldValue.privateKey);
    const oldView = new Uint8Array(oldValue.privateKey);
    const newView = new Uint8Array(field.value.privateKey);
    const expectedView = new Uint8Array(newPrivateKeyBuffer);
    for (let i = 0; i < oldView.length; i++) {
        if (i >= newView.length) {
            break;
        }
        const oldByte = oldView[i];
        const newByte = newView[i];
        const expectedByte = expectedView[i];
        assert_not_equals(newByte, oldByte);
        assert_equals(newByte, expectedByte);
    }
    await channel.close();
}, "BerytusSharedKeyField revision with dictated value");

// TODO(berytus): Tests for BerytusEncryptedPacket values.

// TODO(berytus): Tests for creation/rejection of multiple fields
// at the same time.