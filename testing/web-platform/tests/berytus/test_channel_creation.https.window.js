/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

setup(() => {
    // Making sure we are in a secure context, as expected.
    assert_true(window.isSecureContext);
});


test(() => {
    assert_true("BerytusChannel" in window);
}, "Ensure BerytusChannel is available");