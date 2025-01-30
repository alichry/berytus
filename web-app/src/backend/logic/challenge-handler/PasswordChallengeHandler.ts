import { EChallengeType, PasswordChallengeParameters } from "@root/backend/db/models/AccountDefAuthChallenge";
import type { MessagePayload } from "../../db/models/AuthChallengeMessage";
import { AbstractChallengeHandler, type MessageDraft, type MessageDictionary, type Message } from "@root/backend/logic/challenge-handler/AbstractChallengeHandler";
import { AccountField } from "@root/backend/db/models/AccountField";
import { z } from "zod";
import { PasswordHandler, StoredPassword } from "../field-handler/PasswordHandler";
import type { PoolConnection } from "mysql2/promise";
import type { AuthSession } from "@root/backend/db/models/AuthSession";
import type { AuthChallenge } from "@root/backend/db/models/AuthChallenge";

type MessageName = BerytusPasswordChallengeMessageName;

const PasswordResponse = z.array(
    z.object({
        id: z.string(),
        value: z.string()
    })
);

const Expected = z.array(
    z.object({
        id: z.string(),
        password: StoredPassword
    })
);

type Expected = z.infer<typeof Expected>;

export class PasswordChallengeHandler extends AbstractChallengeHandler<MessageName> {
    protected challengeParameters: PasswordChallengeParameters;


    get handlerType(): EChallengeType {
        return EChallengeType.Password;
    }

    public constructor(
        conn: PoolConnection,
        session: AuthSession,
        challenge: AuthChallenge
    ) {
        super(conn, session, challenge);
        this.challengeParameters =
            PasswordChallengeParameters.parse(
                challenge.challengeDef.challengeParameters
            );
    }

    protected async draftNextMessage(
        processedMessages: MessageDictionary<MessageName>
    ): Promise<MessageDraft<MessageName> | null> {
        if (processedMessages.GetPasswordFields) {
            return null;
        }
        const expected: Expected = [];
        const { passwordFieldIds } = this.challengeParameters;
        for (let i = 0; i < passwordFieldIds.length; i++) {
            const fieldId = passwordFieldIds[i];
            const field = await AccountField.getField(
                this.challenge.challengeDef.accountVersion,
                this.session.accountId,
                fieldId,
                this.conn
            );
            const storedPassword = await
                StoredPassword.parseAsync(field.fieldValue);
            expected.push({
                id: fieldId,
                password: storedPassword
            });
        }

        const initialMessageDraft = {
            messageName: "GetPasswordFields" as const,
            request: passwordFieldIds,
            expected,
        };
        return initialMessageDraft;
    }

    protected async validateMessageResponse(
        processedMessages: MessageDictionary<MessageName>,
        pendingMessage: Message<MessageName>,
        response: MessagePayload
    ) {
        const expected: Expected = await Expected.parseAsync(
            pendingMessage.expected
        );
        // response should be an array of FieldInput.
        const passwords = await PasswordResponse.parseAsync(response);
        if (passwords.length === 0) {
            throw new Error('Malformed message response.');
        }
        for (let i = 0; i < passwords.length; i++) {
            const { id: fieldId, value: passedValue } = passwords[i];
            const expectedEntry = expected.find(x => x.id === fieldId);
            if (! expectedEntry) {
                throw new Error('Malformed message response.');
            }
            const cmp = await PasswordHandler.compare(
                passedValue,
                expectedEntry.password
            );
            if (! cmp) {
                return `Error:InvalidPassword` as const;
            }
        }
        return `Ok` as const;
    }
}