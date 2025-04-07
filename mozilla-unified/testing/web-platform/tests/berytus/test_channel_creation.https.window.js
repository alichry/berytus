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

promise_test(async () => {
    const actor = new BerytusAnonymousWebAppActor();
    const constraints = {
        secretManagerPublicKey: [],
        enableEndToEndEncryption: true,
        account: {
           identity: { 'username': 'ali' },
           schemaVersion: 1,
           category: "user"
        }
    };
    const channel = await BerytusChannel.create({
        webApp: actor,
        constraints
    });
    assert_equals(channel.webApp, actor);
    assert_not_equals(channel.constraints, constraints);
    assert_deep_equals(channel.constraints, constraints);
    assert_equals(channel.active, true);
    await channel.close();
    assert_equals(channel.active, false);
}, "BerytusChannel correctly stores properties");

promise_test(async (t) => {
    const actor = new BerytusAnonymousWebAppActor();
    const channelProm = BerytusChannel.create({
        webApp: actor
    });
    await promise_rejects_dom(
        t,
        'InvalidStateError',
        Promise.race([
            BerytusChannel.create({
                webApp: actor
            }),
            Promise.resolve()
        ])
    )
    const channel = await channelProm;
    await promise_rejects_dom(
        t,
        'InvalidStateError',
        Promise.race([
            BerytusChannel.create({
                webApp: actor
            }),
            Promise.resolve()
        ])
    )
    await channel.close();
    // create another channel, this should succeed as the other
    // one has been closed.
    await BerytusChannel.create({
        webApp: actor
    });
}, "BerytusChannel rejects creation request when an existing channel is active");