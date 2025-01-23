import type { ResultSetHeader, PoolConnection } from "mysql2/promise";
import type { RowDataPacket } from "mysql2";
import { useConnection } from "../pool";
import { AuthSession } from "./AuthSession";
import { AccountDefAuthChallenge } from "./AccountDefAuthChallenge";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";

export enum EAuthOutcome {
    Pending = 'Pending',
    Aborted = 'Aborted',
    Succeeded = 'Succeeded'
}

interface PGetOutcome extends RowDataPacket {
    Outcome: EAuthOutcome;
}

export class AuthChallenge {
    readonly sessionId: number;
    readonly challengeId: string;
    readonly challengeDef: AccountDefAuthChallenge;
    outcome: EAuthOutcome;

    constructor(
        sessionId: number,
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
        sessionId: number,
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
        sessionId: number,
        challengeId: string,
    ) {
        const [res] = await conn.query<PGetOutcome[]>(
            'SELECT Outcome FROM berytus_account_auth_challenge ' +
            'WHERE SessionID = ? AND ChallengeID = ?',
            [sessionId, challengeId]
        )
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
            res[0].Outcome
        );
    }

    static async createChallenge(
        sessionId: number,
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
        sessionId: number,
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
        await conn.query<ResultSetHeader>(
            'INSERT INTO berytus_account_auth_challenge ' +
            '(SessionID, ChallengeID, Outcome) VALUES (?, ?, ?)',
            [sessionId, challengeId, EAuthOutcome.Pending]
        );
        return new AuthChallenge(
            sessionId,
            challengeId,
            challengeDef,
            EAuthOutcome.Pending
        );
    }

    static async #getAuthChallengeDef(
        conn: PoolConnection,
        sessionId: number,
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
        await conn.query(
            'UPDATE berytus_account_auth_challenge ' +
            'SET Outcome = ? ' +
            'WHERE SessionID = ? AND ChallengeID = ?',
            [outcome, this.sessionId, this.challengeId]
        );
        this.outcome = outcome;
    }
}