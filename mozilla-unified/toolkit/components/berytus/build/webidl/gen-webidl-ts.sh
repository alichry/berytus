#!/bin/sh
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set -e

SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

file="./src/generated/berytus.web.idl"
dtsFile="./src/generated/berytus.web.d.ts"

cat ../../../dom/webidl/SubtleCrypto.webidl \
    ../../../dom/webidl/BerytusEncryptedPacket.webidl \
    ../../../dom/webidl/BerytusField.webidl \
    ../../../dom/webidl/BerytusFieldOptions.webidl \
    ../../../dom/webidl/BerytusFieldValueDictionary.webidl \
    ../../../dom/webidl/BerytusForeignIdentityField.webidl \
    ../../../dom/webidl/BerytusIdentityField.webidl \
    ../../../dom/webidl/BerytusKeyAgreementParameters.webidl \
    ../../../dom/webidl/BerytusKeyField.webidl \
    ../../../dom/webidl/BerytusPasswordField.webidl \
    ../../../dom/webidl/BerytusSecurePasswordField.webidl \
    ../../../dom/webidl/BerytusSharedKeyField.webidl \
    ../../../dom/webidl/BerytusUserAttribute.webidl \
    > "$file"

### TODO(berytus): open an issue in webidl-dts-gen to report
### that files with an *.webidl extension
### are not being silently ignored.

npm exec webidl-dts-gen -- \
    -i "$file" \
    -o "$dtsFile"
sed -i '' "1s/^/type Base64URLString = string;\n/" "$dtsFile"
sed -i '' "1s/^/type BufferSource = ArrayBuffer | ArrayBufferView;\n/" "$dtsFile"
echo "" >> "$dtsFile"

"${SCRIPT_DIR}/gen-webidl-ts.ts"

# sed -i '' -E 's/^(interface .*)$/export \1/g' "$dtsFile"
# sed -i '' -E 's/^(type .*)$/export \1/g' "$dtsFile"
# sed -i '' -E 's/^(enum .*)$/export \1/g' "$dtsFile"




