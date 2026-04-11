import { z } from "zod";

export const Result = z.object({
    userAttributes: z.array(z.object({
        id: z.string(),
        value: z.string()
    }))
});
export type Result = z.infer<typeof Result>;