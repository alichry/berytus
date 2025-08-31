/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import { Root, InternalCommand } from '@alichry/jsconsole';

const commands = [
    new InternalCommand({
        id: 'signingKey:list',
        description: ''
    }, async ({ context: { app }}) => {
        await app.onRun(`window.$signingKeys`);
    }),
    new InternalCommand({
        id: 'generateKey:x25519',
        description: '[returnVariable="x25519KeyPair"]'
    }, async ({ args: [returnVariable="x25519KeyPair"], context: { app } }) => {
        await app.onRun(`var ${returnVariable} = await crypto.subtle.generateKey(
    "X25519",
    true,
    ["deriveKey"]
);`);
    }),
    new InternalCommand({
        id: 'importKey:spki:x25519',
        description: '[spkiData] [keyVariable="x25519PubKey"]'
    }, async function ({ args: [spkiData, keyVariable="x25519PubKey"], context: { app }}) {
        await app.onRun(`\
var ${keyVariable} = await crypto.subtle.importKey(
    'spki',
    ${spkiData},
    'X25519',
    true,
    []
);`);
    }),
    new InternalCommand({
        id: 'importKey:pkcs8:x25519',
        description: '[pkcs8Data] [keyVariable="x25519PrivKey"]'
    }, async function ({ args: [pkcs8Data, keyVariable="x25519PrivKey"], context: { app }}) {
        await app.onRun(`\
var ${keyVariable} = await crypto.subtle.importKey(
    'pkcs8',
    ${pkcs8Data},
    'X25519',
    true,
    ['deriveKey']
);`);
    }),
    new InternalCommand({
        id: 'exportKey:pkcs8:x25519',
        description: '[keyVariable="x25519KeyPair.privateKey"] [returnVariable="x25519KeyPkcs"]'
    }, async ({ args: [keyVariable="x25519KeyPair.privateKey", returnVariable="x25519KeyPkcs"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.exportKey(
    'pkcs8',
    ${keyVariable}
);`);
    }),
    new InternalCommand({
        id: 'exportKey:spki:x25519',
        description: '[keyVariable="x25519KeyPair.publicKey"] [returnVariable="x25519KeySpki"]'
    }, async ({ args: [keyVariable="x25519KeyPair.publicKey", returnVariable="x25519KeySpki"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.exportKey(
    'spki',
    ${keyVariable}
);`);
    }),
    new InternalCommand({
        id: 'importKey:pkcs8:ed25519',
        description: '[pkcs8Data] [returnVariable="signingKeyPriv"]'
    }, async ({ args: [pkcs8Data, returnVariable = "signingKeyPriv"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.importKey(
    'pkcs8',
    ${pkcs8Data},
    "Ed25519",
    true,
    ['sign']
);
`);
    }),
    new InternalCommand({
        id: 'importKey:spki:ed25519',
        description: '[spkiData] [returnVariable="signingKeyPub"]'
    }, async ({ args: [spkiData, returnVariable = "signingKeyPub"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.importKey(
    'spki',
    ${spkiData},
    "Ed25519",
    true,
    ['verify']
);
`);
    }),
    new InternalCommand({
        id: 'exportKey:spki:ed25519',
        description: '[keyVariable="signingKeyPub"] [returnVariable="signingKeySpki"]'
    }, async ({ args: [keyVariable="signingKeyPub", returnVariable="signingKeySpki"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.exportKey(
    'spki',
    ${keyVariable}
);
`);
    }),
    new InternalCommand({
        id: 'sign:ed25519',
        description: '[data] [keyVariable="signingKeyPriv"] [returnVariable="signed"]'
    }, async ({ args: [data, keyVariable = 'signingKeyPriv', returnVariable = 'signed'], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.sign(
    "Ed25519",
    ${keyVariable},
    ${data}
);`);
    }),
    new InternalCommand({
        id: 'verify:ed25519',
        description: '[signature] [message] [keyVariable="signingKeyPub"] [returnVariable="sigValid"]'
    }, async ({ args: [signature, message, keyVariable = 'signingKeyPub', returnVariable = 'sigValid'], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.verify(
    "Ed25519",
    ${keyVariable},
    ${signature},
    ${message}
);`);
    }),
    new InternalCommand({
        id: 'deriveKey:x25519:hkdf:aes-gcm',
        description: '[selfX25519PrivKey] [peerX25519PubKey] [returnVariable="agreedKey"]'
    }, async function ({ args: [selfX25519PrivKey, peerX25519PubKey, returnVariable = "agreedKey"], context: { app } }) {
        await app.onRun(`\
var sharedKey = await crypto.subtle.deriveKey(
    {
        name: "X25519",
        public: ${peerX25519PubKey}
    },
    ${selfX25519PrivKey},
    'HKDF',
    false,
    ['deriveKey']
);
var ${returnVariable} = await crypto.subtle.deriveKey(
    channel.keyAgreementParams.derivation,
    sharedKey,
    channel.keyAgreementParams.generation,
    true,
    ['encrypt', 'decrypt']
);`);
    }),
    new InternalCommand({
        id: 'encrypt:aes-gcm',
        description: `[key] [data] [tagLen=128] [returnVariable="encrypted"] [ivVariable="iv"]`
    }, async function ({ args: [key, data, tagLen=128, returnVariable="encrypted", ivVariable="iv"], context: { app }}) {
        await app.onRun(`\
var ${ivVariable} = new Uint8Array(12);
await crypto.getRandomValues(${ivVariable});
var ${returnVariable} = await crypto.subtle.encrypt(
    {
        name: "AES-GCM",
        iv: ${ivVariable},
        tagLength: ${tagLen}
    },
    ${key},
    ${data}
);`);
    }),
    new InternalCommand({
        id: 'encrypt:aes-gcm+auth',
        description: `[key] [data] [addData] [tagLen=128] [returnVariable="encrypted"] [ivVariable="iv"]`
    }, async function ({ args: [key, data, addData, tagLen=128, returnVariable="encrypted", ivVariable="iv"], context: { app }}) {
        await app.onRun(`\
var ${ivVariable} = new Uint8Array(12);
await crypto.getRandomValues(${ivVariable});
var ${returnVariable} = await crypto.subtle.encrypt(
    {
        name: "AES-GCM",
        iv: ${ivVariable},
        tagLength: ${tagLen},
        additionalData: ${addData}
    },
    ${key},
    ${data}
);`);
    }),
    new InternalCommand({
        id: 'decrypt:aes-gcm',
        description: `[key] [iv] [data] [tagLen=128] [returnVariable="decrypted"]`
    }, async function ({ args: [key, iv, data, tagLen=128, returnVariable="decrypted"], context: { app }}) {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.encrypt(
    {
        name: "AES-GCM",
        iv: ${iv},
        tagLength: ${tagLen}
    },
    ${key},
    ${data}
);`);
    }),
    new InternalCommand({
        id: 'decrypt:aes-gcm+auth',
        description: `[key] [iv] [data] [addData] [tagLen=128] [returnVariable="decrypted"]`
    }, async function ({ args: [key, iv, data, addData, tagLen=128, returnVariable="decrypted"], context: { app }}) {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.decrypt(
    {
        name: "AES-GCM",
        iv: ${iv},
        tagLength: ${tagLen},
        additionalData: ${addData}
    },
    ${key},
    ${data}
);`);
    }),
    new InternalCommand({
        id: 'channel:create',
        description: `[signingKeyEntry]`,
    }, async function ({ args: [signingKeyEntry], context: { app } }) {
        let code;
        signingKeyEntry = Number.parseInt(signingKeyEntry, 10);
        if (! Number.isNaN(signingKeyEntry)) {
            signingKeyEntry--;
            // @ts-ignore
            const key = window.$signingKeys[signingKeyEntry];
            if (! key) {
                await app.onRun('new Error("Signing key entry not found")');
                return;
            }
            await app.onRun(`:importKey:pkcs8:ed25519 Uint8Array.fromBase64("${key.private}") webAppEd25519Priv`);
            await app.onRun(`:importKey:spki:ed25519 Uint8Array.fromBase64("${key.public}") webAppEd25519Pub`);
            await app.onRun(`:exportKey:spki:ed25519 webAppEd25519Pub webAppEd25119Spki`);
            code =`\
var channel = await BerytusChannel.create({
    webApp: new BerytusCryptoWebAppActor(
        new Uint8Array(webAppEd25119Spki).toBase64()
    )
});`;
        } else {
            code = `\
var channel = await BerytusChannel.create({
  webApp: new BerytusAnonymousWebAppActor()
});`;
        }
        await app.onRun(code);
    }),
    new InternalCommand({
        id: 'channel:e2e',
        description: ''
    }, async function ({ args, context: { app } }) {
        await app.onRun(`:generateKey:x25519 webAppX25519KeyPair`);
        await app.onRun(`:exportKey:spki:x25519 webAppX25519KeyPair.publicKey webAppX25519KeySpki`);
        await app.onRun(`\
var params = await channel.prepareKeyAgreementParameters({
    public: new Uint8Array(webAppX25519KeySpki).toBase64(),
    unmaskAllowlist: ["https://localhost/*"]
});
var message = new TextEncoder().encode(params.toCanonicalJSON())`);
        await app.onRun(`\
:sign:ed25519 message webAppEd25519Priv webAppSignedParams`);
        await app.onRun(`\
var scmSignature = await channel.exchangeKeyAgreementSignatures(
    webAppSignedParams
);`);
        await app.onRun(`:importKey:spki:ed25519 Uint8Array.fromBase64(params.authentication.public.scm) scmEd25519Pub`);
        await app.onRun(`:verify:ed25519 scmSignature message scmEd25519Pub scmSigValid`);
        // @ts-ignore
        if (! window.scmSigValid) {
            console.error(new Error("SCM Signature is invalid; key agreement failed!"));
            return;
        }
        await app.onRun(`\
await channel.enableEndToEndEncryption();`);
        await app.onRun(`:importKey:spki:x25519 Uint8Array.fromBase64(channel.keyAgreementParams.exchange.public.scm) scmX25519Key`);
        await app.onRun(`:deriveKey:x25519:hkdf:aes-gcm webAppX25519KeyPair.privateKey scmX25519Key e2eeKey`);
    }),
    new InternalCommand({
        id: 'channel:e2e:encrypt:raw',
        description: "<data>"
    }, async function ({ args, context: { app }}) {
        await app.onRun(`var toEncrypt = ${args.join(' ')}`);
        await app.onRun(`:encrypt:aes-gcm+auth e2eeKey toEncrypt channel.keyAgreementParams.session.fingerprint.value`);
    }),
    new InternalCommand({
        id: 'channel:e2e:decrypt:raw',
        description: "[iv] [data]"
    }, async function ({ args: [iv, data], context: { app }}) {
        await app.onRun(`:decrypt:aes-gcm+auth e2eeKey ${iv} ${data} channel.keyAgreementParams.session.fingerprint.value`);
    }),
    new InternalCommand({
        id: 'channel:e2e:encrypt:jwe',
        description: '[data] [key] [returnVariable="jwe"]'
    }, async function ({ args: [data, key, returnVariable="jwe"], context: { app }}) {
        // e2eeKey
        await app.onRun(`\
var content = undefined;
{
    if (typeof ${data} === "string") {
        const dataURL = window.parseDataURL(${data});
        if (dataURL) {
            const charset = dataURL.mimeType.parameters.get("charset");
            if (charset && charset !== "utf-8") {
                throw new Error("Refusing to encrypt data URL with a charset other than utf-8");
            }
            content = {
                mimeType: dataURL.mimeType.toString(),
                plaintext: dataURL.body
            };
        } else {
            content = {
                plaintext: new TextEncoder().encode(${data}),
                mimeType: "text/plain;charset=utf-8"
            };
        }
    } else if (
        ${data} instanceof ArrayBuffer ||
        ${data} instanceof SharedArrayBuffer
    ) {
        content = {
            plaintext: new Uint8Array(${data}),
            mimeType: "application/octet-stream"
        };
    } else if (ArrayBuffer.isView(${data})) {
        content = {
            plaintext: new Uint8Array(${data}.buffer),
            mimeType: "application/octet-stream"
        };
    }
}
if (! content) {
    throw new Error(\`Cannot encrypt unsupported data of '\${typeof ${data}}' type.\`);
}
var jweOp = new window.jose.CompactEncrypt(content.plaintext);
var ${returnVariable} = await jweOp.setProtectedHeader({
    alg: 'dir',
    enc: 'A256GCM',
    cty: content.mimeType,
    typ: "JOSE"
}).encrypt(e2eeKey)`);
    }),
    new InternalCommand({
        id: 'channel:e2e:decrypt:jwe',
        description: '[jwe] [returnVariable="decrypted"]'
    }, async function ({ args: [jwe, returnVariable="decrypted"], context: { app }}) {
        await app.onRun(`\
var ${returnVariable} = await window.jose.compactDecrypt(${jwe}, e2eeKey)`);
    }),
    new InternalCommand({
        id: 'channel:e2e:unmask',
        description: '[body] [returnVariable="response"]'
    }, async function ({ args: [body, returnVariable="resp"], context: { app }}) {
        await app.onRun(`\
var ${returnVariable} = await fetch('/unmask', { method: "POST", body: ${body} });
`);
    }),
    new InternalCommand({
        id: 'channel:e2e:unmask+decrypt',
        description: '[body] [returnVariable="response"]'
    }, async function ({ args: [body, returnVariable="plaintext"], context: { app }}) {
        await app.onRun(`:channel:e2e:unmask ${body} unmaskedResp`);
        await app.onRun(`var unmaskedJwe = await unmaskedResp.text()`);
        await app.onRun(`:channel:e2e:decrypt:jwe unmaskedJwe decrypted`);
        await app.onRun(`var ${returnVariable} = decrypted.plaintext`);
        console.log(window["decrypted"]);
    }),
    new InternalCommand({
        id: 'channel:e2e:encrypt+mask',
        description: '[data] [returnVariable="jwePacket"]'
    }, async function ({ args: [data, returnVariable="plaintext"], context: { app }}) {
        console.log(`TODO(berytus): Once we implement masking of encrypted packets coming from web app.`);
    }),
];
export interface Props {
    className?: string;
}

export default function JSConsole({ className }: Props) {
    return <Root
        environment="top-level"
        defaultTheme="dark"
        extraCommands={commands}
        className={className}
    />
}