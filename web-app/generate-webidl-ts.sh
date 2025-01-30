#!/bin/sh

set -e
file="./src/generated/berytus.idl"
dtsFile="./src/generated/berytus.d.ts"
# cat ../mozilla-unified/dom/webidl/Berytus* > "$file"
cat ../mozilla-unified/dom/webidl/BerytusAccount.webidl ../mozilla-unified/dom/webidl/BerytusAccountAuthenticationOperation.webidl ../mozilla-unified/dom/webidl/BerytusAccountCreationOperation.webidl ../mozilla-unified/dom/webidl/BerytusAnonymousWebAppActor.webidl ../mozilla-unified/dom/webidl/BerytusChallengeMap.webidl ../mozilla-unified/dom/webidl/BerytusChallengeParameters.webidl ../mozilla-unified/dom/webidl/BerytusChannel.webidl ../mozilla-unified/dom/webidl/BerytusCryptoWebAppActor.webidl ../mozilla-unified/dom/webidl/BerytusEncryptedPacket.webidl ../mozilla-unified/dom/webidl/BerytusField.webidl ../mozilla-unified/dom/webidl/BerytusFieldMap.webidl ../mozilla-unified/dom/webidl/BerytusFieldOptions.webidl ../mozilla-unified/dom/webidl/BerytusFieldValueDictionary.webidl ../mozilla-unified/dom/webidl/BerytusForeignIdentityField.webidl ../mozilla-unified/dom/webidl/BerytusIdentityField.webidl ../mozilla-unified/dom/webidl/BerytusKeyAgreementParameters.webidl ../mozilla-unified/dom/webidl/BerytusKeyField.webidl ../mozilla-unified/dom/webidl/BerytusLoginOperation.webidl ../mozilla-unified/dom/webidl/BerytusPasswordField.webidl ../mozilla-unified/dom/webidl/BerytusSecretManagerActor.webidl ../mozilla-unified/dom/webidl/BerytusSecurePasswordField.webidl ../mozilla-unified/dom/webidl/BerytusSharedKeyField.webidl ../mozilla-unified/dom/webidl/BerytusUserAttribute.webidl ../mozilla-unified/dom/webidl/BerytusUserAttributeMap.webidl ../mozilla-unified/dom/webidl/BerytusWebAppActor.webidl ../mozilla-unified/dom/webidl/BerytusChallenge.webidl \
    > "$file"

### BRTTODO: open an issue for webidl-dts-gen to report that files with an *.webidl extension
### are not being silently ignored.

npm exec webidl-dts-gen -- -i "$file" -o "$dtsFile"
echo "" >> "$dtsFile"

# Additional fixes for further typescript support:

# 1. hardcode auth/register operation intent
sed -i '' -E 's/^(interface BerytusAccountCreationOperation extends BerytusLoginOperation .*)$/\1\n\treadonly intent: "Register";/g' "$dtsFile"
sed -i '' -E 's/^(interface BerytusAccountAuthenticationOperation extends BerytusLoginOperation .*)$/\1\n\treadonly intent: "Authenticate";/g' "$dtsFile"

# 2. Add declare var: interface
sed -E -n 's/^interface[[:space:]]([a-zA-Z0-9]+)[[:space:]].*$/\1/p' "$file" |
    grep -v 'mixin' |
    sed -E 's/(.*)/declare var \1: \1;/' >> "$dtsFile"

# 3. Fix `addFields` and `rejectAndReviseFields` as they expect 1 parameter
# instead of ...params:
sed -i '' -E 's/([[:space:]]*addFields\()([a-zA-Z0-9]+:[[:space:]]*)([a-zA-Z0-9]+)(\).*)/\1...\2Array<\3>\4/g' \
    "$dtsFile"
sed -i '' -E 's/([[:space:]]*rejectAndReviseFields\()([a-zA-Z0-9]+:[[:space:]]*)([a-zA-Z0-9]+)(\).*)/\1...\2Array<\3>\4/g' \
    "$dtsFile"
# addFields(...field: Array<BerytusField>)

sed -i '' -E 's/(.*)/\t\1/g' "$dtsFile"
sed -i '' '1i\
export {};\
declare global {
' "$dtsFile"
echo "}" >> "$dtsFile"

