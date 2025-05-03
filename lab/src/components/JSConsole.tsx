import { Root, InternalCommand } from '@alichry/jsconsole';

const commands = [
    new InternalCommand({
        id: 'signingKey:list',
        description: ''
    }, async ({ context: { app }}) => {
        await app.onRun(`window.$signingKeys`);
    }),
    new InternalCommand({
        id: 'X25519:create',
        description: '[returnVariable="x25519KeyPair"]'
    }, async ({ args: [returnVariable="x25519KeyPair"], context: { app } }) => {
        await app.onRun(`var ${returnVariable} = await crypto.subtle.generateKey(
    "X25519",
    true,
    ["deriveKey"]
);`);
    }),
    new InternalCommand({
        id: 'X25519:export:pkcs8',
        description: '[keyVariable="x25519KeyPair.privateKey"] [returnVariable="x25519KeyPkcs"]'
    }, async ({ args: [keyVariable="x25519KeyPair.privateKey", returnVariable="x25519KeyPkcs"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.exportKey(
    'pkcs8',
    ${keyVariable}
);`);
    }),
    new InternalCommand({
        id: 'X25519:export:spki',
        description: '[keyVariable="x25519KeyPair.publicKey"] [returnVariable="x25519KeySpki"]'
    }, async ({ args: [keyVariable="x25519KeyPair.publicKey", returnVariable="x25519KeySpki"], context: { app } }) => {
        await app.onRun(`\
var ${returnVariable} = await crypto.subtle.exportKey(
    'spki',
    ${keyVariable}
);`);
    }),
    new InternalCommand({
        id: 'Ed25519:import:pkcs8',
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
        id: 'Ed25519:import:spki',
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
        id: 'Ed25519:export:spki',
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
        id: 'Ed25519:sign',
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
        id: 'Ed25519:verify',
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
            await app.onRun(`:Ed25519:import:pkcs8 Uint8Array.fromBase64("${key.private}") webAppEd25519Priv`);
            await app.onRun(`:Ed25519:import:spki Uint8Array.fromBase64("${key.public}") webAppEd25519Pub`);
            await app.onRun(`:Ed25519:export:spki webAppEd25519Pub webAppEd25119Spki`);
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
        await app.onRun(`:X25519:create webAppX25519KeyPair`);
        await app.onRun(`:X25519:export:spki webAppX25519KeyPair.publicKey webAppX25519KeySpki`);
        await app.onRun(`\
var params = await channel.prepareKeyAgreementParameters(
    new Uint8Array(webAppX25519KeySpki).toBase64()
);
var message = new TextEncoder().encode(params.toCanonicalJSON())`);
        await app.onRun(`\
:Ed25519:sign message webAppEd25519Priv webAppSignedParams`);
        await app.onRun(`\
var scmSignature = await channel.exchangeKeyAgreementSignatures(
    webAppSignedParams
);`);
        await app.onRun(`:Ed25519:import:spki Uint8Array.fromBase64(params.authentication.public.scm) scmEd25519Pub`);
        await app.onRun(`:Ed25519:verify scmSignature message scmEd25519Pub scmSigValid`)
        await app.onRun(`\
if (! scmSigValid) {
    throw new Error("SCM Signature is invalid; key agreement failed!");
}
console.info("SCM Signature is valid!");
`);
    })
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