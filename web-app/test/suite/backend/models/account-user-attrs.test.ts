import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { pool } from '@root/backend/db/pool.js';
import { AccountUserAttributes } from '@root/backend/db/models/AccountUserAttributes.js';
import { createAccounts, getAccountIds } from "@test/seed/account.js";
import { strict as assert } from 'node:assert';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus Account User Attributes", () => {

    const getExistingUserAttributes = async () => {
        const userAttrs = await pool`
            SELECT AccountID, UserAttributeDictionary
            FROM berytus_account_user_attributes
        `;
        assert(userAttrs.length > 0);
        return userAttrs.map(({
            accountid: accountId,
            userattributedictionary: userAttributes
        }) => {
            assert(typeof accountId === 'bigint');
            assert(typeof userAttributes === 'object');
            assert(userAttributes !== null);
            return {
                accountId,
                userAttributes
            };
        });
    }

    beforeEach(async () => {
        await createAccountDefs();
        await createAccounts();
    });

    it('Should create a user attribute dictionary', async () => {
        const accountIds = await getAccountIds();
        const accountId = accountIds[accountIds.length - 1];
        const existingAttributes = await getExistingUserAttributes();
        assert(! existingAttributes.find(a => a.accountId === accountId));
        const dict = { 'name': 'Mr. X', 'gender': 'Male' };
        const creationResult = await AccountUserAttributes.createUserAttributes(
            accountId,
            dict
        );
        expect(creationResult.accountId).to.equal(accountId);
        expect(creationResult.userAttributes).to.deep.equal(dict);
        expect(existingAttributes).to.not.deep.include(
            {
                accountId: creationResult.accountId,
                userAttributes: creationResult.userAttributes
            }
        );
        const retrievalResult = await AccountUserAttributes.getUserAttributes(
            accountId
        );
        expect(retrievalResult.accountId).to.equal(accountId);
        expect(retrievalResult.userAttributes).to.deep.equal(dict);
        const updatedExistingAttributes = await getExistingUserAttributes();
        expect(updatedExistingAttributes).to.deep.include(
            {
                accountId: retrievalResult.accountId,
                userAttributes: retrievalResult.userAttributes
            }
        );
    });

    it('Should correctly lists user attributes', async () => {
        const accountIds = await getAccountIds();
        const accountId = accountIds[accountIds.length - 1];
        assert(! (await getExistingUserAttributes()).find(a => a.accountId === accountId));
        await AccountUserAttributes.createUserAttributes(
            accountId,
            { 'name': 'Mr. X', 'gender': 'Male' }
        );
        const existingAttributes = await getExistingUserAttributes();
        for (const existingAttributeDictionary of existingAttributes) {
            const attributes = await AccountUserAttributes.getUserAttributes(
                existingAttributeDictionary.accountId
            );
            expect(attributes.accountId).to.equal(
                existingAttributeDictionary.accountId
            );
            expect(attributes.userAttributes).to.deep.equal(
                existingAttributeDictionary.userAttributes
            );
        }
    });
});