import type { APIRoute } from 'astro';
import { z, ZodError } from "zod";
import { Account } from '@root/backend/db/models/Account.js';
import { Field } from './common';
import { transformField } from '@root/backend/logic/field-handler/index.js';

const Body = z.object({
  fields: z.array(Field),
});

export const POST: APIRoute = async ({ params, locals }) => {
    const { version } = params;
    let parsed;
    try {
        parsed = await Body.parseAsync(locals.requestBody);
    } catch (e) {
        if (e instanceof ZodError) {
            return new Response(JSON.stringify({
                error: e.message
            }), {
                status: 400,
                headers: {
                    'Content-Type': 'application/json'
                }
            });
        }
        throw e;
    }
    const { fields } = parsed;
    const transformedFields = await Promise.all(
        fields.map(f => transformField(Number(version), f))
    );
    const exists = await Account.accountExists(
        Number(version),
        transformedFields
    );

    return new Response(JSON.stringify({
        exists
    }), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}