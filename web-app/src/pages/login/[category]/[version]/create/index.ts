import type { APIRoute } from 'astro';
import { Account } from '@root/backend/db/models/Account.js';
import { transformField } from '@root/backend/logic/field-handler';
import { AccountUserAttributes } from '@root/backend/db/models/AccountUserAttributes';
import { Body } from './schema';


export const POST: APIRoute = async ({ params, request }) => {
    const { category, version } = params;
    const { fields, userAttributes } = Body.parse(await request.json());

    const transformedFields = await Promise.all(
        fields.map(f => transformField(Number(version), f))
    );
    const acc = await Account.createAccount(
        Number(version),
        transformedFields
    );
    await AccountUserAttributes.createUserAttributes(
        acc.accountId,
        userAttributes
    );

    return new Response(JSON.stringify({}), {
        headers: {
            "Content-Type": "application/json"
        }
    });
}