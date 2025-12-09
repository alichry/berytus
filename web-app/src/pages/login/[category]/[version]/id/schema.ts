import { z } from 'zod';
import { Field } from '../common';

export const ReqBody = z.object({
    accountVersion: z.number(),
    fields: z.array(Field),
});

export const Result = z.object({
    sessionId: z.string().regex(/^[1-9][0-9]*$/)
});

export type Result = z.infer<typeof Result>;
export type ReqBody = z.infer<typeof ReqBody>;