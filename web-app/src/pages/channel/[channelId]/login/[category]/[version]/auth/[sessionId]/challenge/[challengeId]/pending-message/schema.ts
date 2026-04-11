import { z } from 'zod';

export const Result = z.object({
    nextMessage: z.object({
        messageName: z.string(),
        request: z.any()
    })
});
export type Result = z.infer<typeof Result>;