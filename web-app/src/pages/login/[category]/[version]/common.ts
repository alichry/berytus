import { z } from "zod";

export const FieldValue = z.union([
    z.string(),
    z.object({
        publicKey: z.string()
    }).strict().required()
]);

export type FieldValue = z.infer<typeof FieldValue>;

export const Field = z.object({
    id: z.string(),
    value: FieldValue
});

export type Field = z.infer<typeof Field>;