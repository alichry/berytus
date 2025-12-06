import { AccountDefAuthChallenge, EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";
import { PasswordChallengeHandler } from "./PasswordChallengeHandler.js";
import { AuthSession } from "@root/backend/db/models/AuthSession.js";
import { AbstractChallengeHandler, type CCHDependencies } from "./AbstractChallengeHandler.js";
import { DigitalSignatureChallengeHandler } from "./DigitalSignatureChallengeHandler.js";
import type { PoolConnection } from "@root/backend/db/pool.js";
export const setupChallenge = async (
    sessionId: BigInt,
    challengeId: string,
    conn?: PoolConnection,
    dependencies?: CCHDependencies
) => {
    const authSession = await AuthSession.getSession(sessionId, conn);
    const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
        challengeId,
        authSession.accountVersion,
        conn
    );
    const handlerCtr = getHandlerCtor(challengeDef.challengeType);
    return AbstractChallengeHandler.setupChallenge(
        sessionId,
        challengeId,
        handlerCtr,
        conn,
        dependencies
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