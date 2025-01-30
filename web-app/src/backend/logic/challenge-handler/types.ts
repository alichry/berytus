import type { EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge";
import type { AuthChallenge } from "@root/backend/db/models/AuthChallenge";
import type { AuthChallengeMessage, AuthChallengeMessageName, ChallengeMessageStatus, MessagePayload } from "@root/backend/db/models/AuthChallengeMessage";

export type InitiateAuthChallengeResullt = {
    challenge: AuthChallenge;
    initialMessage: AuthChallengeMessage;
}

export type ProcessChallengeMessageResult = {
    nextMessage: AuthChallengeMessage;
}

export interface ChallengePendingMessage<MN extends AuthChallengeMessageName = AuthChallengeMessageName> {
    messageName: MN;
    request: MessagePayload;
    expected: MessagePayload;
}

export interface IChallengeHandler {
    challenge: AuthChallenge;

    getPendingMessage(): Promise<ChallengePendingMessage | null>;

    processPendingMessageResponse(
        response: MessagePayload
    ): Promise<NonNullable<ChallengeMessageStatus>>;
};