import {
    AccountDefAuthChallenge,
    EChallengeType
} from "@root/backend/db/models/AccountDefAuthChallenge.js";
import type {
    AuthChallengeMessageName,
    MessagePayload
} from "../../db/models/AuthChallengeMessage";
import {
    AbstractChallengeHandler,
    type MessageDraft,
    type MessageDictionary,
    type Message,
    type CCHDependencies
} from "@root/backend/logic/challenge-handler/AbstractChallengeHandler.js";
import { AccountField } from "@root/backend/db/models/AccountField.js";
import { z } from "zod";
import type { AuthSession } from "@root/backend/db/models/AuthSession";
import {
    PublicKeyFieldInput,
    PublicKeyFieldValue
} from "../field-handler/DigitalSignatureHandler.js";
import { randomBytes } from "crypto";
import { KeyUtils, SignUtils } from "../../utils/key-utils.js";
import type { PoolConnection } from "@root/backend/db/pool";
type MessageName = BerytusDigitalSignatureChallengeMessageName;

const messageNames: ReadonlyArray<MessageName> = [
    "SelectKey", "SignNonce"
];

const SelectKeyExpected = PublicKeyFieldInput;

type SelectKeyExpected = z.infer<typeof SelectKeyExpected>;

const SelectKeyResponse = SelectKeyExpected;

const SignNonceResponse = z.string(); // base64

export const DigitalSignatureChallengeParameters = z.object({
    keyFieldId: z.string()
});
export type DigitalSignatureChallengeParameters = z.infer<typeof DigitalSignatureChallengeParameters>;

export class DigitalSignatureChallengeHandler extends AbstractChallengeHandler<MessageName> {
    protected challengeParameters: DigitalSignatureChallengeParameters;
    protected randomBytes: typeof randomBytes;

    get handlerType(): EChallengeType {
        return EChallengeType.DigitalSignature;
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
            DigitalSignatureChallengeParameters.parse(
                challengeDef.challengeParameters
            );
        this.randomBytes = dependencies.randomBytes || randomBytes;
    }

    protected async draftNextMessage(
        processedMessages: MessageDictionary<MessageName>
    ): Promise<MessageDraft<MessageName> | null> {
        if (processedMessages.SignNonce) {
            return null;
        }
        if (processedMessages.SelectKey) {
            const nonce = this.randomBytes(64);
            const initialMessageDraft = {
                messageName: "SignNonce" as const,
                request: nonce.toString('base64'),
                expected: null,
            };
            return initialMessageDraft;
        }
        const field = await AccountField.getField(
            this.challengeDef.accountVersion,
            this.session.accountId,
            this.challengeParameters.keyFieldId,
            this.conn
        );
        const expected: SelectKeyExpected = {
            id: this.challengeParameters.keyFieldId,
            value: await PublicKeyFieldValue.parseAsync(field.fieldValue)
        }
        const initialMessageDraft = {
            messageName: "SelectKey" as const,
            request: this.challengeParameters.keyFieldId,
            expected,
        };
        return initialMessageDraft;
    }

    protected async validateMessageResponse(
        processedMessages: MessageDictionary<MessageName>,
        pendingMessage: Message<MessageName>,
        response: MessagePayload
    ) {
        switch (pendingMessage.messageName) {
            case "SelectKey": {
                const expected: SelectKeyExpected = await SelectKeyExpected.parseAsync(
                    pendingMessage.expected
                );
                // response should be a key field input
                const { id: fieldId, value: passedValue } = await SelectKeyResponse.parseAsync(response);
                if (expected.id !== fieldId) {
                    throw new Error('Malformed message response.');
                }
                // TODO(berytus): A better way is to compare the stripped
                // base64 data which does not include newlines.
                if (expected.value.publicKey !== passedValue.publicKey) {
                    return `Error:PublicKeyMismatch` as const;
                }
                return `Ok` as const;
            }
            case "SignNonce": {
                const sigBase64 = await SignNonceResponse.parseAsync(response);
                const nonce = Buffer.from(pendingMessage.request as string, 'base64');
                const sig = Buffer.from(sigBase64, 'base64');
                const key = await KeyUtils.importArmoredKeyForVerification(
                    (processedMessages.SelectKey!.expected as SelectKeyExpected).value.publicKey,
                );
                const res = await SignUtils.verify(key, sig, nonce);
                return res
                    ? `Ok` as const
                    : `Error:InvalidSignature` as const;
            }
            default:
                throw new Error("Invalid message response; message name not recognised");
        }
    }
}