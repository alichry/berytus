#!/bin/sh

set -e
file="./src/generated/berytus.idl"
dtsFile="./src/generated/berytus.d.ts"
# cat ../dom/webidl/Berytus* > "$file"
cat ../dom/webidl/BerytusAccount.webidl ../dom/webidl/BerytusAccountAuthenticationOperation.webidl ../dom/webidl/BerytusAccountCreationOperation.webidl ../dom/webidl/BerytusAnonymousWebAppActor.webidl ../dom/webidl/BerytusChallengeMap.webidl ../dom/webidl/BerytusChallengeParameters.webidl ../dom/webidl/BerytusChannel.webidl ../dom/webidl/BerytusCryptoWebAppActor.webidl ../dom/webidl/BerytusEncryptedPacket.webidl ../dom/webidl/BerytusField.webidl ../dom/webidl/BerytusFieldMap.webidl ../dom/webidl/BerytusFieldOptions.webidl ../dom/webidl/BerytusFieldValueDictionary.webidl ../dom/webidl/BerytusForeignIdentityField.webidl ../dom/webidl/BerytusIdentityField.webidl ../dom/webidl/BerytusKeyAgreementParameters.webidl ../dom/webidl/BerytusKeyField.webidl ../dom/webidl/BerytusLoginOperation.webidl ../dom/webidl/BerytusPasswordField.webidl ../dom/webidl/BerytusSecretManagerActor.webidl ../dom/webidl/BerytusSecurePasswordField.webidl ../dom/webidl/BerytusSharedKeyField.webidl ../dom/webidl/BerytusUserAttribute.webidl ../dom/webidl/BerytusUserAttributeMap.webidl ../dom/webidl/BerytusWebAppActor.webidl ../dom/webidl/AuthRealmChallenge.webidl \
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

