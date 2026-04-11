import type { APIRoute } from 'astro';
import { AuthSession } from '@root/backend/db/models/AuthSession';
import { AuthError } from '@root/backend/db/errors/AuthError';
import { AccountUserAttributes } from '@root/backend/db/models/AccountUserAttributes';
import type { Result } from './schema';

export const POST: APIRoute = async ({ params }) => {
    const { sessionId } = params;
    if (typeof sessionId === "undefined") {
        return new Response(JSON.stringify({
            error: "Missing session id path paramemter"
        }), { status: 400 });
    }

    // TODO(berytus): Check if any challenge is pending
    // and if so, throw UserError
    const session = await AuthSession.getSession(BigInt(sessionId));
    try {
        await session.finish();
        const result: Result = {
            userAttributes: (await AccountUserAttributes.getUserAttributes(session.accountId))
                .asArray()
                .map(({ id, value }) => ({
                    id,
                    value: typeof value === "string"
                        ? value
                        : JSON.stringify(value)
                }))
        };

        return new Response(JSON.stringify(result), {
            headers: {
                "Content-Type": "application/json"
            }
        });
    } catch (e) {
        if (e instanceof AuthError) {
            return new Response(JSON.stringify({
                error: e.message
            }), {
                status: 500,
                headers: {
                    "Content-Type": "application/json"
                }
            })
        }
        throw e;
    }
}