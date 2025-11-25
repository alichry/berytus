import * as chai  from 'chai';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { AccountDefField } from "@root/backend/db/models/AccountDefField.js";
import chaiAsPromised from 'chai-as-promised';
import { createAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { AccountDefAuthChallenge } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { pool } from '@root/backend/db/pool.js';
import { AccountDefKeyFieldList } from '@root/backend/db/models/AccountDefKeyFieldList.js';
const { expect } = chai;
chai.use(chaiAsPromised);


describe("Berytus Account Def", () => {

    const getExistingChallenges = async () => {
        const challenges = await pool`
            SELECT ChallengeID, AccountVersion,
                   ChallengeType, ChallengeParameters
            FROM berytus_account_def_auth_challenge
        `;
        return challenges.map(({
            challengeid: challengeId,
            accountversion: accountVersion,
            challengetype: challengeType,
            challengeparameters: challengeParameters
        }) => {
            expect(challengeId).to.be.a('string');
            expect(accountVersion).to.be.a('number');
            expect(challengeType).to.be.a('string');
            expect(challengeParameters).to.be.a("object");
            return {
                challengeId,
                accountVersion,
                challengeType,
                challengeParameters
            };
        });
    }
    const getExistingChallenge = async () => {
        return (await getExistingChallenges())[0];
    }
    const getExistingKeyFields = async () => {
        const fields = await pool`
            SELECT AccountVersion, FieldID
            FROM berytus_account_def_key_field_id
        `;
        return fields.map(({
            accountversion: accountVersion,
            fieldid: fieldId
        }) => {
            expect(accountVersion).to.be.a('number');
            expect(fieldId).to.be.a('string');
            return { accountVersion, fieldId };
        });
    }
    const groupByAccountVersion = <T extends { accountVersion: number }>(
        records: Array<T>
    ) => {
        return records.reduce((prev, curr) => {
            if (!(curr.accountVersion in prev)) {
                prev[curr.accountVersion] = [];
            }
            prev[curr.accountVersion].push(curr);
            return prev;
        }, {} as Record<string, Array<T>>);
    }

    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
    });

    it("Should correctly retrieve field def", async () => {
        const fieldDef = await AccountDefField.getField(1, "username");
        expect(fieldDef.accountVersion).to.equal(1);
        expect(fieldDef.fieldId).to.equal("username");
        expect(fieldDef.fieldOptions).to.deep.equal({ "private": false, "humanReadable": true, "maxLength": 24 });
        expect(fieldDef.fieldType).to.equal("Identity");
    });

    it("Should correctly return challenge def existance", async () => {
        const existingChallenge = await getExistingChallenge();

        expect(await AccountDefAuthChallenge.challengeDefExists(
            existingChallenge.challengeId,
            existingChallenge.accountVersion
        )).to.be.true;
        expect(await AccountDefAuthChallenge.challengeDefExists(
            'dummy',
            1000
        )).to.be.false;
    });

    it("Should correctly retrieve challenge def", async () => {
        const existingChallenge = await getExistingChallenge();
        const challenge = await AccountDefAuthChallenge.getChallengeDef(
            existingChallenge.challengeId,
            existingChallenge.accountVersion
        );
        expect(challenge.challengeId).to.equal(existingChallenge.challengeId);
        expect(challenge.accountVersion).to.equal(existingChallenge.accountVersion);
        expect(challenge.challengeType).to.equal(existingChallenge.challengeType);
        expect(challenge.challengeParameters).to.deep.equal(existingChallenge.challengeParameters);
    });

    it("Should correctly list all challenge defs", async () => {
        const existingChallenges = (await getExistingChallenges());
        const byAccountVersion = groupByAccountVersion(existingChallenges);
        const selectedChallenges = Object.values(byAccountVersion)
            .sort((a, b) => b.length - a.length)[0];
        const challenges = await AccountDefAuthChallenge.getAllChallengeDef(
            selectedChallenges[0].accountVersion
        );
        expect(challenges.length).to.equal(selectedChallenges.length);
        for (const challenge of challenges) {
            expect(selectedChallenges).to.deep.include(challenge);
        }
    });

    it("Should correctly list all key field defs", async () => {
        const existingKeyFieldDefs = await getExistingKeyFields();
        const byAccountVersion = groupByAccountVersion(existingKeyFieldDefs);
        const selectedKeyFieldDefs = Object.values(byAccountVersion)
            .sort((a, b) => b.length - a.length)[0];
        const keyFieldDefList = await AccountDefKeyFieldList.fetchList(
            selectedKeyFieldDefs[0].accountVersion
        );
        expect(keyFieldDefList.accountVersion).to.equal(selectedKeyFieldDefs[0].accountVersion);
        expect(keyFieldDefList.fields.length).to.equal(selectedKeyFieldDefs.length);
        for (let i = 0; i < keyFieldDefList.fields.length; i++) {
            const fieldId = keyFieldDefList.fields[i].fieldId;
            expect(fieldId).to.be.a('string');
            expect(selectedKeyFieldDefs).to.deep.include({
                accountVersion: keyFieldDefList.accountVersion,
                fieldId
            });
        }
    });
});