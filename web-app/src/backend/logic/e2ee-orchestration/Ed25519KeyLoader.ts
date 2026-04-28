import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";
import type { ExportKeyInput, ExportKeyInputBoth, ExportKeyInputPrivate, ExportKeyInputPublic, ExportKeyResult, ExportKeyResultBoth, ExportKeyResultPrivate, ExportKeyResultPublic, ImportKeyInput, ImportKeyInputBoth, ImportKeyInputPrivate, ImportKeyInputPublic, ImportKeyResult, ImportKeyResultBoth, ImportKeyResultPrivate, ImportKeyResultPublic, ISigningKeyLoader, KeyMaterial, KeyPairMaterial } from "./types";
import { uniformReturn } from "./utils.js";

export class Ed25519KeyLoader implements ISigningKeyLoader {
    exportKey(input: ExportKeyInputBoth): Promise<ExportKeyResultBoth>;
    exportKey(input: ExportKeyInputPublic): Promise<ExportKeyResultPublic>;
    exportKey(input: ExportKeyInputPrivate): Promise<ExportKeyResultPrivate>;
    exportKey(input: ExportKeyInput): Promise<ExportKeyResult>;
    async exportKey(input: ExportKeyInput): Promise<ExportKeyResult> {
        let privateMaterial: KeyMaterial | undefined = undefined;
        let publicMaterial: KeyMaterial | undefined = undefined;
        if ("privateKey" in input) {
            privateMaterial = await this.#exportPrivateKey(input.privateKey);
        }
        if ("publicKey" in input) {
            publicMaterial = await this.#exportPublicKey(input.publicKey);
        }
        return uniformReturn(publicMaterial, privateMaterial);
    }

    importKey(input: ImportKeyInputBoth): Promise<ImportKeyResultBoth>;
    importKey(input: ImportKeyInputPublic): Promise<ImportKeyResultPublic>;
    importKey(input: ImportKeyInputPrivate): Promise<ImportKeyResultPrivate>;
    importKey(input: ImportKeyInput): Promise<ImportKeyResult>;
    public async importKey(input: ImportKeyInput): Promise<ImportKeyResult> {
        let privateKey: CryptoKey | undefined = undefined;
        let publicKey: CryptoKey | undefined = undefined;
        if ("privateKey" in input) {
            privateKey = await this.#importPrivateKey(input.privateKey);
        }
        if ("publicKey" in input) {
            publicKey = await this.#importPublicKey(input.publicKey);
        }
        return uniformReturn(publicKey, privateKey);
    }

    async #importPrivateKey(meterial: KeyMaterial): Promise<CryptoKey> {
        return await crypto.subtle.importKey(
            "pkcs8",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(meterial)
                .buffer,
            "Ed25519",
            false,
            ["sign"]
        );
    }

    async #importPublicKey(meterial: KeyMaterial): Promise<CryptoKey> {
        return await crypto.subtle.importKey(
            "spki",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(meterial)
                .buffer,
            "Ed25519",
            true,
            ["verify"]
        );
    }

    async #exportPublicKey(key: CryptoKey): Promise<KeyMaterial> {
        if (key.algorithm.name !== "Ed25519") {
            throw new InvalidArgError("Key must be an Ed25519 key");
        }
        if (key.type !== "public") {
            throw new InvalidArgError("Key must be a public key");
        }
        if (key.extractable === false) {
            throw new InvalidArgError("Key must be extractable");
        }
        const spki = await crypto.subtle.exportKey(
            "spki",
            key
        );
        return new Uint8Array(spki)
            // @ts-ignore: Node 25+
            .toBase64();
    }

    async #exportPrivateKey(key: CryptoKey): Promise<KeyMaterial> {
        if (key.algorithm.name !== "Ed25519") {
            throw new InvalidArgError("Key must be an Ed25519 key");
        }
        if (key.type !== "private") {
            throw new InvalidArgError("Key must be a private key");
        }
        if (key.extractable === false) {
            throw new InvalidArgError("Key must be extractable");
        }
        const pkcs8 = await crypto.subtle.exportKey(
            "pkcs8",
            key
        );
        return new Uint8Array(pkcs8)
            // @ts-ignore: Node 25+
            .toBase64();
    }
}