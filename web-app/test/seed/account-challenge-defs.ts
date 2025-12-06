import { EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";
import { useConnection } from "../../src/backend/db/pool.js"
import { strict as assert } from 'node:assert';
import type { JSONValue } from "@root/backend/db/types.js";

const stmts = [
    `INSERT INTO berytus_account_def_auth_challenge
    (AccountVersion, ChallengeID, ChallengeType, ChallengeParameters)
    VALUES
    (1, 'password', 'Password', '{ "passwordFieldIds": ["password"] }'),
    (2, 'secure-remote-password', 'SecureRemotePassword', '{ "todo": true }'),
    (3, 'digital-signature', 'DigitalSignature', '{ "keyFieldId": "key" }'),
    (4, 'secure-remote-password', 'SecureRemotePassword', '{ "todo": true }'),
    (4, 'digital-signature', 'DigitalSignature', '{ "keyFieldId": "key" }'),
    (1000, 'email-code', 'OffChannelOtp', '{ "todo": true }'),
    (1000, 'secure-remote-password', 'SecureRemotePassword', '{ "todo": true }'),
    (1001, 'email-code', 'OffChannelOtp', '{ "todo": true }'),
    (2000, 'secure-remote-password', 'SecureRemotePassword', '{ "todo": true }');`
];

let cachedAccountChallengeDefs: Array<{
    accountVersion: number;
    challengeId: string;
    challengeType: EChallengeType;
    challengeParameters: Record<string, JSONValue>
}> | null = null;

export const createAccountChallengeDefs = async () => {
    return useConnection(async conn => {
        for (let i = 0; i < stmts.length; i++) {
            await conn.unsafe(stmts[i]);
        }
        const rows = await conn`
            SELECT AccountVersion, ChallengeID,
                   ChallengeType, ChallengeParameters
            FROM berytus_account_def_auth_challenge
            ORDER BY CreatedAt ASC
        `;
        cachedAccountChallengeDefs = rows.map(({
            accountversion: accountVersion,
            challengeid: challengeId,
            challengetype: challengeType,
            challengeparameters: challengeParameters
        }) => {
            assert(typeof accountVersion === 'number');
            assert(accountVersion > 0);
            assert(typeof challengeId === 'string');
            assert(challengeId.trim().length > 0);
            assert(challengeType in EChallengeType);
            assert(typeof challengeParameters === 'object');
            return {
                accountVersion,
                challengeId,
                challengeType,
                challengeParameters
            };
        });
        assert(cachedAccountChallengeDefs.length > 0);
        return cachedAccountChallengeDefs;
    });
}

export const getAccountChallengeDefs = async () => {
    if (cachedAccountChallengeDefs) {
        return cachedAccountChallengeDefs;
    }
    const challengeDefs = await createAccountChallengeDefs();
    assert(challengeDefs.length > 0);
    return challengeDefs;
}

export type ChallengeDefs = Awaited<ReturnType<typeof getAccountChallengeDefs>>;