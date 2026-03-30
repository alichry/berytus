import type { APIRoute } from 'astro';
import { Channel } from '@root/backend/db/models/Channel.js';
import { releaseAssert } from '@root/backend/utils/assert.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';

export const POST: APIRoute = async ({ request, params }) => {
    const { channelId } = params;
    releaseAssert(typeof channelId === "string");
    let channel;
    try {
        channel = await Channel.getChannel(channelId);
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
    await channel.closeChannel();
    return new Response(null, {
        status: 204
    });
};