/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { type FetchEntry } from "../signals/fetch-entries";
interface FetchEntryProps {
    entry: FetchEntry;
}
export default function FetchEntry({ entry }: FetchEntryProps) {
    return (
        <details className="border-t border-white/50 px-4 py-2 text-white">
            <summary className="">Fetch Entry ({entry.source})</summary>
            <code className="text-sm whitespace-pre">
                {JSON.stringify(entry, null, 2)}
            </code>
        </details>
    )
}