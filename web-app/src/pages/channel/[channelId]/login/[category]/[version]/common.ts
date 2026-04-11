import { z } from "zod";

export const FieldValue = z.union([
    z.string(),
    z.object({
        publicKey: z.instanceof(Blob)
        // ^ regardless of whether e2ee is enabled
        // or not, format is the same :)
    }).strict().required()
]);

export type FieldValue = z.infer<typeof FieldValue>;

export const Field = z.object({
    id: z.string(),
    value: FieldValue
});

export type Field = z.infer<typeof Field>;