import { setupChallenge } from '@root/backend/logic/challenge-handler/index.js';
import type { APIRoute } from 'astro';
import type { Result } from './schema.js';
import {
    validateParamsHasSessionIdAndChallengeId,
    validatePendingSessionState,
    validatePendingChallengeState } from '../common-validations.js';
import { UserError } from '@root/backend/errors/UserError.js';
import { debugAssert } from '@root/backend/utils/assert.js';

export const POST: APIRoute = async ({ params, request }) => {
    let sessionId: string, challengeId: string;
    try {
        validateParamsHasSessionIdAndChallengeId(params);
        sessionId = params.sessionId;
        challengeId = params.challengeId;
        await validatePendingChallengeState(
            await validatePendingSessionState(BigInt(sessionId)),
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
            throw new Error("Expecting next message to be non-null!");
        }
        let msgResponse;
        try {
            msgResponse = await request.json();
        } catch (e) {
            return new Response(JSON.stringify({
                "error": "Invalid request body."
            }), {
                status: 400,
                headers: {
                    "Content-Type": "application/json"
                }
            })
        }
        const statusMsg = await handler.processPendingMessageResponse(
            msgResponse
        );
        await handler.save();
        debugAssert(
            assert =>
                assert(
                    handler.challenge !== null,
                    "handler.challenge !== null"
                )
        );
        const result: Result = {
            outcome: handler.challenge!.outcome,
            statusMsg
        };
        return new Response(JSON.stringify(result), {
            headers: {
                "Content-Type": "application/json"
            }
        });
    } finally {
        handler.destroy();
    }
}