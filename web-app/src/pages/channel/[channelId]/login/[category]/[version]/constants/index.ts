import type { APIRoute } from 'astro';
import { AccountConstant } from '@root/backend/db/models/AccountConstant.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { Body } from './schema';
import { ZodError } from 'zod';

export const POST: APIRoute = async ({ params, locals, request }) => {
    const { version } = params;
    try {
        const names = await Body.parseAsync(await request.json());
        const result: Record<string, string> = {};
        const records = await Promise.all(
            names.map(n => AccountConstant.getConstant(Number(version!), n))
        );
        for (const record of records) {
            result[record.constantName] = record.value;
        }
        return new Response(JSON.stringify(result), {
            headers: {
                'Content-Type': 'application/json'
            }
        });
    } catch (e) {
        if (e instanceof SyntaxError || e instanceof ZodError || e instanceof EntityNotFoundError) {
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
};