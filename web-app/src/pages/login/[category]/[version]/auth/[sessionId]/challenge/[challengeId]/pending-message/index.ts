import type { APIRoute } from 'astro';
import { loadChallenge } from '@root/backend/logic/challenge-handler';
import { Result } from './schema';

export const GET: APIRoute = async ({ params }) => {
    const { sessionId, challengeId } = params;
    if (typeof sessionId === "undefined") {
        return new Response(JSON.stringify({
            error: "Missing session id path paramemter"
        }), { status: 400 });
    }
    // TODO: Handle cases where sessionId/challengeId not found,
    //                          incompatible challenge state
    const handler = await loadChallenge(
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