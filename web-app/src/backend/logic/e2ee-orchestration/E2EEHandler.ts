import type { KeyAgreementParametersJson } from "@root/backend/db/models/Channel";
import { releaseAssert } from "@root/backend/utils/assert.js";
import type { IExchangeKeyLoader, ISessionKeyLoader, ISigningKeyLoader, ISigningKeyStore, KeyMaterial } from "./types.js";
import { StaticSigningKeyStore } from "./StaticSigningKeyStore.js";
import { Ed25519KeyLoader } from "./Ed25519KeyLoader.js";
import { AesGcmKeyLoader } from "./AesGcmKeyLoader.js";
import { X25519KeyLoader } from "./X25519KeyLoader.js";

const { subtle } = globalThis.crypto;

export class E2EEHandler {
    #signingKeyStore: ISigningKeyStore;
    #signingKeyLoader: ISigningKeyLoader;
    #exchangeKeyLoader: IExchangeKeyLoader;
    #sessionKeyLoader: ISessionKeyLoader;

    public constructor(
        signingKeyStore: ISigningKeyStore = new StaticSigningKeyStore(),
        signingKeyLoader: ISigningKeyLoader = new Ed25519KeyLoader(),
        exchangeKeyLoader: IExchangeKeyLoader = new X25519KeyLoader(),
        sessionKeyLoader: ISessionKeyLoader = new AesGcmKeyLoader()
    ) {
        this.#signingKeyStore = signingKeyStore;
        this.#signingKeyLoader = signingKeyLoader;
        this.#exchangeKeyLoader = exchangeKeyLoader;
        this.#sessionKeyLoader = sessionKeyLoader;
    }

    /**
     * @param kap order of keys mattered, must be the same key order
     *  put forth by BerytusKeyAgreementParameters::toCanonicalJSON()
     * @returns signature (base64-encoded)
     */
    public async signKeyAgreementParameters(
        kap: KeyAgreementParametersJson
    ): Promise<string> {
        releaseAssert(
            (await this.#signingKeyStore.getPublicKeyMaterial()) ===
                kap.authentication.public.webApp);
        const keyMaterial = await this.#signingKeyStore.getPrivateKeyMaterial();
        const { privateKey: key } = await this.#signingKeyLoader.importKey({
            privateKey: keyMaterial
        });
        const signedBuf = await subtle.sign(
            "Ed25519",
            key,
            new TextEncoder().encode(JSON.stringify(kap))
        );
        return new Uint8Array(signedBuf)
            // @ts-ignore: Node 25+
            .toBase64();
    }

    public async verifyPeerKapSignature(
        kap: KeyAgreementParametersJson,
        peerSignature: string
    ): Promise<boolean> {
        const peerKeyMaterial =
            kap.authentication.public.scm;
        const { publicKey: peerPublicKey } = 
            await this.#signingKeyLoader.importKey({
                publicKey: peerKeyMaterial
            });
        return await subtle.verify(
            "Ed25519",
            peerPublicKey,
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(peerSignature),
            new TextEncoder().encode(JSON.stringify(kap))
        );
    }

    public async deriveSessionKey(
        kap: KeyAgreementParametersJson,
        selfExchangePrivateKey: KeyMaterial,
    ): Promise<KeyMaterial> {
        const peerKeyMaterial = kap.exchange.public.scm;
        const { publicKey: peerX25519PubKey } =
            await this.#exchangeKeyLoader.importKey(
                { publicKey: peerKeyMaterial }
            );
        const { privateKey: selfX25519PrivKey } =
            await this.#exchangeKeyLoader.importKey(
                { privateKey: selfExchangePrivateKey }
            );
        releaseAssert(kap.exchange.name === "X25519", `kap.exchange.name === "X25519"`);
        const sharedKey = await crypto.subtle.deriveKey(
            {
                name: "X25519",
                public: peerX25519PubKey
            },
            selfX25519PrivKey,
            'HKDF',
            false,
            ['deriveKey']
        );
        releaseAssert(kap.derivation.name === "HKDF", `kap.derivation.name === "HKDF"`);
        releaseAssert(kap.derivation.hash === "SHA-256", `kap.derivation.hash === "SHA-256"`);
        releaseAssert(kap.generation.name === "AES-GCM", `kap.generation.name === "AES-GCM"`);
        releaseAssert(kap.generation.length === 256, `kap.generation.length === 256`);
        const sessionKey = await crypto.subtle.deriveKey(
            {
                name: kap.derivation.name,
                hash: kap.derivation.hash,
                salt: Uint8Array
                    // @ts-ignore: Node 25+
                    .fromBase64(kap.derivation.salt),
                info: Uint8Array
                    // @ts-ignore: Node 25+
                    .fromBase64(kap.derivation.info),
            },
            sharedKey,
            kap.generation,
            true,
            ['encrypt', 'decrypt']
        );
        return await this.#sessionKeyLoader.exportKey(sessionKey);
    }
}