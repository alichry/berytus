import * as chai  from 'chai';
import chaiAsPromised from 'chai-as-promised';
import { createAccountDefs } from "@test/seed/account-defs.js";
import { createAccountChallengeDefs, getAccountChallengeDefs } from '@test/seed/account-challenge-defs.js';
import { createAccounts } from '@test/seed/account.js';
import { pool, toPostgresBigInt } from '@root/backend/db/pool.js';
import { strict as assert } from 'node:assert';
import { AuthChallenge, EAuthOutcome } from '@root/backend/db/models/AuthChallenge.js';
import { createAuthSessions, getAuthSessions } from '@test/seed/auth-session.js';
import { createAuthChallenges, getAuthChallenges } from '@test/seed/auth-challenge.js';
import { createAuthChallengeMessages } from '@test/seed/auth-challenge-message.js';
import { AuthChallengeMessage } from '@root/backend/db/models/AuthChallengeMessage.js';
import { EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import { UpsertChallengeAndMessages } from '@root/backend/db/actions/UpsertChallengeAndMessages.js';
import { EntityNotFoundError } from '@root/backend/db/errors/EntityNotFoundError.js';
import { AuthError } from '@root/backend/db/errors/AuthError.js';
import { InvalidArgError } from '@root/backend/errors/InvalidArgError.js';
const { expect } = chai;
chai.use(chaiAsPromised);

describe("Berytus UpsertChallengeAndMessages", () => {
    beforeEach(async () => {
        await createAccountDefs();
        await createAccountChallengeDefs();
        await createAccounts();
        await createAuthSessions();
        await createAuthChallenges();
        await createAuthChallengeMessages();
    });

    const getSessionThatCanCreateSrpChallenge = async () => {
        const sessions = await getAuthSessions();
        const challenges = await getAuthChallenges();
        const challengeDefs = await getAccountChallengeDefs();
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
            return [session, srpChDef] as const;
        }
        assert(false, "could not find appropriate session . challenge def");
    }

    it("Should insert challenge and insert messages correctly", async () => {
        const [session, challengeDef] = await getSessionThatCanCreateSrpChallenge();
        await expect(AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(EntityNotFoundError);

        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: true },
                expected: { dummyResponse: true },
                response: { dummyResponse: true },
                statusMsg: 'Ok',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await action.execute();
        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedChallenge.outcome).to.equal(
            EAuthOutcome.Pending
        );
        const retrievedMessages = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedMessages).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));
        const messages2 = [
            {
                messageName: "ComputeClientProof",
                request: { anotherDummyRequest: 5 },
                expected: { anotherDummyExpected: 6 },
                response: { anotherDummyResponse: 6 },
                statusMsg: 'Ok',
            },
            {
                messageName: "VerifyServerProof",
                request: { anotherDummyRequest: 50 },
                expected: { anotherDummyExpected: 60 },
                response: { anotherDummyResponse: 60 },
                statusMsg: 'Ok',
            },
        ] as const;
        const action2 = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages: messages2
        });
        await action2.execute();
        const retrievedChallengeAtT1 = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const retrievedMessagesAtT1 = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedMessagesAtT1).to.deep.equal(
            [...messages, ...messages2].map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            }))
        );
        expect(retrievedChallengeAtT1.outcome).to.equal(
            EAuthOutcome.Succeeded
        );
    });

    it("Should update challenge and update messages correctly", async () => {
        const [session, challengeDef] = await getSessionThatCanCreateSrpChallenge();
        await expect(AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(EntityNotFoundError);

        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: true },
                expected: { dummyResponse: true },
                response: { dummyResponse: true },
                statusMsg: 'Ok',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: null,
                statusMsg: null,
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await action.execute();
        const retrievedChallenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedChallenge.outcome).to.equal(
            EAuthOutcome.Pending
        );
        const retrievedMessages = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedMessages).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));

        const messages2 = [
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const,
            {
                messageName: "ComputeClientProof",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: null,
                statusMsg: null,
            } as const
        ] as const;
        const action2 = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages: messages2
        });
        await action2.execute();
        const retrievedChallengeAtT1 = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedChallengeAtT1.outcome).to.equal(
            EAuthOutcome.Pending
        );
        const retrievedMessagesAtT1 = await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(retrievedMessagesAtT1).to.deep.equal(
            [...messages.slice(0, 1), ...messages2].map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            }))
        )
    });

    it("Marks challenge as aborted if message has Non-Ok status [no prev messages, mlen = 1]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: 'Error:Bad proof',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await action.execute();
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Aborted);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        ))).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));
    });

    it("Marks challenge as aborted if message has Non-Ok status [no prev messages, mlen = 2]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: 'Error:Bad proof',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await action.execute();
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Aborted);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        ))).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));
    });

    it("Marks challenge as aborted if message has Non-Ok status [with prev messages, mlen = 1]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messagesT0 = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const messagesT1 = [
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: 'Error:Bad proof',
            } as const
        ] as const;
        {
            const action = new UpsertChallengeAndMessages({
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId,
                messages: messagesT0
            });
            await action.execute();
            expect((await AuthChallenge.getChallenge(
                session.sessionId,
                challengeDef.challengeId
            )).outcome).to.equal(EAuthOutcome.Pending);
            expect((await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challengeDef.challengeId
            ))).to.deep.equal(messagesT0.map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            })));
        }
        {
            const action = new UpsertChallengeAndMessages({
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId,
                messages: messagesT1
            });
            await action.execute();
            expect((await AuthChallenge.getChallenge(
                session.sessionId,
                challengeDef.challengeId
            )).outcome).to.equal(EAuthOutcome.Aborted);
            expect((await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challengeDef.challengeId
            ))).to.deep.equal([...messagesT0, ...messagesT1].map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            })));
        }
    });

    it("Marks challenge as aborted if message has Non-Ok status [with prev messages, mlen = 2]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messagesT0 = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const messagesT1 = [
            {
                messageName: "ComputeClientProof",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const,
                        {
                messageName: "VerifyServerProof",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: 'Error:Bad Proof',
            } as const
        ] as const;
        {
            const action = new UpsertChallengeAndMessages({
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId,
                messages: messagesT0
            });
            await action.execute();
            expect((await AuthChallenge.getChallenge(
                session.sessionId,
                challengeDef.challengeId
            )).outcome).to.equal(EAuthOutcome.Pending);
            expect((await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challengeDef.challengeId
            ))).to.deep.equal(messagesT0.map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            })));
        }
        {
            const action = new UpsertChallengeAndMessages({
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId,
                messages: messagesT1
            });
            await action.execute();
            expect((await AuthChallenge.getChallenge(
                session.sessionId,
                challengeDef.challengeId
            )).outcome).to.equal(EAuthOutcome.Aborted);
            expect((await AuthChallengeMessage.getAllMessages(
                session.sessionId,
                challengeDef.challengeId
            ))).to.deep.equal([...messagesT0, ...messagesT1].map(m => ({
                ...m,
                sessionId: session.sessionId,
                challengeId: challengeDef.challengeId
            })));
        }
    });

    it("Rejects forward message creation [1. no prev messages]", async () => {
        const [session, challengeDef] = await getSessionThatCanCreateSrpChallenge();
        await expect(AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(EntityNotFoundError);

        const messages = [
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });

        await expect(action.execute())
            .to.be.rejectedWith(AuthError);
        await expect(AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(EntityNotFoundError);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });

    it("Rejects forward message creation [2. with prev messages]", async () => {
        const [session, challengeDef] = await getSessionThatCanCreateSrpChallenge();
        await expect(AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).to.be.rejectedWith(EntityNotFoundError);

        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await action.execute();
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        ))).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));

        const messages2 = [
            {
                messageName: "ComputeClientProof",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action2 = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages: messages2
        });

        await expect(action2.execute())
            .to.be.rejectedWith(AuthError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        ))).to.deep.equal(messages.map(m => ({
            ...m,
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId
        })));
    });

    it("Rejects when challenge's outcome is aborted", async() => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        const challenge = await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        await challenge.updateOutcome(EAuthOutcome.Aborted);
        expect(challenge.outcome).to.equal(EAuthOutcome.Aborted);
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(AuthError);
    });

    it("Rejects when challenge's outcome is succeeded", async() => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        await pool`
            UPDATE berytus_account_auth_challenge
            SET Outcome = 'Succeeded'
            WHERE SessionID = ${toPostgresBigInt(session.sessionId)}
            AND   ChallengeID = ${challengeDef.challengeId}
        `;
        const challenge = await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        expect(challenge.outcome).to.equal(EAuthOutcome.Succeeded);
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(AuthError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Succeeded);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });

    it("Rejects invalid messages [Non-Ok (Aborted) not at n - 1]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Error:Bad',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(InvalidArgError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });

    it("Rejects invalid messages [Non-Ok (null) not at n - 1]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: null,
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 2 },
                statusMsg: 'Ok',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(InvalidArgError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });

    it("Rejects invalid messages [multiple Non-Oks (Error)]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: 'Error:Bad',
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 3 },
                response: { dummyResponse: 2 },
                statusMsg: 'Error:Bad Proof',
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(InvalidArgError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });

    it("Rejects invalid messages [multiple Non-Oks (null)]", async () => {
        const [session, challengeDef] =
            await getSessionThatCanCreateSrpChallenge();
        await AuthChallenge.createChallenge(
            session.sessionId,
            challengeDef.challengeId
        );
        const messages = [
            {
                messageName: "SelectSecurePassword",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 2 },
                response: { dummyResponse: 3 },
                statusMsg: null,
            } as const,
            {
                messageName: "ExchangePublicKeys",
                request: { dummyRequest: 1 },
                expected: { dummyResponse: 3 },
                response: { dummyResponse: 2 },
                statusMsg: null,
            } as const
        ] as const;
        const action = new UpsertChallengeAndMessages({
            sessionId: session.sessionId,
            challengeId: challengeDef.challengeId,
            messages
        });
        await expect(action.execute())
            .to.be.rejectedWith(InvalidArgError);
        expect((await AuthChallenge.getChallenge(
            session.sessionId,
            challengeDef.challengeId
        )).outcome).to.equal(EAuthOutcome.Pending);
        expect((await AuthChallengeMessage.getAllMessages(
            session.sessionId,
            challengeDef.challengeId
        )).length).to.equal(0);
    });
});