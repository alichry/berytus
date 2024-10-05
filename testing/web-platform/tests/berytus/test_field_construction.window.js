/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

test(() => {
    assert_true(true);
}, 'dummy');

/* From sanitizer-query-config.https.html; thanks */
function assert_deep_equals(obj1, obj2) {
    assert_equals(typeof obj1, typeof obj2);
    if (typeof obj1 == "string") {
        assert_equals(obj1, obj2);
    } else if (typeof obj1 == "boolean") {
        assert_true(obj1 == obj2);
    } else if (Array.isArray(obj1)) {
        assert_equals(obj1.length, obj2.length);
        assert_array_equals(obj1.sort(), obj2.sort());
    } else if (typeof obj1 == "object") {
        assert_array_equals(Object.keys(obj1).sort(), Object.keys(obj2).sort());
        for (const k of Object.keys(obj1))
            assert_deep_equals(obj1[k], obj2[k]);
    }
}

function clone(obj) {
    return JSON.parse(JSON.stringify(obj));
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
    assert_true(id === field.id);
    assert_true(options !== field.options);
    assert_deep_equals(options, field.options);
    assert_true(value === field.value);
    const oldOptions = clone(options);
    options.private = false;
    options.allowedCharacters = "def";
    options.maxLength = 12;
    options.humanReadable = false;
    assert_deep_equals(oldOptions, field.options);
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
    assert_true(id === field.id);
    assert_true(options !== field.options);
    assert_deep_equals(options, field.options);
    assert_true(value === field.value);
    const oldOptions = clone(options);
    options.private = false;
    options.kind = "PhoneNumber";
    assert_deep_equals(oldOptions, field.options);
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
    assert_true(id === field.id);
    assert_true(options !== field.options);
    assert_deep_equals(options, field.options);
    assert_true(value === field.value);
    const oldOptions = clone(options);
    options.passwordRules = "maxlength: 32;";
    assert_deep_equals(oldOptions, field.options);
}, "BerytusPasswordField correctly stores properties");

test(() => {
    const id = "password";
    const options = {
        passwordRules: "minlength: 8; maxlength: 16;"
    };
    const value = "poiuy";
    assert_throws_js(TypeError, () => {
        new BerytusIdentityField(
            id,
            options,
            value
        );
    });
}, "BerytusPasswordField rejects invalud value (passwordRules)");

