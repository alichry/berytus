import { z } from "zod";

export const Result = z.object({
    identity: z.array(z.object({
        id: z.string(),
        value: z.string()
    })),
    userAttributes: z.array(z.object({
        id: z.string(),
        value: z.string()
    }))
});
export type Result = z.infer<typeof Result>;