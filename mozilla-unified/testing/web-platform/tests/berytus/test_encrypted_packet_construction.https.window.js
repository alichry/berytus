/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});

test(() => {
    assert_true(
        new BerytusJWEPacket(
            "eyJhbGciOiJkaXIiLCJlbmMiOiJBMjU2R0NNIiwiY3R5IjoidGV4dC9wbGFpbjtjaGFyc2V0PXV0Zi04IiwidHlwIjoiSk9TRSJ9..NIS9k6ZROLy_mgjL.Vl5ufGUlvxXcmoIAN-UaxIfhZQ.NTMhhZ23xUslkP_NIac83g"
        )
        instanceof
        BerytusEncryptedPacket
    );
}, "BerytusJWEPacket is instanceof BerytusEncryptedPacket");

promise_test(async () => {
    const content = "eyJhbGciOiJkaXIiLCJlbmMiOiJBMjU2R0NNIiwiY3R5IjoidGV4dC9wbGFpbjtjaGFyc2V0PXV0Zi04IiwidHlwIjoiSk9TRSJ9..NIS9k6ZROLy_mgjL.Vl5ufGUlvxXcmoIAN-UaxIfhZQ.NTMhhZ23xUslkP_NIac83g";
    const packet = new BerytusJWEPacket(content);
    assert_equals(await packet.text(), content);
}, "BerytusJWEPacket is exposed when constructed")