import { EntityNotFoundError } from "@root/backend/db/errors/EntityNotFoundError.js";
import { AccountDefAuthChallenge } from "@root/backend/db/models/AccountDefAuthChallenge.js";
import { AuthChallenge, EAuthOutcome } from "@root/backend/db/models/AuthChallenge.js";
import { AuthSession } from "@root/backend/db/models/AuthSession.js";
import { UserError } from "@root/backend/errors/UserError.js";
import { debugAssert } from "@root/backend/utils/assert.js";

export const validatePendingSessionState = async (
    sessionId: BigInt
): Promise<ValidPendingSessionToken> => {
    try {
        const session = await AuthSession.getSession(
            sessionId
        );
        if (session.outcome !== EAuthOutcome.Pending) {
            throw new UserError(
                `Illegal state. Session is not in a pending state.`
            );
        }
        return new ValidPendingSessionToken(session);
    } catch (e) {
         if (!(e instanceof EntityNotFoundError)) {
            throw e;
        }
        throw new UserError(
            `Session identifier passed for ${e.entityType} does not exist`,
            { cause: e }
        );
    }
}

class ValidPendingSessionToken {
    #session: AuthSession;

    constructor(session: AuthSession) {
        debugAssert(
            assert => assert(
                session.outcome === EAuthOutcome.Pending,
                "session.outcome === EAuthOutcome.Pending"
            )
        );
        this.#session = session;
    }

    get sessionId() {
        return this.#session.sessionId;
    }

    get accountVersion() {
        return this.#session.accountVersion;
    }

    get outcome() {
        return this.#session.outcome;
    }
}

export const validatePendingChallengeState = async (
    sessionToken: ValidPendingSessionToken,
    challengeId: string
) => {
    debugAssert(
        assert => assert(
            sessionToken.outcome === EAuthOutcome.Pending,
            "sessionToken.outcome === EAuthOutcome.Pending"
        )
    );
    try {
        const challenge = await AuthChallenge.getChallenge(
            sessionToken.sessionId,
            challengeId
        );
        if (challenge.outcome !== EAuthOutcome.Pending) {
            throw new UserError(
                `Illegal state. Challenge is not in a pending state.`
            );
        }
        return challenge;
    } catch (e) {
        if (!(e instanceof EntityNotFoundError)) {
            throw e;
        }
        throw new UserError(
            `Identifier passed for ${e.entityType} does not exist`,
            { cause: e }
        );
    }
}

export const validateNewChallengeState = async (
    sessionToken: ValidPendingSessionToken,
    challengeId: string
) => {
    try {
        await AuthChallenge.getChallenge(
            sessionToken.sessionId,
            challengeId
        );
        throw new UserError(
            `Illegal state. Challenge '${challengeId}' already exists.`
        );
    } catch (e) {
        if (e instanceof EntityNotFoundError) {
            return;
        }
        throw e;
    }
}

export const validateChallengeDefExists = async (
    sessionToken: ValidPendingSessionToken,
    challengeId: string
) => {
    debugAssert(
        assert => assert(
            sessionToken.outcome === EAuthOutcome.Pending,
            "sessionToken.outcome === EAuthOutcome.Pending"
        )
    );
    try {
        await AccountDefAuthChallenge.getChallengeDef(
            challengeId,
            sessionToken.accountVersion
        );
    } catch (e) {
        if (e instanceof UserError) {
            throw new UserError(
                `Challenge ID '${challengeId}' is not defined `
                + `unser Session ID `
                + `'${sessionToken.sessionId}'`
            )
        }
    }
}
