import { BerytusEncryptedPacket } from "@berytus/types-extd";
import { CipherBoxOptions, AbstractCipherBox } from "./CipherBox";
import { CompactEncrypt, compactDecrypt } from 'jose';
import parseDataURL from 'data-urls';

interface BaseJWEPacketCipherBoxOptions {
    key: CryptoKey;
    avoidReEncryption?: boolean;
}

interface JWEPacketCipherBoxOptions extends BaseJWEPacketCipherBoxOptions,
    Omit<CipherBoxOptions<BerytusEncryptedPacket, ArrayBuffer>, "encrypt" | "decrypt"> {}

export default class JWEPacketCipherBox extends AbstractCipherBox<BerytusEncryptedPacket, ArrayBuffer> {
    #opts: BaseJWEPacketCipherBoxOptions;

    constructor(opts: JWEPacketCipherBoxOptions) {
        super({
            ignoreProp: opts.ignoreProp,
            ignoreValue: opts.ignoreValue,
            encrypt: (d, p) => {
                return this.#encrypt(d);
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

    isCiphertextType(ciph: unknown): ciph is BerytusEncryptedPacket {
        return typeof ciph === "object" &&
            ciph !== null
            && "type" in ciph
            && ciph.type === "JWE"
            && "value" in ciph
            && typeof ciph.value === "string";
    }

    async #encrypt(datum: string | ArrayBufferLike | ArrayBufferView): Promise<BerytusEncryptedPacket> {
        if (this.#opts.avoidReEncryption && this.isCiphertextType(datum)) {
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
        return {
            type: "JWE",
            value: jweCompact
        };
    }

    async #decrypt(datum: BerytusEncryptedPacket, path?: string): Promise<ArrayBuffer> {
        const { plaintext } = await compactDecrypt(datum.value, this.#opts.key);
        if (plaintext.buffer instanceof ArrayBuffer) {
            return plaintext.buffer;
        }
        throw new Error(
            'jose returned plaintext of unsupported data type. Maybe a SharedArrayBuffer?'
        );
    }
}