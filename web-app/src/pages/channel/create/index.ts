import type { APIRoute } from 'astro';
import { Body } from './schema';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { Channel } from '@root/backend/db/models/Channel.js';

export const POST: APIRoute = async ({ request }) => {
    const { channelId, channelRequestId, scmActor } =  Body.parse(await request.json());
    try {
        await Channel.create(
            channelId,
            BigInt(channelRequestId),
            scmActor
        );
    } catch (err) {
        if (err instanceof EntityNotFoundError) {
            return new Response(JSON.stringify({
                error: `${err.entityType} not found [${err.keyName}=${err.key}]`
            }), {
                status: 404,
                headers: {
                    "Content-Type": "application/json"
                }
            });
        }
        throw err;
    }
    return new Response(null, {
        status: 204
    });
};