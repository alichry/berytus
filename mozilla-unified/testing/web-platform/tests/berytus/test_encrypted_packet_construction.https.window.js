/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});

/* From sanitizer-query-config.https.html; thanks */
function assert_deep_equals(obj1, obj2) {
    assert_equals(typeof obj1, typeof obj2);
    if (typeof obj1 == "string") {
        assert_equals(obj1, obj2);
    } else if (typeof obj1 == "boolean") {
        assert_true(obj1 == obj2);
    } else if (Array.isArray(obj1)) {
        assert_true(Array.isArray(obj2));
        assert_equals(obj1.length, obj2.length);
        assert_array_equals(obj1.sort(), obj2.sort());
    } else if (typeof obj1 == "object") {
        if (obj1 === null || obj2 === null) {
            assert_equals(obj1, null);
            assert_equals(obj2, null);
            return;
        }
        assert_array_equals(Object.keys(obj1).sort(), Object.keys(obj2).sort());
        for (const k of Object.keys(obj1))
            assert_deep_equals(obj1[k], obj2[k]);
    }
}

test(() => {
    const params = {
        name: "AES-GCM",
        iv: new Uint8Array([1, 2, 3, 4]),
        additionalData: new Uint8Array([5, 6, 7, 8]),
        tagLength: 128
    };
    const ciphertext = new Uint8Array([9, 10, 11, 12]);
    const packet = new BerytusEncryptedPacket(
        params,
        ciphertext
    );
    assert_deep_equals(packet.parameters.name, params.name);
    assert_deep_equals(new Uint8Array(packet.parameters.iv), params.iv);
    assert_deep_equals(new Uint8Array(packet.parameters.additionalData), params.additionalData);
    assert_deep_equals(packet.parameters.tagLength, params.tagLength);
    assert_deep_equals(new Uint8Array(packet.ciphertext), ciphertext);
}, "BerytusEncryptedPacket correctly stores properties");

test(() => {
    const params = {
        name: "AES-GCM",
        iv: new Uint8Array([1, 2, 3, 4]),
        additionalData: new Uint8Array([5, 6, 7, 8]),
        tagLength: 128
    };
    const ciphertext = new Uint8Array([9, 10, 11, 12]);
    const packet = new BerytusEncryptedPacket(
        params,
        ciphertext
    );
    assert_deep_equals(
        packet.toJSON(),
        {
            parameters: {
                ...params,
                iv: "AQIDBA",
                additionalData: "BQYHCA"
            },
            ciphertext: "CQoLDA"
        }
    );
}, "BerytusEncryptedPacket correctly produces JSON");