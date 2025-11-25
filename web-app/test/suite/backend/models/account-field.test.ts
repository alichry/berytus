import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { pool } from '@root/backend/db/pool.js';
import { AccountField } from '@root/backend/db/models/AccountField.js';
import { createAccounts } from "@test/seed/account.js";
import { strict as assert } from 'node:assert';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus Account Field", () => {

    const getExistingFields = async () => {
        const fields = await pool`
            SELECT AccountID, AccountVersion,
                   FieldID, FieldValue
            FROM berytus_account_field
        `;
        assert(fields.length > 0);
        return fields.map(({
            accountid: accountId,
            accountversion: accountVersion,
            fieldid: fieldId,
            fieldvalue: fieldValue
        }) => {
            expect(accountId).to.be.a('bigint');
            expect(accountVersion).to.be.a('number');
            expect(fieldId).to.be.a('string');
            expect(fieldValue).to.be.not.undefined;
            return {
                accountId,
                accountVersion,
                fieldId,
                fieldValue
            };
        });
    }

    beforeEach(async () => {
        await createAccountDefs();
        await createAccounts();
    });

    it("Should correctly retrieve field", async () => {
        const existingFields = await getExistingFields();
        for (const existingField of existingFields) {
            const field = await AccountField.getField(
                existingField.accountVersion,
                existingField.accountId,
                existingField.fieldId
            );
            expect(field).to.deep.equal(existingField);
        }
    });
});