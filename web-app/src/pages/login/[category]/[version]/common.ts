import { z } from "zod";

export const Field = z.object({
    id: z.string(),
    value: z.string()
});