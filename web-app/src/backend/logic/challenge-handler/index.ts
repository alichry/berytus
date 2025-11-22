import { AccountDefAuthChallenge, EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge";
import { PasswordChallengeHandler } from "./PasswordChallengeHandler";
import { AuthSession } from "@root/backend/db/models/AuthSession";
import { AbstractChallengeHandler } from "./AbstractChallengeHandler";
import { DigitalSignatureChallengeHandler } from "./DigitalSignatureChallengeHandler";

export const initiateChallenge = async (
    sessionId: BigInt,
    challengeId: string
) => {
    const authSession = await AuthSession.getSession(sessionId);
    const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
        challengeId,
        authSession.accountVersion
    );
    const handlerCtr = getHandlerCtor(challengeDef.challengeType);
    return AbstractChallengeHandler.initiateChallenge(
        sessionId,
        challengeId,
        handlerCtr
    );
}

export const loadChallenge = async (
    sessionId: BigInt,
    challengeId: string
) => {
    const authSession = await AuthSession.getSession(sessionId);
    const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
        challengeId,
        authSession.accountVersion
    );
    const handlerCtr = getHandlerCtor(challengeDef.challengeType);
    return AbstractChallengeHandler.loadChallenge(
        sessionId,
        challengeId,
        handlerCtr
    );
}

const getHandlerCtor = (challengeType: EChallengeType) => {
    switch (challengeType) {
        case EChallengeType.Password:
            return PasswordChallengeHandler;
        case EChallengeType.DigitalSignature:
            return DigitalSignatureChallengeHandler;
        default:
            throw new Error("Could not determine challenge handler");
    }
}