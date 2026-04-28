import type { APIRoute } from 'astro';
import { AuthSession } from '@root/backend/db/models/AuthSession';
import { AuthError } from '@root/backend/db/errors/AuthError';
import { AccountUserAttributes } from '@root/backend/db/models/AccountUserAttributes';
import type { Result } from './schema';
import { AccountField } from '@root/backend/db/models/AccountField';
import { releaseAssert } from '@root/backend/utils/assert';
import { AccountDefField } from '@root/backend/db/models/AccountDefField';

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
    const accountFields = (await AccountField.listFields(session.accountVersion, session.accountId))
    const identityFields = [];
    for (const f of accountFields) {
         const fieldDef = await AccountDefField.getField(
            session.accountVersion,
            f.fieldId
        );
        if (fieldDef.fieldType === "Identity" || fieldDef.fieldType === "ForeignIdentity") {
            identityFields.push(f);
        }
    }
    try {
        const result: Result = {
            identity: identityFields
                .map(f => {
                    releaseAssert(typeof f.fieldValue === "string", 'typeof f.fieldValue === "string"');
                    return {
                        id: f.fieldId,
                        value: f.fieldValue
                    };
                }),
            userAttributes: (await AccountUserAttributes.getUserAttributes(session.accountId))
                .asArray()
                .map(({ id, value }) => ({
                    id,
                    value: typeof value === "string"
                        ? value
                        : JSON.stringify(value)
                }))
        };
        await session.finish();
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