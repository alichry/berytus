import type { APIRoute } from 'astro';
import { Channel } from '@root/backend/db/models/Channel.js';
import { releaseAssert } from '@root/backend/utils/assert.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { E2EEHandler } from '@root/backend/logic/e2ee-orchestration/E2EEHandler.js';
import { IllegalStateError } from '@root/backend/errors/IllegalStateError';
import { Body } from './schema.js';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError.js';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';

export const POST: APIRoute = async ({ request, params }) => {
    const { channelId } = params;
    releaseAssert(typeof channelId === "string");
    const { signature } = await Body.parseAsync(await request.json());
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

    try {
        const channelRequest = await ChannelRequest.getRequest(channel.requestId);
        if (! channelRequest.supportsE2EE()) {
            throw new IllegalStateError(
                "Channel does not support end-to-end encryption."
            );
        }
        const handler = new E2EEHandler();
        if (! channel.keyAgreementParameters) {
            throw new IllegalStateError("Key agreement parameters not set for channel.");
        }
        const valid = await handler.verifyPeerKapSignature(
            channel.keyAgreementParameters,
            signature
        );
        if (! valid) {
            throw new InvalidArgError(
                "Invalid signature for the provided key agreement parameters."
            );
        }
        await channel.setScmKapSignature(signature);
    } catch (e) {
        if (e instanceof IllegalStateError || e instanceof InvalidArgError) {
            return new Response(JSON.stringify({
                error: e.message
            }), {
                status: 409,
                headers: {
                    "Content-Type": "application/json"
                }
            });
        }
        throw e;
    }
    return new Response(null, {
        status: 204
    });
};