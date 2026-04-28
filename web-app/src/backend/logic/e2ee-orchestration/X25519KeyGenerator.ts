import { releaseAssert } from "@root/backend/utils/assert.js";
import type { IExchangeKeyGenerator, KeyPairMaterial } from "./types";

const { subtle } = globalThis.crypto;

export class X25519KeyGenerator implements IExchangeKeyGenerator {
   public async generateKeyPair(): Promise<KeyPairMaterial> {
           const keyPair = await subtle.generateKey(
               "X25519",
               true,
               ["deriveKey"]
           );
           releaseAssert(
               "publicKey" in keyPair,
               "subtle.generateKey() must return a CryptoKeyPair"
           );
           const publicKey = await subtle.exportKey(
               "spki",
               keyPair.publicKey
           );
           const privateKey = await subtle.exportKey(
               "pkcs8",
               keyPair.privateKey
           );
           return {
               publicKey: new Uint8Array(publicKey)
                   // @ts-ignore: Node 25+
                   .toBase64(),
               privateKey: new Uint8Array(privateKey)
                   // @ts-ignore: Node 25+
                   .toBase64()
           };
       }
}