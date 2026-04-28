import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";
import type { ISessionKeyLoader, KeyMaterial } from "./types";
import { debugAssert } from "@root/backend/utils/assert.js";

export class AesGcmKeyLoader implements ISessionKeyLoader {
    async importKey(keyMaterial: KeyMaterial): Promise<CryptoKey> {
        return await crypto.subtle.importKey(
            "raw",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(keyMaterial),
            {
                name: "AES-GCM",
                length: 256
            },
            true,
            ['encrypt', 'decrypt']
        );
    }
    async exportKey(key: CryptoKey): Promise<KeyMaterial> {
        if (key.algorithm.name !== "AES-GCM") {
            throw new InvalidArgError("Key must be an AES-GCM key");
        }
        if (key.extractable === false) {
            throw new InvalidArgError("Key must be extractable");
        }
        debugAssert(() => key.type === "secret");
        const buf = await crypto.subtle.exportKey(
            "raw",
            key
        );
        return new Uint8Array(buf)
            // @ts-ignore: Node 25+
            .toBase64();
    }

}