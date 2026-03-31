import type { APIRoute } from 'astro';
import { Channel } from '@root/backend/db/models/Channel.js';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';
import { releaseAssert } from '@root/backend/utils/assert.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { E2EEHandler } from '@root/backend/logic/e2ee-orchestration/E2EEHandler.js';
import { IllegalStateError } from '@root/backend/errors/IllegalStateError';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError.js';

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
    try {
        const channelRequest = await ChannelRequest.getRequest(channel.requestId);
        if (! channelRequest.supportsE2EE()) {
            throw new IllegalStateError("Channel does not supports E2EE");
        }
        if (! channel.keyAgreementParameters) {
            throw new IllegalStateError("Key agreement parameters not set for channel.");
        }
        if (! channel.keyAgreementSignatures) {
            throw new IllegalStateError("Key agreement signatures not set for channel.");
        }
        if (! channel.keyAgreementSignatures.scm) {
            throw new IllegalStateError("SCM key agreement signature not set for channel.");
        }
        const handler = new E2EEHandler();
        const key = await handler.deriveSessionKey(
            channel.keyAgreementParameters,
            channelRequest.webAppX25519.private
        );
        await channel.setSessionKey(key);
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