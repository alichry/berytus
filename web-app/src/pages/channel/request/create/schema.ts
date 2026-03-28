import { z } from "zod";

export const Body = z.object({
    type: z.enum(["E2EE", "NonE2EE"])
});

export type Body = z.infer<typeof Body>;

export const Result = z.object({
    webAppActor: z.union([
        z.object({ ed25519Key: z.string() }),
        z.object({ origin: z.string() })
    ]),
    webAppX25519: z.string().nullable(),
    unmaskAllowlist: z.array(z.string()).nullable(),
    channelRequestId: z.string()
});

export type Result = z.infer<typeof Result>;