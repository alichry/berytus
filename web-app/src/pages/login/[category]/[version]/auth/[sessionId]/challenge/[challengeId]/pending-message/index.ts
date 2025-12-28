import type { APIRoute } from 'astro';
import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import { Result } from './schema.js';
import {
    validateChallengeDefExists,
    validatePendingSessionState
} from '@root/pages/login/[category]/[version]/auth/[sessionId]/utils.state-validation.js';
import { UserError } from '@root/backend/errors/UserError.js';
import { releaseAssert } from '@root/backend/utils/assert.js';

export const GET: APIRoute<
    Record<string, any>,
    { sessionId: string; challengeId: string; }
> = async ({ params }) => {
    releaseAssert(typeof params["sessionId"] === "string");
    releaseAssert(typeof params["challengeId"] === "string");
    const { sessionId, challengeId } = params;
    try {
        const sessionToken =
            await validatePendingSessionState(BigInt(sessionId));
        await validateChallengeDefExists(
            sessionToken,
            challengeId
        );
    } catch (e) {
        if (e instanceof UserError) {
            return new Response(JSON.stringify({
                error: e.message
            }), { status: 400 });
        }
        throw e;
    }
    const handler = await setupChallenge(
        BigInt(sessionId),
        challengeId!
    );
    try {
        const msg = await handler.getPendingMessage();
        if (! msg) {
            throw new Error("Expecting first message to be non-null!");
        }
        const result: Result = {
            nextMessage: {
                messageName: msg.messageName,
                request: msg.request
            }
        };
        return new Response(JSON.stringify(result));
    } finally {
        handler.destroy();
    }
}