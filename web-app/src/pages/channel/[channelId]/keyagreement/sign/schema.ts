import { z } from "zod";

export const KeyAgreementParameters = z.object({
    session: z.object({
        id: z.string(),
	    timestamp: z.number(),
        fingerprint: z.object({
            hash: z.string(),
	        version: z.string(),
            salt: z.string(), // base64 encoded
	        value: z.string(), // base64 encoded
        }),
        unmaskAllowlist: z.array(z.string()).optional()
    }),
    authentication: z.object({
        name: z.enum(["Ed25519"]),
	    public: z.object({
            scm: z.string(),
	        webApp: z.string()
        })
    }),
    exchange: z.object({
        name: z.enum(["X25519"]),
	    public: z.object({
            scm: z.string(),
	        webApp: z.string()
        })
    }),
    derivation: z.object({
        name: z.enum(["HKDF"]),
	    hash: z.enum(["SHA-256"]),
        salt: z.string(), // base64 encoded
        info: z.string() // base64 encoded
    }),
    generation: z.object({
        name: z.enum(["AES-GCM"]),
	    length: z.literal(256)
    }),
});

export type KeyAgreementParameters = z.infer<typeof KeyAgreementParameters>;

export const Body = z.object({
    canonicalJson: z.string().refine(str => {
        try {
            JSON.parse(str);
            return true;
        } catch (err) {
            return false;
        }
    }, { message: "canonicalJson must be a valid JSON string" }),
});

export const Result = z.object({
    signature: z.string()
});

export type Result = z.infer<typeof Result>;