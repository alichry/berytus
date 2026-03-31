import type { APIRoute } from 'astro';
import { Channel } from '@root/backend/db/models/Channel.js';
import { releaseAssert } from '@root/backend/utils/assert.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { E2EEHandler } from '@root/backend/logic/e2ee-orchestration/E2EEHandler';
import { Body, KeyAgreementParameters } from './schema';
import { IllegalStateError } from '@root/backend/errors/IllegalStateError';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest';

export const POST: APIRoute = async ({ request, params }) => {
    const { channelId } = params;
    releaseAssert(typeof channelId === "string");
    const input = await Body.parseAsync(await request.json());
    const kap = await KeyAgreementParameters.parseAsync(JSON.parse(input.canonicalJson));
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
        const signed = await handler.signKeyAgreementParameters(kap);
        await channel.setKeyAgreementParameters(kap);
        await channel.setWebAppKapSignature(signed);
        return new Response(JSON.stringify({
            signature: signed
        }), {
            status: 200,
            headers: {
                "Content-Type": "application/json"
            }
        });
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
};