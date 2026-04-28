import { AccountDefAuthChallenge, EChallengeType } from '@root/backend/db/models/AccountDefAuthChallenge.js';
import type { AuthSession } from '@root/backend/db/models/AuthSession.js';
import type { PoolConnection } from '@root/backend/db/pool.js';
import { z } from 'zod';
import { AbstractChallengeHandler, type CCHDependencies, type InputMessagePayload, type Message, type MessageDictionary, type MessageDraft } from './AbstractChallengeHandler.js';
import type { AuthChallengeMessageName, MessagePayload } from '@root/backend/db/models/AuthChallengeMessage';
import { randomBytes } from "crypto";
import { AccountField } from '@root/backend/db/models/AccountField.js';
import { AccountDefField } from '@root/backend/db/models/AccountDefField.js';
import { releaseAssert } from '@root/backend/utils/assert.js';
import { InMemorySrpStore, InvalidClientProofError, SrpServer, type ISrpStore } from '@root/backend/crypto/SrpServer.js';

type MessageName = BerytusSecureRemotePasswordChallengeMessageName;

const messageNames: ReadonlyArray<MessageName> = [
    "SelectSecurePassword",
    "ExchangePublicKeys",
    "ComputeClientProof",
    "VerifyServerProof"
];

const SrpFieldOptions = z.object({
    identityFieldId: z.string()
});

const SrpFieldValue = z.object({
    salt: z.string(), // base64
    verifier: z.string(), // base64
});

export const SelectSecurePasswordExpected = z.string()
    .describe("identityFieldId");

type SelectSecurePasswordExpected = z.infer<typeof SelectSecurePasswordExpected>;

const SelectSecurePasswordResponse = SelectSecurePasswordExpected;

type SelectSecurePasswordResponse = z.infer<typeof SelectSecurePasswordResponse>;

const ExchangePublicKeysResponse = z.instanceof(Blob)
    .describe("Client Public Key (SRP:A)");

type ExchangePublicKeysResponse = z.infer<typeof ExchangePublicKeysResponse>;

const ComputeClientProofResponse = z.instanceof(Blob)
    .describe("Client Proof (SRP:M1)");

type ComputeClientProofResponse = z.infer<typeof ComputeClientProofResponse>;

export const SecureRemotePasswordChallengeParameters = z.object({
    field: z.string()
});
export type SecureRemotePasswordChallengeParameters = z.infer<typeof SecureRemotePasswordChallengeParameters>;

export class SecureRemotePasswordChallengeHandler extends AbstractChallengeHandler<MessageName> {
    protected readonly challengeParameters: SecureRemotePasswordChallengeParameters;
    protected readonly randomBytes: typeof randomBytes;
    protected readonly srpStore: ISrpStore;
    protected readonly srpSessionId: string;

    get handlerType(): EChallengeType {
        return EChallengeType.SecureRemotePassword;
    }

    public constructor(
        conn: PoolConnection,
        session: AuthSession,
        challengeDef: AccountDefAuthChallenge,
        existingMessages: ReadonlyArray<Message<AuthChallengeMessageName>>,
        dependencies: CCHDependencies
    ) {
        AbstractChallengeHandler.validateMessages(messageNames, existingMessages);
        super(conn, session, challengeDef, existingMessages);
        this.challengeParameters =
            SecureRemotePasswordChallengeParameters.parse(
                challengeDef.challengeParameters
            );
        this.randomBytes = dependencies.randomBytes || randomBytes;
        this.srpStore = dependencies.srpStore || InMemorySrpStore.instance;
        this.srpSessionId = `${session.sessionId}.${challengeDef.challengeId}`;
    }

