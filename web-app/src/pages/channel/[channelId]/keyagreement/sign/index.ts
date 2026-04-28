import type { APIRoute } from 'astro';
import { Channel } from '@root/backend/db/models/Channel.js';
import { debugAssert, releaseAssert } from '@root/backend/utils/assert.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { E2EEHandler } from '@root/backend/logic/e2ee-orchestration/E2EEHandler';
import { Body, KeyAgreementParameters } from './schema';
import { IllegalStateError } from '@root/backend/errors/IllegalStateError';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest';
import { ZodError } from 'zod';

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
        const input = await Body.parseAsync(await request.json());
        const kap: KeyAgreementParameters = JSON.parse(input.canonicalJson);
        debugAssert(assert => assert(JSON.stringify(kap) === input.canonicalJson));
        await KeyAgreementParameters.parseAsync(kap);
        const channelRequest = await ChannelRequest.getRequest(channel.requestId);
        if (! channelRequest.supportsE2EE()) {
            throw new IllegalStateError(
                "Channel does not support end-to-end encryption."
            );
        }
        const handler = new E2EEHandler();
        const { signature, message } = await handler.signKeyAgreementParameters(kap);
        debugAssert(assert => assert(new TextDecoder().decode(message) === input.canonicalJson));
        await channel.setKeyAgreementParameters(kap);
         await channel.setWebAppKapSignature(signature);
        return new Response(JSON.stringify({
            signature
        }), {
            status: 200,
            headers: {
                "Content-Type": "application/json"
            }
        });
    } catch (e) {
        if (
            e instanceof IllegalStateError ||
            e instanceof InvalidArgError ||
            e instanceof ZodError
        ) {
            return new Response(JSON.stringify({
                error: e.message
            }), {
                status: e instanceof IllegalStateError ? 409 : 400,
                headers: {
                    "Content-Type": "application/json"
                }
            });
        }
        throw e;
    }
};