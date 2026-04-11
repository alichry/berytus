import { z } from 'zod';

export const Result = z.object({
    parameters: z.any()
});

export type Result = z.infer<typeof Result>;