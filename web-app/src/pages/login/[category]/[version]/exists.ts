import type { APIRoute } from 'astro';
import { z } from "zod";
import { Account } from '@root/backend/db/models/Account.js';
import { Field } from './common';

const Body = z.object({
  fields: z.array(Field),
});

export const POST: APIRoute = async ({ params, request }) => {
    const { category, version } = params;
    const { fields } = Body.parse(await request.json());

    const exists = await Account.accountExists(
        Number(version),
        fields
    );

    return new Response(JSON.stringify({
        exists
    }), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}