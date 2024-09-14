/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

interface XPCOMUtils {
    defineLazyGetter<O, K extends string, F extends () => any>(
        output: O,
        outputKey: K,
        Fn: F
    ): asserts output is O & { [_ in K]: ReturnType<F> }
    defineLazyPreferenceGetter<O, K extends string>(
        output: O,
        outputKey: K,
        pref: string,
        defaultValue: unknown
    ): asserts output is O & Record<K, unknown>;
}

declare let XPCOMUtils: XPCOMUtils;
export { XPCOMUtils };