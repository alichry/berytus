import type { APIRoute } from 'astro';
import { Account } from '@root/backend/db/models/Account.js';
import { AuthSession } from '@root/backend/db/models/AuthSession';
import { Result, ReqBody } from './schema';

export const POST: APIRoute = async ({ params, request }) => {
    let body: ReqBody;
    try {
        body = ReqBody.parse(
            await request.json()
        );
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
    const { accountVersion, fields } = body;
    const account = await Account.getAccount(
        accountVersion,
        fields
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
        "sessionId": session.sessionId
    };

    return new Response(JSON.stringify(result), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}