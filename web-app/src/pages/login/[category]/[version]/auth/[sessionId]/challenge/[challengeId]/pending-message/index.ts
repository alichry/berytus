import type { APIRoute } from 'astro';
import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import { Result } from './schema.js';
import {
    validateChallengeDefExists,
    validateParamsHasSessionIdAndChallengeId,
    validatePendingSessionState
} from '../common-validations.js';
import { UserError } from '@root/backend/errors/UserError.js';

export const GET: APIRoute = async ({ params }) => {
    let sessionId: string, challengeId: string;
    try {
        validateParamsHasSessionIdAndChallengeId(params);
        sessionId = params.sessionId;
        challengeId = params.challengeId;
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