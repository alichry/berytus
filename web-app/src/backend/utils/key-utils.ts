import { InvalidArgError } from "../errors/InvalidArgError";
import { z } from "zod";
import { subtle, webcrypto } from "node:crypto";

export const armoredKeySchema = (type: "public" | "private" = "public") =>
    z.string().refine(
        (val) => {
            try {
                ArmoredKeyUtils.validateArmor(val, type);
            } catch (e) {
                return false;
            }
            return true;
        },
        { message: "Expected key to have valid armoring; gotherwise" }
    )

export const unarmorKeySchema = (type: "public" | "private" = "public") => z.string()
    .transform((val, ctx): string => {
        try {
            return ArmoredKeyUtils.extractBase64(val, type);
        } catch (e) {
            if (e instanceof InvalidArgError) {
                ctx.addIssue({ code: 'custom', message: e.message });
                return z.NEVER;
            }
            throw e;
        }
    });


class ArmoredKeyUtils {
    static #header = {
        private: `-----BEGIN PRIVATE KEY-----\n`,
        public: `-----BEGIN PUBLIC KEY-----\n`
    };
    static #footer = {
        private: `-----END PRIVATE KEY-----`,
        public: `-----END PUBLIC KEY-----`
    };

    static validateArmor(pemStr: string, type: "public" | "private" = "public"): void {
        if (! pemStr.startsWith(ArmoredKeyUtils.#header[type])) {
            throw new InvalidArgError(
                'Expected key to have a valid armor header; got otherwise.'
            );
        }
        if (! pemStr.endsWith(ArmoredKeyUtils.#footer[type])) {
            throw new InvalidArgError(
                 'Expected key to have a valid armor footer; got otherwise.'
            );
        }
    }

    static extractBase64(pemStr: string, type: "public" | "private" = "public"): string {
        this.validateArmor(pemStr, type);
        const body = pemStr.substring(
            ArmoredKeyUtils.#header[type].length,
            pemStr.length - ArmoredKeyUtils.#footer[type].length
        );
        const base64 = body.replace(/\r?\n|\r/g, '');
        if (base64.length === 0) {
            throw new InvalidArgError(
                'Expected unarmored key to contain some data; got otherwise.'
            );
        }
        return base64;
    }
}

export class KeyUtils {
    static async importArmoredKeyForVerification(
        armored: string
    ) {
        const bodyBase64 = ArmoredKeyUtils.extractBase64(armored);
        const keyBuf = Buffer.from(bodyBase64, 'base64');
        const key = await subtle.importKey(
            "spki",
            keyBuf,
            {
                name: "RSASSA-PKCS1-v1_5",
                hash: "SHA-256"
            },
            true,
            ["verify"]
        );
        return key;
    }
}

export class SignUtils {
    static verify(key: CryptoKey, signature: webcrypto.BufferSource, data: webcrypto.BufferSource) {
        return subtle.verify(
            "RSASSA-PKCS1-v1_5",
            key,
            signature,
            data
        );
    }
}