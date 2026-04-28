import type { UserFieldInput, IFieldHandler } from "./types";
import { z } from "zod";

export const SecurePasswordFieldInput = z.object({
    id: z.string(),
    value: z.object({
        salt: z.instanceof(Blob),
        verifier: z.instanceof(Blob)
    })
}).required();

export const EncodedSecurePasswordFieldValue = z.object({
    salt: z.string(), // base64-encoded string
    verifier: z.string() // base64-encoded string
}).required();

export type EncodedSecurePasswordFieldValue = z.infer<typeof EncodedSecurePasswordFieldValue>;

export const StoredSecurePasswordField = z.object({
    id: z.string(),
    value: EncodedSecurePasswordFieldValue
}).required();

export type StoredSecurePasswordField = z.infer<typeof StoredSecurePasswordField>;

export class SecurePasswordHandler implements IFieldHandler {

    async transform(field: UserFieldInput): Promise<StoredSecurePasswordField> {
        const { id, value } = await SecurePasswordFieldInput.parseAsync(field);
        return {
            id,
            value: {
                salt: (await value.salt.bytes())
                    // @ts-ignore: Node 25+
                    .toBase64(),
                verifier: (await value.verifier.bytes())
                    // @ts-ignore: Node 25+
                    .toBase64(),
            }
        };
    }
}