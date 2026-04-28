import z from "zod";

export const Body = z.array(z.string());

export type Body = z.infer<typeof Body>;

export const Result = z.record(z.union([
    z.string(),
    z.instanceof(Blob)
]));