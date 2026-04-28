import { z } from 'zod';

export const Result = z.object({
    outcome: z.enum([
        "Pending", "Aborted", "Succeeded"
    ]),
    statusMsg: z.string()
});

export type Result = z.infer<typeof Result>;