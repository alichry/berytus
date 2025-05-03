# Lab: Experiment with Web App Keys

To facilitate experimentation with Berytus X509 Certificate Extensions/Web App Signing Keys, we provide a well equipped lab. The lab provisions
a platform that loads an X509 certificate, hosts an HTTPS server, and renders an interactive JavaScript console.

## Usage

### Certificate generation

The lab comes with a default X509 certificate that includes the Berytus extension. If you wish to generate a new one, invoke the `cert` tool provided
in the top-level directory of this repository. For example:

```sh
./cert -a localhost -a 127.0.0.1 -k https://localhost/app/a/* -k https://localhost/app/b/* lab/server
```

### Entering the lab

(1) Install the lab's dependencies if you have not already:
```sh
npm install
```
(2) Run the server and its privileged proxy
```sh
npm run start
```
(3) Using our extended Firefox browser,
navigate to the lab using the endpoint provided by
the above command (default is `https://localhost`)

### Assuming a web application
The server renders the lab environment on any
requested path. For example, to assume a logical
web application served under `/app/a/*`, navigate to `/app/a`.

#### Constructing a crypto web app actor
Now, in the interactive console, construct a crypt web app actor.
To create such actor backed by a specific Ed25519 public key,
simply pass a base64 string of the DER-encoded SPKI, e.g.:
```js
var actor = new BerytusCryptoWebAppActor(
    "MCowBQYDK2VwAyEAoIHnUKGRWjtc8yIBBTmWAoQ2S/j/Lg2WxhY717m16nA="
);
```

### Creating a channel
Pass the `actor` object to `BerytusChannel.create()` as shown below.
```js
var channel = await BerytusChannel.create({ webApp: actor });
```
The Berytus implementation will parse the certificate extension
and validate whether the passed key is included and allowed
to be used in the served HTTP resource; e.g., see the below figures.

<img width="1440" alt="BerytusNotAllowed" src="https://github.com/user-attachments/assets/03840304-9763-47fd-8d38-a40ac821934f" />

**Fig.**
Rejection of an assumed crypto web app actor.
The provided key is defined in the first entry
of the allowlist; however, the web page's URL
does not match that of the entry.

<img width="1440" alt="BerytusAllowed" src="https://github.com/user-attachments/assets/cb22079c-76ad-4a01-9fba-d63314b5fe08" />

**Fig.**
Acceptance of an assumed crypto web app actor.
The provided key is defined in the first entry
of the allowlist and the web page's URL matches
that of the entry.
