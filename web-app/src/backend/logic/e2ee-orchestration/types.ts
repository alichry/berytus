import type { KeyAgreementParametersJson } from "@root/backend/db/models/Channel";

export type KeyMaterial = string;

export type KeyPairMaterial = {
    publicKey: KeyMaterial;
    privateKey: KeyMaterial;
}

export interface IE2EEHandler {
    /**
     * @param kap order of keys matters, must be the same key order
     *  put forth by BerytusKeyAgreementParameters::toCanonicalJSON()
     * @returns signature (base64-encoded)
     */
    signKeyAgreementParameters(
        kap: KeyAgreementParametersJson
    ): Promise<string>;
    verifyPeerKapSignature(
        kap: KeyAgreementParametersJson,
        peerSignature: ArrayBuffer
    ): Promise<boolean>;
    deriveSessionKey(
        kap: KeyAgreementParametersJson,
        selfExchangePrivateKey: KeyMaterial,
    ): Promise<KeyMaterial>;
}

export type ImportKeyInputBoth = KeyPairMaterial;
export type ImportKeyInputPublic = Pick<ImportKeyInputBoth, "publicKey">;
export type ImportKeyInputPrivate = Pick<ImportKeyInputBoth, "privateKey">;

export type ImportKeyInput =
    | ImportKeyInputBoth
    | ImportKeyInputPrivate
    | ImportKeyInputPublic;

export type ImportKeyResultBoth = CryptoKeyPair;
export type ImportKeyResultPublic = Pick<ImportKeyResultBoth, "publicKey">;
export type ImportKeyResultPrivate = Pick<ImportKeyResultBoth, "privateKey">;

export type ImportKeyResult =
    | ImportKeyResultBoth
    | ImportKeyResultPublic
    | ImportKeyResultPrivate;

export type ExportKeyInputBoth = CryptoKeyPair;
export type ExportKeyInputPublic = Pick<ExportKeyInputBoth, "publicKey">;
export type ExportKeyInputPrivate = Pick<ExportKeyInputBoth, "privateKey">;

export type ExportKeyInput =
    | ExportKeyInputBoth
    | ExportKeyInputPublic
    | ExportKeyInputPrivate;

export type ExportKeyResultBoth = {
    publicKey: KeyMaterial;
    privateKey: KeyMaterial;
}
export type ExportKeyResultPublic = Pick<ExportKeyResultBoth, "publicKey">;
export type ExportKeyResultPrivate = Pick<ExportKeyResultBoth, "privateKey">;

export type ExportKeyResult =
    | ExportKeyResultBoth
    | ExportKeyResultPublic
    | ExportKeyResultPrivate;

export interface ISigningKeyStore {
    getPrivateKeyMaterial(): Promise<KeyMaterial>;
    getPublicKeyMaterial(): Promise<KeyMaterial>;
}

export interface ISigningKeyLoader {
    importKey(input: ImportKeyInputBoth): Promise<ImportKeyResultBoth>;
    importKey(input: ImportKeyInputPublic): Promise<ImportKeyResultPublic>;
    importKey(input: ImportKeyInputPrivate): Promise<ImportKeyResultPrivate>;
    importKey(input: ImportKeyInput): Promise<ImportKeyResult>;

    exportKey(input: ExportKeyInputBoth): Promise<ExportKeyResultBoth>;
    exportKey(input: ExportKeyInputPublic): Promise<ExportKeyResultPublic>;
    exportKey(input: ExportKeyInputPrivate): Promise<ExportKeyResultPrivate>;
    exportKey(input: ExportKeyInput): Promise<ExportKeyResult>;
}

export interface IExchangeKeyGenerator {
    generateKeyPair(): Promise<KeyPairMaterial>;
}

export interface IExchangeKeyLoader {
    importKey(input: ImportKeyInputBoth): Promise<ImportKeyResultBoth>;
    importKey(input: ImportKeyInputPublic): Promise<ImportKeyResultPublic>;
    importKey(input: ImportKeyInputPrivate): Promise<ImportKeyResultPrivate>;
    importKey(input: ImportKeyInput): Promise<ImportKeyResult>;

    exportKey(input: ExportKeyInputBoth): Promise<ExportKeyResultBoth>;
    exportKey(input: ExportKeyInputPublic): Promise<ExportKeyResultPublic>;
    exportKey(input: ExportKeyInputPrivate): Promise<ExportKeyResultPrivate>;
    exportKey(input: ExportKeyInput): Promise<ExportKeyResult>;
}

// symmetric key
export interface ISessionKeyLoader {
    importKey(keyMaterial: KeyMaterial): Promise<CryptoKey>;
    exportKey(key: CryptoKey): Promise<KeyMaterial>;
}