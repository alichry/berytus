import type { FieldInput } from "@root/backend/db/types";
import type { IFieldHandler } from "./types";
import { z } from "zod";
import { armoredKeySchema, ArmoredKeyUtils } from "@root/backend/utils/key-utils.js";

export const ArmoredPublicKeyFieldValue = z.object({
    publicKey: armoredKeySchema("public")
}).required();

export type ArmoredPublicKeyFieldValue = z.infer<typeof ArmoredPublicKeyFieldValue>;

export const PublicKeyFieldInput = z.object({
    id: z.string(),
    value: z.object({
        publicKey: z.instanceof(ArrayBuffer)
    })
}).required();

export interface TransformedPublicKeyFieldInput {
    id: string;
    value: ArmoredPublicKeyFieldValue
}

export class DigitalSignatureHandler implements IFieldHandler {

    async transform(field: FieldInput): Promise<TransformedPublicKeyFieldInput> {
        const { id, value } = await PublicKeyFieldInput.parseAsync(field);
        return {
            id,
            value: {
                publicKey: ArmoredKeyUtils.armorBase64(
                    Buffer.from(value.publicKey).toString('base64'),
                    "public"
                )
            }
        };
    }
}