    protected async draftNextMessage(
        processedMessages: MessageDictionary<MessageName>,
    ): Promise<MessageDraft<MessageName> | null> {
        if (processedMessages.VerifyServerProof) {
            return null;
        }
        if (processedMessages.ComputeClientProof) {
            const srpServer = await SrpServer.load(
                this.srpSessionId,
                this.srpStore
            );
            releaseAssert(srpServer.getA() instanceof ArrayBuffer);
            const valueM2 = await srpServer.computeM2();
            const initialMessageDraft = {
                messageName: "VerifyServerProof" as const,
                request: new Uint8Array(valueM2)
                    // @ts-ignore: Node 25+
                    .toBase64(),
                expected: true,
            };
            return initialMessageDraft;
        }
        if (processedMessages.ExchangePublicKeys) {
            const srpServer = await SrpServer.load(
                this.srpSessionId,
                this.srpStore
            );
            releaseAssert(srpServer.getA() instanceof ArrayBuffer);
            const saltBuf = srpServer.getSalt();
            const expectedValueM1 = await srpServer.computeExpectedM1();
            const initialMessageDraft = {
                messageName: "ComputeClientProof" as const,
                request: new Uint8Array(saltBuf)
                    // @ts-ignore: Node 25+
                    .toBase64(),
                expected: new Uint8Array(expectedValueM1)
                    // @ts-ignore: Node 25+
                    .toBase64(),
            };
            return initialMessageDraft;
        }
        if (processedMessages.SelectSecurePassword) {
            const identityField = processedMessages.SelectSecurePassword?.expected;
            releaseAssert(identityField);
            releaseAssert(typeof identityField === "string");
            const srpField = await AccountField.getField(
                this.challengeDef.accountVersion,
                this.session.accountId,
                this.challengeParameters.field
            );
            const { salt, verifier } = await SrpFieldValue.parseAsync(srpField.fieldValue);
            const saltBuf = Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(salt).buffer;
            const verifierBuf = Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(verifier).buffer;
            const srpServer = await SrpServer.create(
                this.srpSessionId,
                4096,
                identityField,
                saltBuf,
                verifierBuf,
                this.randomBytes,
                this.srpStore
            );
            const valueB = await srpServer.computeB();
            await srpServer.save();
            const initialMessageDraft = {
                messageName: "ExchangePublicKeys" as const,
                request: new Uint8Array(valueB)
                    // @ts-ignore: Node 25+
                    .toBase64(),
                expected: null,
            };
            return initialMessageDraft;
        }
        const srpDefField = await AccountDefField.getField(
            this.challengeDef.accountVersion,
            this.challengeParameters.field,
        );
        const srpFieldOptions = await SrpFieldOptions.parseAsync(
            srpDefField.fieldOptions
        );
        const identityField = await AccountField.getField(
            this.challengeDef.accountVersion,
            this.session.accountId,
            srpFieldOptions.identityFieldId,
            this.conn
        );
        const expected: SelectSecurePasswordExpected =
            await SelectSecurePasswordExpected.parseAsync(
                identityField.fieldValue
            );
        const initialMessageDraft = {
            messageName: "SelectSecurePassword" as const,
            request: null,
            expected,
        };
        return initialMessageDraft;
    }

    protected async validateMessageResponse(
        processedMessages: MessageDictionary<MessageName>,
        pendingMessage: Message<MessageName>,
        response: InputMessagePayload
    ) {
        switch (pendingMessage.messageName) {
            case "SelectSecurePassword": {
                const expected: SelectSecurePasswordExpected =
                    await SelectSecurePasswordExpected.parseAsync(
                        pendingMessage.expected
                    );
                // response should be a key field input
                const providedValue =
                    await SelectSecurePasswordResponse.parseAsync(response);
                if (expected !== providedValue) {
                    return `Error:IdentityFieldMismatch` as const;
                }
                return `Ok` as const;
            }
            case "ExchangePublicKeys": {
                const valueA =
                    await ExchangePublicKeysResponse.parseAsync(response);
                const srpServer = await SrpServer.load(
                    this.srpSessionId,
                    this.srpStore
                );
                await srpServer.setA(await valueA.arrayBuffer());
                await srpServer.save();
                return `Ok` as const;
            }
            case "ComputeClientProof": {
                const valueM1 =
                    await ComputeClientProofResponse.parseAsync(response);
                const srpServer = await SrpServer.load(
                    this.srpSessionId,
                    this.srpStore
                );
                releaseAssert(srpServer.getA() instanceof ArrayBuffer);
                try {
                    await srpServer.checkM1(await valueM1.arrayBuffer());
                } catch (e) {
                    if (e instanceof InvalidClientProofError) {
                        return `Error:InvalidClientProof` as const
                    }
                    throw e;
                }
                return `Ok` as const;
            }
            case "VerifyServerProof": {
                await z.boolean().parseAsync(response);
                if (response === true) {
                    return `Ok` as const;
                }
                return `Error:ClientServerProofValidationFailed` as const;
            }
            default:
                throw new Error("Invalid message response; message name not recognised");
        }
    }

    protected async transformResponseForStorage(
        pendingMessage: Message<MessageName>,
        response: InputMessagePayload
    ): Promise<MessagePayload> {
        switch (pendingMessage.messageName) {
            case "SelectSecurePassword": {
                const identityField =
                    response as SelectSecurePasswordResponse;
                return identityField;
            }
            case "ExchangePublicKeys":
            case "ComputeClientProof": {
                const blob: Blob =
                    response as ExchangePublicKeysResponse | ComputeClientProofResponse;
                const base64 = (await blob.bytes())
                    // @ts-ignore: Node 25+
                    .toBase64();
                return base64;
            }
            case "VerifyServerProof": {
                releaseAssert(typeof response === "boolean");
                return response;
            }
            default:
                throw new Error(
                    "Invalid message response; message name not recognised"
                );
        }
    }
}