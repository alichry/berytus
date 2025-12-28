import type { APIRoute } from 'astro';
import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import {
    validateChallengeDefExists,
    validateNewChallengeState,
    validatePendingSessionState
} from '@root/pages/login/[category]/[version]/auth/[sessionId]/utils.state-validation.js';
import { UserError } from '@root/backend/errors/UserError.js';
import { releaseAssert } from '@root/backend/utils/assert.js';

export const POST: APIRoute<
    Record<string, any>,
    { sessionId: string; challengeId: string; }
> = async ({ params }) => {
    releaseAssert(typeof params["sessionId"] === "string");
    releaseAssert(typeof params["challengeId"] === "string");
    const { sessionId, challengeId } = params;
    try {
        const sessionToken = await validatePendingSessionState(BigInt(sessionId));
        await validateNewChallengeState(sessionToken, challengeId);
        await validateChallengeDefExists(sessionToken, challengeId);
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
        await handler.save(); // save the first, pending message.
        return new Response(JSON.stringify({}));
    } finally {
        handler.destroy();
    }
}