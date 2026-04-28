import { z } from "zod";

export const Body = z.object({
    channelId: z.string(),
    channelRequestId: z.string(),
    scmActor: z.object({
        ed25519Key: z.string()
    })
});

export type Body = z.infer<typeof Body>;