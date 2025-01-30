/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Sadly the behaviour of builtin addons is a bit puzzling.
 * Secret* is loaded as a builtin addon by XPIInstall; however,
 * the browser action does not appear. This builtin addon,
 * ss-loader, would be run at startup and would load Secret*
 * as a temporary addon instead. Such mechanism does add
 * a browser action to the menu panel.
 */

browser.ssLoader.load();