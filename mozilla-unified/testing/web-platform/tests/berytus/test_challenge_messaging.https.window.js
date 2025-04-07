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
    const challenge = new BerytusIdentificationChallenge('identification');
    assert_equals(challenge.active, false);
    await operation.challenge(challenge);
    assert_equals(challenge.active, true);
    assert_equals(operation.challenges.get('identification'), challenge);
    const { response } = await challenge.getIdentityFields(["username"]);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_equals(typeof response.username, "string");
    await challenge.seal();
    assert_equals(challenge.active, false);
    await channel.close();
}, "BerytusIdentificationChallenge messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusOffChannelOtpChallenge('otp');
    assert_equals(challenge.active, false);
    await operation.challenge(challenge);
    assert_equals(challenge.active, true);
    assert_equals(operation.challenges.get('otp'), challenge);
    const { response } = await challenge.getOtp("email");
    assert_equals(typeof response, 'string');
    await challenge.seal();
    assert_equals(challenge.active, false);
    await channel.close();
}, "BerytusOffChannelOtpChallenge messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusPasswordChallenge('password');
    assert_equals(challenge.active, false);
    await operation.challenge(challenge);
    assert_equals(challenge.active, true);
    assert_equals(operation.challenges.get('password'), challenge);
    const { response } = await challenge.getPasswordFields(["password"]);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_equals(typeof response.password, "string");
    await challenge.seal();
    assert_equals(challenge.active, false);
    await channel.close();
}, "BerytusPasswordChallenge messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusDigitalSignatureChallenge('signature');
    assert_equals(challenge.active, false);
    await operation.challenge(challenge);
    assert_equals(operation.challenges.get('signature'), challenge);
    assert_equals(challenge.active, true);

    var { response } = await challenge.selectKey("key");
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_equals(typeof response.publicKey, "object");
    assert_not_equals(response.publicKey, null);
    assert_true(response.publicKey instanceof ArrayBuffer);

    var { response } = await challenge.signNonce(new Uint8Array([55, 56, 57]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    console.log(response);
    assert_true(response instanceof ArrayBuffer);

    await challenge.seal();
    assert_equals(challenge.active, false);
    await channel.close();
}, "BerytusDigitalSignatureChallenge messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusSecureRemotePasswordChallenge('srp');
    assert_equals(challenge.active, false);
    await operation.challenge(challenge);
    assert_equals(operation.challenges.get('srp'), challenge);
    assert_equals(challenge.active, true);

    var { response } = await challenge.selectSecurePassword("securePassword");
    assert_equals(typeof response, 'string');

    var { response } = await challenge.exchangePublicKeys(new Uint8Array([55, 56, 57]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.computeClientProof(new Uint8Array([65, 66, 67]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.verifyServerProof(new Uint8Array([75, 76, 77]).buffer);
    assert_equals(typeof response, 'undefined');

    await challenge.seal();
    assert_equals(challenge.active, false);
    await channel.close();
}, "BerytusSecureRemotePasswordChallenge messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusSecureRemotePasswordChallenge('srp');
    await operation.challenge(challenge);

    var { response } = await challenge.selectSecurePassword("securePassword");
    assert_equals(typeof response, 'string');

    var { response } = await challenge.exchangePublicKeys(new Uint8Array([55, 56, 57]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.computeClientProof(new Uint8Array([65, 66, 67]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.verifyServerProof(new Uint8Array([75, 76, 77]).buffer);
    assert_equals(typeof response, 'undefined');

    await challenge.seal();
    await channel.close();
}, "BerytusSecureRemotePasswordChallenge (Unspecified encodig) messaging");

promise_test(async () => {
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusSecureRemotePasswordChallenge('srp', {
        encoding: "None"
    });
    await operation.challenge(challenge);

    var { response } = await challenge.selectSecurePassword("securePassword");
    assert_equals(typeof response, 'string');

    var { response } = await challenge.exchangePublicKeys(new Uint8Array([55, 56, 57]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.computeClientProof(new Uint8Array([65, 66, 67]).buffer);
    assert_equals(typeof response, 'object');
    assert_not_equals(response, null);
    assert_true(response instanceof ArrayBuffer);

    var { response } = await challenge.verifyServerProof(new Uint8Array([75, 76, 77]).buffer);
    assert_equals(typeof response, 'undefined');

    await challenge.seal();
    await channel.close();
}, "BerytusSecureRemotePasswordChallenge (None encodig) messaging");


promise_test(async () => {
    const assert_is_hex = (val) => {
        assert_true(/^([0-9A-F]{2})+$/i.test(val));
    };
    const { operation, channel } = await operationCtx();
    const challenge = new BerytusSecureRemotePasswordChallenge('srp', {
        encoding: "Hex"
    });
    await operation.challenge(challenge);

    var { response } = await challenge.selectSecurePassword("securePassword");
    assert_equals(typeof response, 'string');

    var { response } = await challenge.exchangePublicKeys("0A0B0C05");
    assert_equals(typeof response, 'string');
    assert_is_hex(response);

    var { response } = await challenge.computeClientProof("0A0B0C05");
    assert_equals(typeof response, 'string');
    assert_is_hex(response)

    var { response } = await challenge.verifyServerProof("0A0B0C05");
    assert_equals(typeof response, 'undefined');

    await challenge.seal();
    await channel.close();
}, "BerytusSecureRemotePasswordChallenge (Hex encodig) messaging");

// TODO(berytus): Test sending ArrayBufferViews instead
// of ArrayBuffer

// TODO(berytus); Test sending BerytusEncryptedPacket

// TODO(berytus): Test sending messages using send()