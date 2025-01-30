import type { FieldInput } from "@root/backend/db/types";
import type { IFieldHandler } from "./types";
import { z } from "zod";
import { armoredKeySchema } from "@root/backend/utils/key-utils";

export const PublicKeyFieldValue = z.object({
    publicKey: armoredKeySchema("public")
}).required();

export type PublicKeyFieldValue = z.infer<typeof PublicKeyFieldValue>;

export const PublicKeyFieldInput = z.object({
    id: z.string(),
    value: PublicKeyFieldValue
}).required();

export class DigitalSignatureHandler implements IFieldHandler {

    async transform(field: FieldInput) {
        const transformed = await PublicKeyFieldInput.parseAsync(field);
        return transformed;
    }
}