import * as chai  from 'chai';
import { createAccountDefs } from "@test/seed/account-defs.js";
import chaiAsPromised from 'chai-as-promised';
import { createAccounts } from '@test/seed/account.js';
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { strict as assert } from 'node:assert';
import { initiateChallenge } from '@root/backend/logic/challenge-handler/index.js';
import { AuthChallengeMessage } from '@root/backend/db/models/AuthChallengeMessage.js';
import { AccountField } from '@root/backend/db/models/AccountField.js';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus Password Challenge Handler", () => {

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });


    it("Should initiate a challenge and create the first message", async () => {
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
        const [session, challengeDef] = (() => {
            for (const session of sessions) {
                if (session.outcome !== EAuthOutcome.Pending) {
                    continue;
                }
                const passwordChDef = challengeDefs.find(
                    d => d.challengeType === EChallengeType.Password
                        && d.accountVersion === session.accountVersion
                );
                if (! passwordChDef) {
                    continue;
                }
                const hasPasswordCh = challenges.find(
                    c => c.challengeId === passwordChDef.challengeId
                        && c.sessionId === session.sessionId
                );
                if (hasPasswordCh) {
                    continue;
                }
                return [session, passwordChDef];
            }
            assert(false, "can't find appropriate session . challenge def");
        })();
        const passwordFieldId = 'password';
        const passwordField = await AccountField.getField(
            session.accountVersion,
            session.accountId,
            passwordFieldId
        );
        const ch = await initiateChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        await ch.save();
        expect(ch.challenge.challengeDef).to.deep.equal(challengeDef);
        expect(ch.session).to.deep.equal(session);
        const expectedChallenge = {
            challengeDef,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            outcome: EAuthOutcome.Pending
        };
        expect(ch.challenge).to.deep.equal(expectedChallenge);
        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedChallenge).to.deep.equal(expectedChallenge);
        const retrievedMessages = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedMessages).to.deep.equal([
            {
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId,
                messageName: 'GetPasswordFields',
                request: [passwordFieldId],
                expected: [
                    {
                        id: passwordFieldId,
                        password: passwordField.fieldValue
                    }
                ],
                response: null,
                statusMsg: null
            }
        ]);
    });
});