import { loadChallenge } from '@root/backend/logic/challenge-handler';
import type { APIRoute } from 'astro';
import type { Result } from './schema';

export const POST: APIRoute = async ({ params, request }) => {
    const { category, version, sessionId, challengeId } = params;
    const handler = await loadChallenge(
        Number(sessionId),
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
        const result: Result = {
            outcome: handler.challenge.outcome,
            statusMsg
        };
        return new Response( JSON.stringify(result), {
            headers: {
                "Content-Type": "application/json"
            }
        });
    } finally {
        handler.destroy();
    }
}