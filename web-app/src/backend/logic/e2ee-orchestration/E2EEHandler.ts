import type { KeyAgreementParametersJson } from "@root/backend/db/models/Channel";
import { releaseAssert } from "@root/backend/utils/assert";
import type { IExchangeKeyLoader, ISigningKeyStore, KeyMaterial } from "./types.js";
import { StaticSigningKeyStore } from "./StaticSigningKeyStore.js";
import { X25519KeyComposer } from "./X25519KeyComposer.js";

const { subtle } = globalThis.crypto;

export class E2EEHandler {
    #signingKeyStore: ISigningKeyStore;
    #exchangeKeyLoader: IExchangeKeyLoader;

    public constructor(
        exchangeKeyLoader: IExchangeKeyLoader = new X25519KeyComposer(),
        signingKeyStore: ISigningKeyStore = new StaticSigningKeyStore()
    ) {
        this.#signingKeyStore = signingKeyStore;
        this.#exchangeKeyLoader = exchangeKeyLoader;
    }

    /**
     * @param kap order of keys mattered, must be the same key order
     *  put forth by BerytusKeyAgreementParameters::toCanonicalJSON()
     * TODO(berytus): returned sig should be base64 encoded
     */
    public async signKeyAgreementParameters(
        kap: KeyAgreementParametersJson
    ) {
        const key = await this.#signingKeyStore.getSigningCryptoKey();
        releaseAssert(
            await this.#signingKeyStore.exportAsSpkiString() ===
                kap.authentication.public.webApp);
        return await subtle.sign(
            "Ed25519",
            key,
            new TextEncoder().encode(JSON.stringify(kap))
        );
    }

    public async verifyScmKapSignature(
        kap: KeyAgreementParametersJson,
        signature: ArrayBuffer
    ): Promise<boolean> {
        const scmSpki =
            kap.authentication.public.scm;
        const scmPublicKey = await subtle.importKey(
            "spki",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(scmSpki)
                .buffer,
            "Ed25519",
            true,
            ["verify"]
        );
        return await subtle.verify(
            "Ed25519",
            scmPublicKey,
            signature,
            new TextEncoder().encode(JSON.stringify(kap))
        );
    }

    // TODO(berytus): Returned key shouldd be of type KeyMaterial
    public async deriveSessionKey(
        selfExchangePrivateKey: KeyMaterial,
        kap: KeyAgreementParametersJson
    ) {
        const peerX25519PubKey = await subtle.importKey(
            "spki",
            Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(kap.exchange.public.webApp)
                .buffer,
            "X25519",
            true,
            []
        );
        const selfX25519PrivKey = await this.#exchangeKeyLoader.importPrivateKey(
            selfExchangePrivateKey
        );
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
        return await crypto.subtle.deriveKey(
            {
                ...kap.derivation,
                salt: Uint8Array
                    // @ts-ignore: Node 25+
                    .fromBase64(kap.derivation.salt),
                info: new Uint8Array
                    // @ts-ignore: Node 25+
                    .fromBase64(kap.derivation.info),
            },
            sharedKey,
            kap.generation,
            true,
            ['encrypt', 'decrypt']
        );
    }
}