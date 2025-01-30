import type { APIRoute } from 'astro';
import { initiateChallenge } from '@root/backend/logic/challenge-handler';

export const POST: APIRoute = async ({ params }) => {
    const { category, version, sessionId, challengeId } = params;
    const handler = await initiateChallenge(
        Number(sessionId),
        challengeId!
    );
    try {
        await handler.save(); // save the first, pending message.
        return new Response(JSON.stringify({}));
    } finally {
        handler.destroy();
    }
}