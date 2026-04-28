import type { APIRoute } from 'astro';
import { Account } from '@root/backend/db/models/Account.js';
import { AuthSession } from '@root/backend/db/models/AuthSession.js';
import { Result, ReqBody } from './schema.js';
import { transformField } from '@root/backend/logic/field-handler/index.js';
import { releaseAssert } from '@root/backend/utils/assert.js';

export const POST: APIRoute = async ({ locals, params }) => {
    const { version: versionParam } = params;
    releaseAssert(typeof versionParam === "string", "Version param is missing.");
    const accountVersion = Number(versionParam);
    let body: ReqBody;
    try {
        body = await ReqBody.parseAsync(locals.requestBody);
    } catch (e) {
        return new Response(JSON.stringify({
            "error": "Bad input"
        }), {
            status: 400,
            headers: {
                'Content-Type': 'application/json'
            }
        });
    }
    const { fields } = body;
    const transformedFields = await Promise.all(
        fields.map(f => transformField(accountVersion, f))
    );
    const account = await Account.getAccount(
        accountVersion,
        transformedFields
    );
    if (! account) {
        return new Response(JSON.stringify({
            "error": "Account was not found."
        }), {
            status: 404,
            headers: {
                "Content-Type": "application/json"
            }
        });
    }

    const session = await AuthSession.createSession(
        account.accountId,
        accountVersion
    );

    const result: Result = {
        "sessionId": String(session.sessionId)
    };

    return new Response(JSON.stringify(result), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}