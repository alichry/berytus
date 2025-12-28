/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { atom } from 'nanostores';

export interface FetchEntry {
    source: string
    error: string | null;
    originUrl?: string;
    originType?: string; /* SW Only */
    request: {
        url: string;
        method: string;
        headers: Record<string, string>;
        body: string;
    };
    response: {
        headers: Record<string, string>;
        body: string;
    };
}

export const fetchEntries = atom<Array<FetchEntry>>([]);