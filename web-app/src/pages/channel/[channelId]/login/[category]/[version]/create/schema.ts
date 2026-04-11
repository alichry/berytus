import { z } from "zod";
import { Field } from "../common";

export const Body = z.object({
    fields: z.array(Field),
    userAttributes: z.record(z.string(), z.string())
  });

export type Body = z.infer<typeof Body>;