// NOTE(berytus): Polyfill for Uint8Array.fromBase64 and Uint8Array.prototype.toBase64,
// since Netlify/AWS Lambda does not support node 25+ yet. Remove
// this once we can deploy on Netlify with node 25+ (i.e. 26 TLS)
// @ts-ignore
if (! Uint8Array.fromBase64) {
    console.log(`[NodeJS v${process.version}] Polyfilling Uint8Array.fromBase64 and Uint8Array.prototype.toBase64...`);
    // @ts-ignore
    Uint8Array.fromBase64 = function (base64, options) {
        let encoding: 'base64' | 'base64url' = 'base64';
        if (options?.alphabet === "base64url") {
            encoding = 'base64url';
        }
        if (options?.lastChunkHandling) {
            throw new Error("lastChunkHandling option is not supported in this polyfill.");
        }
        const buf = Buffer.from(base64, encoding);
        const ab = buf.buffer.slice(
            buf.byteOffset,
            buf.byteOffset + buf.byteLength
        );
        return new Uint8Array(ab);
    };
    // @ts-ignore
    Uint8Array.prototype.toBase64 = function (options) {
        let encoding: 'base64' | 'base64url' = 'base64';
        if (options?.alphabet === "base64url") {
            encoding = 'base64url';
        }
        const b64 = Buffer.from(this).toString(encoding);
        if (options?.omitPadding) {
            return b64.replace(/=+$/, '');
        }
        return b64;
    };
}

export const env = (key: string, def?: string): string => {
    const value = key in process.env ? process.env[key] : def;
    if (value === undefined) {
        console.error('Error: missing env', key);
        process.exit(1);
    }
    if (value.length === 0) {
        console.error('Error: empty env', key);
        process.exit(1);
    }
    return value;
}

export const int = (value: string, nonZero = true): number => {
    const num = Number(value);
    if (Number.isNaN(num)) {
        console.error(
            'Error: one of the env values is an invalid number, got NaN for str:',
            value
        );
        process.exit(2);
    }
    if (! Number.isInteger(num)) {
        console.error(
            'Error: one of the env values is an invalid integer, str:',
            value
        );
        process.exit(2);
    }
    if (num < 0) {
        console.error(
            'Error: one of the env values is a valid integer but negative, str:',
            value
        );
        process.exit(2);
    }
    if (nonZero && num === 0) {
        console.error(
            'Error: one of the env values is a valid integer but happened to be 0, str:',
            value
        );
        process.exit(2);
    }
    return num;
}
