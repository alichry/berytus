#!/bin/bash
set -e
cd "$(cygpath -u "$MOZ_DIR")/testing/web-platform/tests"

echo 'Current CA cert validity:'
openssl x509 -in tools/certs/cacert.pem -noout -dates
echo 'Current Server cert validity:'
openssl x509 -in tools/certs/web-platform.test.pem -noout -dates

CA_SAN=$(openssl x509 -in tools/certs/cacert.pem -noout -ext subjectAltName | tail -1 | sed 's/^[[:space:]]*//')
CA_NC=$(openssl x509 -in tools/certs/cacert.pem -noout -ext nameConstraints | tail +3 | sed 's/^[[:space:]]*//' | awk '{printf "permitted;%s\n", $0}')

openssl req -x509 -newkey rsa:2048 \
    -keyout tools/certs/cacert.key -out tools/certs/cacert.pem \
    -days 365000 -nodes -subj '/CN=web-platform-tests' \
    -addext 'basicConstraints = critical, CA:TRUE' \
    -addext 'keyUsage = keyCertSign' \
    -addext 'extendedKeyUsage = serverAuth' \
    -addext 'authorityKeyIdentifier = keyid:always, issuer:always' \
    -addext "subjectAltName = $CA_SAN" \
    -addext "nameConstraints = $(echo "$CA_NC" | paste -sd',' -)"

openssl req -newkey rsa:2048 \
    -keyout tools/certs/web-platform.test.key \
    -out /tmp/wpt.csr \
    -nodes -subj '/CN=web-platform.test' \
    -addext 'basicConstraints=CA:FALSE' \
    -addext 'keyUsage=digitalSignature,nonRepudiation,keyEncipherment' \
    -addext 'extendedKeyUsage=serverAuth' \
    -addext "subjectAltName=$CA_SAN"

openssl x509 -req -in /tmp/wpt.csr \
    -CA tools/certs/cacert.pem -CAkey tools/certs/cacert.key \
    -CAcreateserial -out tools/certs/web-platform.test.pem \
    -days 365000 \
    -copy_extensions copy

rm -f /tmp/wpt.csr
echo 'New CA cert validity:'
openssl x509 -in tools/certs/cacert.pem -noout -dates
echo 'New Server cert validity:'
openssl x509 -in tools/certs/web-platform.test.pem -noout -dates
