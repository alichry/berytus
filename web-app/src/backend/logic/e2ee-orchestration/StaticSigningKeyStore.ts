import type { ISigningKeyStore, KeyMaterial } from "./types";
import { signingKeySpki, signingKeyPkcs8 } from "@root/backend/env/e2ee.js";

export class StaticSigningKeyStore implements ISigningKeyStore {
    async getPrivateKeyMaterial(): Promise<KeyMaterial> {
        return signingKeyPkcs8;
    }
    async getPublicKeyMaterial(): Promise<KeyMaterial> {
        return signingKeySpki;
    }
}