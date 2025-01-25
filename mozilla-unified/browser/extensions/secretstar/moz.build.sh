#!/bin/sh

set -e

DIRS="build build.worker public"

MOZBUILD="\
# -*- Mode: python; indent-tabs-mode: nil; tab-width: 40 -*-
# vim: set filetype=python:
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

DEFINES[\"MOZ_APP_VERSION\"] = CONFIG[\"MOZ_APP_VERSION\"]
DEFINES[\"MOZ_APP_MAXVERSION\"] = CONFIG[\"MOZ_APP_MAXVERSION\"]


JAR_MANIFESTS += [\"jar.mn\"]

with Files(\"**\"):
    BUG_COMPONENT = (\"BuiltInAddons\", \"Secret*\")

FINAL_TARGET_FILES.features[\"secretstar@alichry\"] += [
    \"manifest.json\"
]
"

listEntries() {
    local dir
    local entries
    dir="$1"
    if [ -z "$dir" ]; then
        echo "Missing dir" >&2
        return 1
    fi
    echo "FINAL_TARGET_FILES.features[\"secretstar@alichry\"][\"${dir}\"] += ["
    find "$dir" -type f -maxdepth 1 | sort -f | sed -E 's/^(.*)$/\t"\1",/g;'
    echo "]"
    find "$dir" -type d -maxdepth 1 -not -path "$dir" | while IFS= read subdir; do
        listEntries "$subdir"
    done
}

run() {
    echo "$MOZBUILD"
    for d in $DIRS
    do
        listEntries "$d"
        # find "$d" -type d -maxdepth 1 -not -path "$d" | while IFS= read subdir; do
        #     echo "$subdir"
        # done
    done
}

run > moz.build