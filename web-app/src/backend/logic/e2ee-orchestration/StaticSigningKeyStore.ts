import type { ISigningKeyStore } from "./types";

const privateKeyPkcs8 = "";
const publicKeySpki = "";

export class StaticSigningKeyStore implements ISigningKeyStore {
    public async exportAsSpkiString(): Promise<string> {
        return publicKeySpki;
    }
    public async getSigningCryptoKey(): Promise<CryptoKey> {
        return await crypto.subtle.importKey(
            "pkcs8",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(privateKeyPkcs8)
                .buffer,
            "Ed25519",
            false,
            ["sign"]
        );
    }
}