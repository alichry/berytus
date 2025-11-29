import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts } from '@test/seed/account.js';
import { pool } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallengeMessage } from '@root/backend/db/models/AuthChallengeMessage.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { UpsertChallengeAndInsertMessages } from '@root/backend/db/actions/UpsertChallengeAndInsertMessages.js';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus UpsertChallengeAndInsertMessages", () => {
    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });

    it("Should upsert challenge and insert messages correctly", async () => {
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const [session, challengeDef] = (() => {
            for (const session of sessions) {
                if (session.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const srpChDef = challengeDefs.find(
                    d => d.challengeType === EChallengeType.SecureRemotePassword
                        && d.accountVersion === session.accountVersion
                    );
                if (! srpChDef) {
                    continue;
                }
                const hasChallenge = challenges.find(
                    c => c.sessionId === session.sessionId
                        && c.challengeId === srpChDef.challengeId
                );
                if (hasChallenge) {
                    continue;
                }
                return [session, srpChDef];
            }
            assert(false, "could not find appropriate session . challenge def");
        })();
        const action = new UpsertChallengeAndInsertMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages: [
                {
                    messageName: "SelectSecurePassword",
                    request: { dummyRequest: true },
                    expected: { dummyResponse: true },
                    response: { dummyResponse: true },
                    statusMsg: 'Ok',
                },
                {
                    messageName: "ExchangePublicKeys",
                    request: { dummyRequest: 1 },
                    expected: { dummyResponse: 2 },
                    response: { dummyResponse: 2 },
                    statusMsg: 'Ok',
                }
            ]
        });
        await action.execute();
    });
});