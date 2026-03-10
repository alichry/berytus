export type KeyMaterial = string;

export type KeyPairMaterial = {
    publicKey: KeyMaterial;
    privateKey: KeyMaterial;
}

export interface IE2EEHandler {

}

export interface ISigningKeyStore {
    exportAsSpkiString(): Promise<string>;
    getSigningCryptoKey(): Promise<CryptoKey>;
}

export interface IExchangeKeyGenerator {
    generateKeyPair(): Promise<KeyPairMaterial>;
}

export interface IExchangeKeyLoader {
    importPrivateKey(
        privateKeyMaterial: KeyMaterial
    ): Promise<CryptoKey>;
}