import { toPostgresBigInt, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { AccountDefAuthChallenge } from "./AccountDefAuthChallenge.js";
import { AuthChallenge, EAuthOutcome } from "./AuthChallenge.js";
import { AuthError } from "../errors/AuthError.js";

interface PCreateSession {
    sessionid: BigInt;
}

interface PGetSession {
    sessionid: BigInt;
    accountid: BigInt;
    accountversion: number;
    outcome: EAuthOutcome;
}

export class AuthSession {
    public readonly sessionId: BigInt;
    public readonly accountId: BigInt;
    public readonly accountVersion: number;
    public outcome: EAuthOutcome;

    constructor(
        sessionId: BigInt,
        accountId: BigInt,
        accountVersion: number,
        outcome: EAuthOutcome
    ) {
        this.sessionId = sessionId;
        this.accountId = accountId;
        this.accountVersion = accountVersion;
        this.outcome = outcome;
    }

    async finish(
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return this.#finish(existingConn);
        }
        return useConnection(conn => this.#finish(conn));
    }

    async #finish(
        conn: PoolConnection
    ) {
        if (this.outcome !== EAuthOutcome.Pending) {
            throw new Error(
                "Session is not in a pending state and thus cannot be modified"
            );
        }
        // need to chek the nb of challenges required
        // and check that each is OK'd
        const challengeDefs = await AccountDefAuthChallenge.getAllChallengeDef(
            this.accountVersion,
            conn
        );
        for (let i = 0; i < challengeDefs.length; i++) {
            const ch = await AuthChallenge.getChallenge(
                this.sessionId,
                challengeDefs[i].challengeId,
                conn
            );
            if (ch.outcome === EAuthOutcome.Aborted) {
                throw new AuthError(
                    `Challenge ${ch.challengeId} was not successful. ` +
                    `Cannnot finish auth session.`
                );
            }
            if (ch.outcome === EAuthOutcome.Pending) {
                throw new AuthError(
                    `Challenge ${ch.challengeId} is still pemnding ` +
                    `Cannnot finish auth session.`
                );
            }
        }
        await conn`
            UPDATE berytus_account_auth_session
            SET Outcome = ${EAuthOutcome.Succeeded}
            WHERE SessionID = ${toPostgresBigInt(this.sessionId)}
        `;
    }

    static async getSession(
        sessionId: BigInt,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AuthSession.#getSession(
                existingConn,
                sessionId
            );
        }
        return useConnection((conn) =>
            AuthSession.#getSession(
                conn,
                sessionId,
            )
        );
    }

    static async #getSession(
        conn: PoolConnection,
        sessionId: BigInt,
    ) {
        const res = await conn<PGetSession[]>`
            SELECT SessionID, AccountID,
                   AccountVersion, Outcome
            FROM berytus_account_auth_session
            WHERE SessionID = ${toPostgresBigInt(sessionId)}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AuthSession.name,
                "SessionID",
                String(sessionId)
            );
        }
        return new AuthSession(
            sessionId,
            res[0].accountid,
            res[0].accountversion,
            res[0].outcome,
        );
    }

    static async createSession(
        accountId: BigInt,
        accountVersion: number,
        existingConn?: PoolConnection
    ): Promise<AuthSession> {
        if (existingConn) {
            return AuthSession.#createSession(
                accountId,
                accountVersion,
                existingConn
            );
        }
        return useConnection(conn =>
            AuthSession.#createSession(
                accountId,
                accountVersion,
                conn
            )
        );
    }

    static async #createSession(
        accountId: BigInt,
        accountVersion: number,
        conn: PoolConnection
    ): Promise<AuthSession> {
        const res = await conn<PCreateSession[]>`
            INSERT INTO berytus_account_auth_session
            (AccountID, AccountVersion, Outcome)
            VALUES (${toPostgresBigInt(accountId)}, ${accountVersion}, ${EAuthOutcome.Pending})
            RETURNING SessionID
        `;
        return new AuthSession(res[0].sessionid, accountId, accountVersion, EAuthOutcome.Pending);
    }
}