export function formatBase64AsPem(str: string, publicKey: boolean = true) {
    const type = publicKey ? "PUBLIC" : "PRIVATE"
    let res = `-----BEGIN ${type} KEY-----\n`;

    while(str.length > 0) {
        res += str.substring(0, 64) + '\n';
        str = str.substring(64);
    }

    res = res + `-----END ${type} KEY-----`;

    return res;
}
export function formatSignatureBufToString(signedMessage: ArrayBuffer): string {
    let hex = ab2base64(signedMessage);
    let res = "-----BEGIN CUSTOM SIGNATURE FORMAT -----\n";

    while(hex.length > 0) {
        res += hex.substring(0, 64) + '\n';
        hex = hex.substring(64);
    }

    res += "----END CUSTOM SIGNATURE FORMAT -----";
    return res;
}

export function extractPemBase64Body(pemStr: string, publicKey: boolean = true): string {
    const type = publicKey ? "PUBLIC" : "PRIVATE"
    const header = `-----BEGIN ${type} KEY-----\n`;
    const footer = `-----END ${type} KEY-----`;
    if (! pemStr.startsWith(header)) {
        throw new Error('Key does not start with "'+header+'"')
    }
    if (! pemStr.endsWith(footer)) {
        throw new Error('Key does not end with "'+footer+'"')
    }
    const body = pemStr.substring(header.length, pemStr.length - footer.length);
    const base64 = body.replace(/\r?\n|\r/g, '');
    return base64;
}
// TODO(berytus): Review this impl
export function pemToBuf(str: string, publicKey: boolean = true): ArrayBufferLike {
    const base64 = extractPemBase64Body(str, publicKey);
    return str2ab(atob(base64));
}

export function bufToPem(buffer: ArrayBuffer, publicKey?: boolean) {
    return formatBase64AsPem(ab2base64(buffer), publicKey);
}

export function ab2str(buffer: ArrayBufferLike) {
    return String.fromCharCode.apply(null, new Uint8Array(buffer) as any);
}

export function str2ab(str: string): ArrayBufferLike {
    return new TextEncoder().encode(str).buffer;
}

export function ab2base64(buffer: ArrayBufferLike) {
    return btoa(ab2str(buffer));
}

export async function privateKeyBufToPublicKeyBuf(privateKeyBuf: ArrayBufferLike): Promise<ArrayBuffer> {
    const privateKey = await window.crypto.subtle.importKey(
        "pkcs8",
        privateKeyBuf instanceof ArrayBuffer
            ? privateKeyBuf
            : new Uint8Array(privateKeyBuf),
        {
            name: "RSA-OAEP",
            hash: {name: "SHA-256"}
        },
        true,
        ["decrypt"]
    );
    const privateKeyJwk = await window.crypto.subtle.exportKey('jwk', privateKey);
    const publicKey = await window.crypto.subtle.importKey(
        'jwk',
        {
            n: privateKeyJwk.n,
            e: privateKeyJwk.e,
            alg: privateKeyJwk.alg,
            kty: privateKeyJwk.kty,
        },
        {
            name: "RSA-OAEP",
            hash: { name: "SHA-256" }
        },
        true,
        ["encrypt"]
    );
    return window.crypto.subtle.exportKey('spki', publicKey);
}

export function base64ToArrayBuffer(base64: string): ArrayBuffer {
    const binaryString = atob(base64);
    const bytes = new Uint8Array(binaryString.length);
    for (let i = 0; i < binaryString.length; i++) {
        bytes[i] = binaryString.charCodeAt(i);
    }
    return bytes.buffer;
}