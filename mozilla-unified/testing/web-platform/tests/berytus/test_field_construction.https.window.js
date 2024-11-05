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

function clone(obj) {
    return JSON.parse(JSON.stringify(obj));
}

function createPacket() {
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
    return packet;
}

test(() => {
    const id = "userId";
    const options = {
        private: true,
        humanReadable: true,
        maxLength: 16,
        allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
    };
    const value = "bob123";
    const field = new BerytusIdentityField(
        id,
        options,
        value
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_equals(field.value, value);
    const oldOptions = clone(options);
    options.private = false;
    options.allowedCharacters = "def";
    options.maxLength = 12;
    options.humanReadable = false;
    assert_deep_equals(field.options, oldOptions);
}, "BerytusIdentityField correctly stores properties");

test(() => {
    const id = "userId";
    const options = {
        private: false,
        humanReadable: true,
        maxLength: 4,
        allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
    };
    const value = "bob123";
    assert_throws_js(TypeError, () => {
        new BerytusIdentityField(
            id,
            options,
            value
        );
    });
}, "BerytusIdentityField rejects invalid value (maxLength)");

test(() => {
    const id = "userId";
    const options = {
        private: false,
        humanReadable: true,
        maxLength: 16,
        allowedCharacters: "abcdefghijklmnopqrstuvwxyz123456789-_"
    };
    const value = "bob123!";
    assert_throws_js(TypeError, () => {
        new BerytusIdentityField(
            id,
            options,
            value
        );
    });
}, "BerytusIdentityField rejects invalid value (allowedCharacters)");

test(() => {
    const id = "userId";
    const options = {
        private: true,
        kind: "EmailAddress"
    };
    const value = "bob123@example.com";
    const field = new BerytusForeignIdentityField(
        id,
        options,
        value
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_equals(field.value, value);
    const oldOptions = clone(options);
    options.private = false;
    options.kind = "PhoneNumber";
    assert_deep_equals(field.options, oldOptions);
}, "BerytusForeignIdentityField correctly stores properties");


test(() => {
    const id = "password";
    const options = {
        passwordRules: "minlength: 8; maxlength: 16;"
    };
    const value = "poiuypoiuypo";
    const field = new BerytusPasswordField(
        id,
        options,
        value
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_equals(field.value, value);
    const oldOptions = clone(options);
    options.passwordRules = "maxlength: 32;";
    assert_deep_equals(field.options, oldOptions);
}, "BerytusPasswordField correctly stores properties");

test(() => {
    const id = "password";
    const options = {
        passwordRules: "minlength: 8; maxlength: 16;"
    };
    const value = "poiuy";
    assert_throws_js(TypeError, () => {
        new BerytusPasswordField(
            id,
            options,
            value
        );
    });
}, "BerytusPasswordField rejects invalid value (passwordRules)");

test(() => {
    const id = "securePassword";
    const options = {
        identityFieldId: "userId"
    };
    const field = new BerytusSecurePasswordField(
        id,
        options
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_equals(field.value, null);
    const oldOptions = clone(options);
    options.identityFieldId = "email";
    assert_deep_equals(field.options, oldOptions);
}, "BerytusSecurePassword correctly stores properties");


test(() => {
    const id = "accountKey";
    const options = {
        alg: -51
    };
    const field = new BerytusKeyField(
        id,
        options
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_equals(field.value, null);
    assert_deep_equals(field.toJSON(), { id, options, type: "Key", value: null });
    const oldOptions = clone(options);
    options.alg = 0;
    assert_deep_equals(field.options, oldOptions);
}, "BerytusKeyField correctly stores properties");

test(() => {
    const id = "accountKey";
    const options = {
        alg: -51
    };
    const privateKey = new Uint8Array([1, 2, 3, 4]);
    const value = new BerytusSharedKeyFieldValue(
        privateKey
    );
    const field = new BerytusSharedKeyField(
        id,
        options,
        value
    );
    assert_equals(field.id, id);
    assert_not_equals(field.options, options);
    assert_deep_equals(field.options, options);
    assert_deep_equals(field.value.privateKey, value.privateKey);
    assert_deep_equals(
        field.toJSON(),
        {
            id,
            options,
            type: "SharedKey",
            value: "AQIDBA"
            //value: ChromeUtils.base64URLEncode(value.privateKey, { pad: false })
        }
    );
    const oldOptions = clone(options);
    options.alg = 0;
    assert_deep_equals(field.options, oldOptions);
}, "BerytusSharedKeyField correctly stores properties");

test(() => {
    const packet = createPacket();
    const field = new BerytusIdentityField(
        "userId",
        { maxLength: 8, humanReadable: false, private: true },
        packet
    );
    assert_equals(field.value, packet);
}, "BerytusIdentityField accepts BerytusEncryptedPakcet");

test(() => {
    const packet = createPacket();
    const field = new BerytusForeignIdentityField(
        "mobile",
        { kind: "PhoneNumber", private: false },
        packet
    );
    assert_equals(field.value, packet);
}, "BerytusForeignIdentityField accepts BerytusEncryptedPakcet");

test(() => {
    const packet = createPacket();
    const field = new BerytusPasswordField(
        "mobile",
        { passwordRules: "minlength: 8;" },
        packet
    );
    assert_equals(field.value, packet);
}, "BerytusPasswordFieldValue accepts BerytusEncryptedPakcet");

test(() => {
    const packet = createPacket();
    const value = new BerytusSharedKeyFieldValue(
        packet
    );
    assert_equals(value.privateKey, packet);
}, "BerytusSharedKeyFieldValue accepts BerytusEncryptedPakcet");
