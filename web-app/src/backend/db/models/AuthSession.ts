import type { PoolConnection, ResultSetHeader, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";
import { AccountDefAuthChallenge } from "./AccountDefAuthChallenge";
import { AuthChallenge, EAuthOutcome } from "./AuthChallenge";
import { AuthError } from "../errors/AuthError";

interface PGetSession extends RowDataPacket {
    SessionID: number;
    AccountID: number;
    AccountVersion: number;
    Outcome: EAuthOutcome;
}

export class AuthSession {
    public readonly sessionId: number;
    public readonly accountId: number;
    public readonly accountVersion: number;
    public outcome: EAuthOutcome;

    constructor(
        sessionId: number,
        accountId: number,
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
        await conn.query(
            'UPDATE berytus_account_auth_session ' +
            'SET Outcome = ? ' +
            'WHERE SessionID = ?',
            [EAuthOutcome.Succeeded, this.sessionId]
        );
    }

    static async getSession(
        sessionId: number,
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
        sessionId: number,
    ) {
        const [res] = await conn.query<PGetSession[]>(
            'SELECT SessionID, AccountID, AccountVersion, Outcome FROM ' +
            'berytus_account_auth_session WHERE SessionID = ?',
            [sessionId]
        );
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                AuthSession.name,
                "SessionID",
                String(sessionId)
            );
        }
        return new AuthSession(
            sessionId,
            res[0].AccountID,
            res[0].AccountVersion,
            res[0].Outcome,
        );
    }

    static async createSession(
        accountId: number,
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
        accountId: number,
        accountVersion: number,
        conn: PoolConnection
    ): Promise<AuthSession> {
        const [res] = await conn.query<ResultSetHeader>(
            'INSERT INTO berytus_account_auth_session ' +
            '(AccountID, AccountVersion, Outcome) VALUES (?, ?, ?)',
            [accountId, accountVersion, EAuthOutcome.Pending]
        );
        return new AuthSession(res.insertId, accountId, accountVersion, EAuthOutcome.Pending);
    }
}