import type { APIRoute } from 'astro';
import { z } from "zod";
import { Account } from '@root/backend/db/models/Account.js';
import { Field } from './common';
import { transformField } from '@root/backend/logic/field-handler/index.js';

const Body = z.object({
  fields: z.array(Field),
});

export const POST: APIRoute = async ({ params, locals }) => {
    const { version } = params;
    const { fields } = Body.parse(locals.requestBody);
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