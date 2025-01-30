import type { APIRoute } from 'astro';
import { loadChallenge } from '@root/backend/logic/challenge-handler';
import { Result } from './schema';

export const GET: APIRoute = async ({ params }) => {
    const { category, version, sessionId, challengeId } = params;
    // TODO: Handle cases where sessionId/challengeId not found,
    //                          incompatible challenge state
    const handler = await loadChallenge(
        Number(sessionId),
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