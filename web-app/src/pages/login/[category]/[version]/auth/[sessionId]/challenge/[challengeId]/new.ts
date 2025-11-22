import type { APIRoute } from 'astro';
import { initiateChallenge } from '@root/backend/logic/challenge-handler';

export const POST: APIRoute = async ({ params }) => {
    const { sessionId, challengeId } = params;
    if (typeof sessionId === "undefined") {
        return new Response(JSON.stringify({
            error: "Missing session id path paramemter"
        }), { status: 400 });
    }
    const handler = await initiateChallenge(
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