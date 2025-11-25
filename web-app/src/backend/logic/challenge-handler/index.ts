import { AccountDefAuthChallenge, EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";
import { PasswordChallengeHandler } from "./PasswordChallengeHandler.js";
import { AuthSession } from "@root/backend/db/models/AuthSession.js";
import { AbstractChallengeHandler } from "./AbstractChallengeHandler.js";
import { DigitalSignatureChallengeHandler } from "./DigitalSignatureChallengeHandler.js";

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