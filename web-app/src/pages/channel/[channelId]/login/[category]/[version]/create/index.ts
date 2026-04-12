import type { APIRoute } from 'astro';
import { Account } from '@root/backend/db/models/Account.js';
import { transformField } from '@root/backend/logic/field-handler';
import { AccountUserAttributes } from '@root/backend/db/models/AccountUserAttributes';
import { Body } from './schema';


export const POST: APIRoute = async ({ params, locals }) => {
    const { version } = params;
    const { fields, userAttributes } = Body.parse(locals.requestBody);

    const transformedFields = await Promise.all(
        fields.map(f => transformField(Number(version), f))
    );
    const acc = await Account.createAccount(
        Number(version),
        transformedFields
    );
    const transformedUserAttributes: Record<string, string> = {};
    await Promise.all(
        Object.keys(userAttributes).map(async key => {
            if (typeof userAttributes[key] === "string") {
                transformedUserAttributes[key] = userAttributes[key];
                return;
            }
            if (
                userAttributes[key].type === "text/plain"
                || userAttributes[key].type.startsWith("text/plain;")
            ) {
                transformedUserAttributes[key] = await userAttributes[key].text();
                return;
            }
            transformedUserAttributes[key] = new Uint8Array(await userAttributes[key].arrayBuffer())
                // @ts-ignore: Node 25+
                .toBase64()
        })
    );
    await AccountUserAttributes.createUserAttributes(
        acc.accountId,
        transformedUserAttributes
    );

    return new Response(JSON.stringify({}), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}