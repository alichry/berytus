# Overview

This directory contains a full stack web application that interacts
with the Berytus Web API for secure user authentication. It
streamlines end-to-end encrypted authentication flows, guaranteeing
protection against credential theft via TLS-proxy-in-the-middle (TPitM) and JavaScript code injection attacks.

### Setup Instructions

First, run `npm install` to gather the dependencies. Node.js 25+
is recommended, however, we have a polyfill if needed for
earlier versions.

Second, to build the project, run `npm run build:node`. This will produce the static files for the frontend.

Third, amend `.env.local` to set the environment variables
according to your preferences and database credentials.
A postgres db is required. You may leave the signing key-
and cert-related environment variables intact.

Fourth, populate your shell with those environment variables
by running `set -o allexport; . .env.local`.

Fifth, run `flyway migrate` as a one-off to setup the database.
This requires the flyway cli to be installed.

Last, run `npm run start` to spin the web application server.
The server will listen for HTTP connections on the port
set by the environment variable `LISTEN_PORT`.

Note, For the pre-configured signing key setup to function by default, the web application should be served on port 443.
To achieve this, either run `npm run dev` which will use
`sudo` along with `socat` to proxy requests on port 443
(set by `PROXY_PORT`) to port 2443 (set by `LISTEN_PORT`).
Alternatively, set `LISTEN_PORT` to 443, and execute
`npm run start` as root.

### Generating a Signing Key

This project comes with a pre-generated Signing Key
for secure credential mapping and key exchange authentication.
Furthermore, it also includes a self-signed certificate
with the Berytus x509 v3 extension.

The below can be used to generate an Ed25519 Signing Key
and a self-signed certificate with the allowlist attached.
Consult the tool's help instruction (`../cert --help`) if you wish to specify non-generated signing keys.

```sh
mkdir -p "cert-custom"
../cert \
    -n "Berytus Web App PoC (Local)" \
    -a localhost \
    -a 127.0.0.1 \
    -k https://localhost/* \
    ./cert-custom
```

The Ed25519 SPKI and PKCS8 can be found in `cert-custom/berytus.0.pubkey.pem` and `cert-custom/berytus.0.privkey.pem`, respectively. Inspect the x509 v3 certificate using the below.

```sh
openssl x509 -noout \
    -text -certopt ext_dump \
    -in ./cert-custom/subj.cert.pem
```

Relevant bit is under X509v3 extensions section. For example:

```
        X509v3 extensions:
            X509v3 Basic Constraints: 
                CA:FALSE
            X509v3 Key Usage: 
                Digital Signature, Non Repudiation, Key Encipherment
            X509v3 Subject Alternative Name: 
                DNS:localhost, DNS:127.0.0.1
            1.2.3.4.22.11.23: 
                0000 - 30 81 b0 30 81 ad 0c 3c-4d 43 6f 77 42   0..0...<MCowB
                000d - 51 59 44 4b 32 56 77 41-79 45 41 4e 31   QYDK2VwAyEAN1
                001a - 74 42 64 2f 53 5a 64 49-44 56 38 58 7a   tBd/SZdIDV8Xz
                0027 - 76 34 64 57 79 39 6a 72-4d 51 30 4f 6e   v4dWy9jrMQ0On
                etc.
```