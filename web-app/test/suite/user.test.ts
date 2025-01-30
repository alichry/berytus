import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from '../account-defs.js';
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
			await Account.createAccount(
				accountVersion,
				fields
			);
		}
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			expect(await Account.accountExists(accountVersion, fields))
				.to.be.true;
			const acc = await Account.getAccount(accountVersion, fields);
			expect(acc).to.be.not.null;
			expect(acc!.accountId).to.be.greaterThan(0);
		}
	});

	it("Should handle duplicate account user key fields", async () => {
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			await Account.createAccount(
				accountVersion,
				fields
			);
		}
		for (let i = 0; i < accountsToAdd.length; i++) {
			const { accountVersion, fields } = accountsToAdd[i];
			await expect(Account.createAccount(
				accountVersion,
				fields
			)).to.be.rejected;
		}
	});
});