import { type CipherBoxOptions, AbstractCipherBox } from "./CipherBox";
import { CompactEncrypt, compactDecrypt } from 'jose';
import parseDataURL from 'data-urls';

interface BaseJWECompactCipherBoxOptions {
    key: CryptoKey;
    avoidReEncryption?: boolean;
}

interface JWECompactCipherBoxOptions extends BaseJWECompactCipherBoxOptions,
    Omit<CipherBoxOptions<string, Blob>, "encrypt" | "decrypt"> {}

export default class JWECompactCipherBox extends AbstractCipherBox<string, Blob> {
    #opts: BaseJWECompactCipherBoxOptions;

    constructor(opts: JWECompactCipherBoxOptions) {
        super({
            ignoreProp: opts.ignoreProp,
            ignoreValue: opts.ignoreValue,
            transformDecrypted: opts.transformDecrypted,
            encrypt: (d, p) => {
                return this.#encrypt(d, p);
            },
            decrypt: (d, p) => {
                return this.#decrypt(d, p);
            },
        });
        this.#opts = {
            key: opts.key,
            avoidReEncryption: opts.avoidReEncryption !== undefined
                ? opts.avoidReEncryption
                : true
        };
    }

    public isCiphertextType(
        ciph: unknown,
        path?: ReadonlyArray<string>
    ): ciph is string {
        if (typeof ciph !== "string") {
            return false;
        }
        const parts = ciph.split('.');
        if (parts.length !== 5) {
            return false;
        }
        let header;
        try {
            header = JSON.parse(
                new TextDecoder().decode(
                    // @ts-ignore: node >= 25
                    Uint8Array.fromBase64(parts[0])
                )
            );
        } catch (e) {
            return false;
        }
        if (typeof header !== "object" || header === null) {
            return false;
        }
        return header.alg && header.enc;
    }

    async #encrypt(
        datum: string | ArrayBufferLike | ArrayBufferView | Blob,
        path?: ReadonlyArray<string>
    ): Promise<string> {
        if (this.#opts.avoidReEncryption && this.isCiphertextType(datum, path)) {
            console.warn("Avoiding re-encryption of existing JWE packet.");
            return datum;
        }
        let content: { mimeType: string; plaintext: Uint8Array; } | undefined = undefined;
        if (typeof datum === "string") {
            const dataURL = parseDataURL(datum);
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
                    plaintext: new TextEncoder().encode(datum),
                    mimeType: "text/plain;charset=utf-8"
                };
            }
        } else if (
            datum instanceof ArrayBuffer ||
            datum instanceof SharedArrayBuffer
        ) {
            content = {
                plaintext: new Uint8Array(datum),
                mimeType: "application/octet-stream"
            };
        } else if (ArrayBuffer.isView(datum)) {
            content = {
                plaintext: new Uint8Array(datum.buffer),
                mimeType: "application/octet-stream"
            };
        } else if (datum instanceof Blob) {
            content = {
                plaintext: await datum.bytes(),
                mimeType: datum.type || "application/octet-stream"
            };
        }
        if (! content) {
            throw new Error(`Cannot encrypt datum of unsupported '${typeof datum}' type.`);
        }
        const op = new CompactEncrypt(
            content.plaintext
        );
        const jweCompact = await op
            .setProtectedHeader({
                //alg: 'ECDH-ES',
                alg: 'dir',
                enc: 'A256GCM',
                cty: content.mimeType,
                typ: "JOSE"
            })
            .encrypt(this.#opts.key);
        return jweCompact;
    }

    async #decrypt(datum: string, path?: ReadonlyArray<string>): Promise<Blob> {
        const { plaintext, protectedHeader } = await compactDecrypt(datum, this.#opts.key);
        if (!(plaintext.buffer instanceof ArrayBuffer)) {
            throw new Error(
                'jose returned plaintext of unsupported data type. Maybe a SharedArrayBuffer?'
            );
        }
        return new Blob([plaintext.buffer], { type: protectedHeader.cty })
    }
}