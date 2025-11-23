import { toPostgresBigInt, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { AuthSession } from "./AuthSession.js";
import { AccountDefAuthChallenge } from "./AccountDefAuthChallenge.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { AuthError } from "../errors/AuthError.js";

export enum EAuthOutcome {
    Pending = 'Pending',
    Aborted = 'Aborted',
    Succeeded = 'Succeeded'
}

interface PGetOutcome {
    outcome: EAuthOutcome;
}

export class AuthChallenge {
    readonly sessionId: BigInt;
    readonly challengeId: string;
    readonly challengeDef: AccountDefAuthChallenge;
    outcome: EAuthOutcome;

    constructor(
        sessionId: BigInt,
        challengeId: string,
        challengeDef: AccountDefAuthChallenge,
        outcome: EAuthOutcome
    ) {
        this.sessionId = sessionId;
        this.challengeId = challengeId;
        this.challengeDef = challengeDef;
        this.outcome = outcome;
    }

    static async getChallenge(
        sessionId: BigInt,
        challengeId: string,
        existingConn?: PoolConnection
    ): Promise<AuthChallenge> {
        if (existingConn) {
            return AuthChallenge.#getChallenge(
                existingConn,
                sessionId,
                challengeId
            );
        }
        return useConnection(conn =>
            AuthChallenge.#getChallenge(
                conn,
                sessionId,
                challengeId,
            )
        );
    }

    static async #getChallenge(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string,
    ) {
        const res = await conn<PGetOutcome[]>`
            SELECT Outcome FROM berytus_account_auth_challenge
            WHERE SessionID = ${toPostgresBigInt(sessionId)}
            AND ChallengeID = ${challengeId}
        `
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AuthChallenge.name,
                `${sessionId},${challengeId}`,
                "SessionID,ChallengeID",
            )
        }
        const challengeDef = await AuthChallenge.#getAuthChallengeDef(
            conn,
            sessionId,
            challengeId,
        );
        return new AuthChallenge(
            sessionId,
            challengeId,
            challengeDef,
            res[0].outcome
        );
    }

    static async createChallenge(
        sessionId: BigInt,
        challengeId: string,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AuthChallenge.#createChallenge(
                existingConn,
                sessionId,
                challengeId
            );
        }
        return useConnection(conn =>
            AuthChallenge.#createChallenge(
                conn,
                sessionId,
                challengeId,
            )
        );
    }

    static async #createChallenge(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string,
    ): Promise<AuthChallenge> {
        /**
         * Check if (challengeId, accountVersion) is valid.
         * Our current DB schema does not have a Foreign Key Constraint
         * on ChallengeID since the AccountVersion is not coupled with it.
         * We check for it here by first retrieving the AccountVersion
         * from the passed SessionID and use it to check if the ChallengeID
         * exists within the AccountVersion.
         */
        let challengeDef: AccountDefAuthChallenge;
        try {
            challengeDef = await AuthChallenge.#getAuthChallengeDef(
                conn,
                sessionId,
                challengeId,
            );
        } catch (e) {
            if (!(e instanceof EntityNotFoundError)) {
                throw e;
            }
            throw EntityNotFoundError.from(
                e,
                `Cannot create auth challenge. ` +
                `Passed ${e.keyName}:${e.key} is invalid.`
            );
        }
        /* now create the record */
        const result = await conn`
            INSERT INTO berytus_account_auth_challenge
            (SessionID, ChallengeID, Outcome)
            SELECT ${toPostgresBigInt(sessionId)},
                   ${challengeId},
                   ${EAuthOutcome.Pending}
            FROM berytus_account_auth_session s
            WHERE s.SessionID = ${toPostgresBigInt(sessionId)}
            AND   s.Outcome = ${EAuthOutcome.Pending}
            FOR UPDATE
        `;
        if (result.count === 0) {
            throw new AuthError(
                `Cannot create a new ${challengeDef.challengeId} `
                + `challenge, auth session#${sessionId} is not pending`
            );
        }
        return new AuthChallenge(
            sessionId,
            challengeId,
            challengeDef,
            EAuthOutcome.Pending
        );
    }

    static async #getAuthChallengeDef(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string
    ) {
        const authSession = await AuthSession.getSession(sessionId, conn);
        const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
                challengeId,
                authSession.accountVersion,
                conn
            );
        return challengeDef;
    }

    async updateOutcome(
        outcome: EAuthOutcome,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return this.#updateOutcome(existingConn, outcome);
        }
        return useConnection(async conn =>
            this.#updateOutcome(conn, outcome)
        );
    }

    async #updateOutcome(
        conn: PoolConnection,
        outcome: EAuthOutcome
    ) {
        if (outcome === EAuthOutcome.Pending) {
            throw new AuthError(
                `Cannot update ${this.challengeId} challenge outcome. `
                + `Refusing to update to default outcome of Pending.`
            )
        }
        const res = await conn`
            UPDATE berytus_account_auth_challenge
            SET Outcome = ${outcome}
            WHERE SessionID = ${toPostgresBigInt(this.sessionId)}
            AND ChallengeID = ${this.challengeId}
            AND Outcome = ${EAuthOutcome.Pending}
        `;
        if (res.count === 0) {
            throw new AuthError(
                `Cannot update ${this.challengeId} challenge outcome. `
                + `Challenge either does not exist anymore or is not in a pending state.`
            );
        }
        this.outcome = outcome;
    }
}