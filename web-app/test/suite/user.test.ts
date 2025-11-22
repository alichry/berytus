import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from '../seed/account-defs.js';
import { Account } from '@root/backend/db/models/Account.js';
const { expect } = chai;
chai.use(chaiAsPromised);

const accountsToAdd = [
	{
		//accountId: 21,
		accountVersion: 2000,
		fields: [
			{
				id: "partyId",
				value: 'jerry-and-sons',
			},
			{
				id: "username",
				value: "jerry"
			},
			{
				id: "securePassword",
				value: "securePassJerry"
			},
		]
	},
	{
		//accountId: 22,
		accountVersion: 2000,
		fields: [
			{
				id: "partyId",
				value: 'jerry-and-sons',
			},
			{
				id: "username",
				value: "caleb"
			},
			{
				id: "securePassword",
				value: "securePassCaleb"
			},
		]
	},
	{
		//accountId: 23,
		accountVersion: 2000,
		fields: [
			{
				id: "partyId",
				value: 'marks-and-sons',
			},
			{
				id: "username",
				value: "mark"
			},
			{
				id: "securePassword",
				value: "securePassMark"
			},
		]
	},
	{
		//accountId: 24,
		accountVersion: 2000,
		fields: [
			{
				id: "partyId",
				value: 'marks-and-sons',
			},
			{
				id: "username",
				value: "matty"
			},
			{
				id: "securePassword",
				value: "securePassMatty"
			},
		]
	},
];

describe("Berytus Account", () => {
	beforeEach(async () => {
		await createAccountDefs();
	});

	it("should create account user key fields", async () => {
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			const account = await Account.createAccount(
				accountVersion,
				fields
			);
			expect(account.accountId).to.be.a('bigint');
		}
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			expect(await Account.accountExists(accountVersion, fields))
				.to.be.true;
			const acc = await Account.getAccount(accountVersion, fields);
			expect(acc).to.be.not.null;
			expect(acc!.accountId.valueOf() > 0).to.be.true;
		}
	});

	it("Should handle duplicate account user key fields", async () => {
		const accounts = [];
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			const account = await Account.createAccount(
				accountVersion,
				fields
			);
			expect(account.accountId).to.be.a('bigint');
			accounts.push(account);
		}
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			expect(await Account.accountExists(accountVersion, fields))
				.to.be.true;
			const acc = await Account.getAccount(accountVersion, fields);
			expect(acc).to.be.not.null;
			expect(acc!.accountId).to.equal(accounts[i].accountId);
		}
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			await expect(Account.createAccount(
				accountVersion,
				fields
			)).to.be.rejected;
		}
	});

	it("Should reject invalid field ids", async () => {
		const { accountVersion, fields } = accountsToAdd[0];
		const invalidFields = fields.map(f => {
			return {
				...f,
				id: f.id + '-invalid'
			};
		});
		// await Account.createAccount(accountVersion, invalidFields);
		await expect(Account.createAccount(
			accountVersion,
			invalidFields
		)).to.be.rejected;
	});

	it("Should reject a combination of valid and invalid field ids", async () => {
		const { accountVersion, fields } = accountsToAdd[0];
		const invalidFields = [...fields, { id: 'invalid', value: 'dummy' }];
		await expect(Account.createAccount(
			accountVersion,
			invalidFields
		)).to.be.rejected;
		// rejects with:
		// PostgresError: insert or update on table
		// "berytus_account_field" violates foreign key constraint
		// "fk_baf_accountversionfieldid"
	});
});