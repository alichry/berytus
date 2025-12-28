/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { fetchEntries } from "../signals/fetch-entries";
import { useStore } from '@nanostores/react';
import FetchEntry from "./FetchEntry";

export default function FetchEntriesTable() {
    const $fetchEntries = useStore(fetchEntries);
    return (
        <div className="flex flex-col">
            {$fetchEntries.map((e, i) => (
                <FetchEntry key={i} entry={e} />
            ))}
        </div>
    );
}