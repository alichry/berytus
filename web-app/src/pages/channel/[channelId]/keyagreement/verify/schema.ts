import { z } from "zod";

export const Body = z.object({
    signature: z.string()
});