import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts, getAccountIds } from '@test/seed/account.js';
import { AuthSession } from '@root/backend/db/models/AuthSession.js';
import { Account } from '@root/backend/db/models/Account.js';
import { pool } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions } from '@test/seed/auth-session.js';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus Auth Session", () => {

    const getExistingSessions = async () => {
        const rows = await pool`
            SELECT SessionID, AccountID,
                   AccountVersion, Outcome
            FROM berytus_account_auth_session
        `;
        return rows.map(({
            sessionid: sessionId,
            accountid: accountId,
            accountversion: accountVersion,
            outcome
        }) => {
            assert(typeof sessionId === 'bigint');
            assert(sessionId > 0n);
            assert(typeof accountId === 'bigint');
            assert(accountId > 0n);
            assert(typeof accountVersion === 'number');
            assert(accountVersion > 0);
            assert(typeof outcome === 'string');
            assert(outcome in EAuthOutcome);
            return {
                sessionId,
                accountId,
                accountVersion,
                outcome
            };
        });
    }

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
    });

    it("Should create a session and correctly return it", async () => {
        const accountIds = await getAccountIds();
        const accountId = accountIds[accountIds.length - 1];
        assert(
            !(await getExistingSessions())
                .find(sess => sess.accountId === accountId)
        );
        const { accountVersion } = await Account.latest(accountId);

        const sess = await AuthSession.createSession(
            accountId, accountVersion
        );
        expect(sess.accountId).to.equal(accountId);
        expect(sess.accountVersion).to.equal(accountVersion);
        expect(sess.sessionId).to.be.a('bigint');
        expect(sess.sessionId.valueOf() > 0n).to.be.true;
        expect(sess.outcome).to.equal('Pending');
        const existingSessions = await getExistingSessions();
        expect(existingSessions).to.deep.include(sess);
    });

    it("Should correctly retrieve a session", async () => {
        const existingSessions = await getExistingSessions();
        for (const existingSession of existingSessions) {
            const retrievedSession = await AuthSession.getSession(
                existingSession.sessionId
            );
            expect(retrievedSession).to.deep.equal(existingSession);
        }
    });

    xit("Should correctly mark session as succeeded (.finish())", async () => {

    });

    xit("Should reject marking session as succeeded when at least one challenge is still pending", async () => {

    });

    xit("Should reject marking a session as succeeded when at least one challenge is aborted", async () => {

    });
});