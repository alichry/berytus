/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});

test(() => {
    const actor = new BerytusAnonymousWebAppActor();
}, "BerytusAnonymousWebAppActor can be instantiated");

test(() => {
    const ed25519PubKey = "MCowBQYDK2VwAyEAJevlUdx72BF8mxdwurBJI9WNgRDMaoYfb0VqywaLOJE=";
    const actor = new BerytusCryptoWebAppActor(
        ed25519PubKey
    );
    assert_true(actor.ed25519Key === ed25519PubKey);
}, "BerytusCryptoWebAppActor correctly stores properties");