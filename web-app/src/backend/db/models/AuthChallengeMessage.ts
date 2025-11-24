/// <reference path="../../../../src/generated/berytus.d.ts" />
import { toPostgresBigInt, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { EChallengeType } from "./AccountDefAuthChallenge.js";
import { AuthChallenge, EAuthOutcome } from "./AuthChallenge.js";
import type { JSONValue } from "../types.js";
import { AuthSession } from "./AuthSession.js";
import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";
import { debugAssert } from "@root/backend/utils/release-assert.js";
import { AuthError } from "../errors/AuthError.js";

export type MessagePayload = JSONValue;
export type AuthChallengeMessageName =
    BerytusPasswordChallengeMessageName |
    BerytusSecureRemotePasswordChallengeMessageName |
    BerytusDigitalSignatureChallengeMessageName |
    BerytusOffChannelOtpChallengeMessageName;

const ValidMessages: Record<
    EChallengeType,
    AuthChallengeMessageName[]
> = {
    "Password": ["GetPasswordFields"],
    "SecureRemotePassword": [
        "SelectSecurePassword",
        "ExchangePublicKeys",
        "ComputeClientProof",
        "VerifyServerProof"
    ],
    "DigitalSignature": ["SelectKey", 'SignNonce'],
    "OffChannelOtp": ['GetOtp']
}

export type ChallengeMessageStatus = "Ok" | `Error:${string}` | null;

interface PGetChallengeMessage {
    messagename: AuthChallengeMessageName;
    request: JSONValue;
    expected: JSONValue;
    response: JSONValue;
    statusmsg: ChallengeMessageStatus
}

interface PUpdateResponseAndStatus {
    messageupdated: boolean;
    challengeaborted: boolean;
}

export class AuthChallengeMessage {
    readonly sessionId: BigInt;
    readonly challengeId: string;
    readonly messageName: AuthChallengeMessageName;
    readonly request: MessagePayload;
    readonly expected: MessagePayload;
    response: MessagePayload;
    statusMsg: ChallengeMessageStatus;

    constructor(
        sessionId: BigInt,
        challengeId: string,
        messageName: AuthChallengeMessageName,
        request: MessagePayload,
        expected: MessagePayload,
        response: MessagePayload,
        statusMsg: ChallengeMessageStatus
    ) {
        this.sessionId = sessionId;
        this.challengeId = challengeId;
        this.messageName = messageName;
        this.request = request;
        this.expected = expected;
        this.response = response;
        this.statusMsg = statusMsg;
    }

    static async getAllMessages(
        sessionId: BigInt,
        challengeId: string,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return AuthChallengeMessage.#getAllMessages(
                existingConn,
                sessionId,
                challengeId
            );
        }
        return useConnection(conn =>
            AuthChallengeMessage.#getAllMessages(
                conn,
                sessionId,
                challengeId
            )
        );
    }

    static async #getAllMessages(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string
    ): Promise<AuthChallengeMessage[]> {
        const res = await conn<PGetChallengeMessage[]>`
            SELECT MessageName,
                   Request,
                   Expected,
                   Response,
                   StatusMsg
            FROM berytus_account_auth_challenge_message
            WHERE SessionID = ${toPostgresBigInt(sessionId)}
            AND ChallengeID = ${challengeId}
            ORDER BY CreatedAt ASC
        `;
        return res.map(packet =>
            new AuthChallengeMessage(
                sessionId,
                challengeId,
                packet.messagename,
                packet.request,
                packet.expected,
                packet.response,
                packet.statusmsg
            )
        );
    }

    static async createMessage(
        sessionId: BigInt,
        challengeId: string,
        messageName: AuthChallengeMessageName,
        request: MessagePayload,
        expected: MessagePayload,
        response: MessagePayload,
        existingConn?: PoolConnection
    ): Promise<AuthChallengeMessage> {
        if (existingConn) {
            return AuthChallengeMessage.#createMessage(
                existingConn,
                sessionId,
                challengeId,
                messageName,
                request,
                expected,
                response
            );
        }
        return useConnection((conn) =>
            AuthChallengeMessage.#createMessage(
                conn,
                sessionId,
                challengeId,
                messageName,
                request,
                expected,
                response
            )
        );
    }

    static async #createMessage(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string,
        messageName: AuthChallengeMessageName,
        request: MessagePayload,
        expected: MessagePayload,
        response: MessagePayload
    ) {
        const session = await AuthSession.getSession(sessionId, conn);
        if (session.outcome !== EAuthOutcome.Pending) {
            throw new AuthError(
                `Cannot create message. Auth session is NOT in pending state, rather it is in ${session.outcome} state`
            );
        }
        const challenge = (await AuthChallenge.getChallenge(
            sessionId,
            challengeId,
            conn
        ));
        if (challenge.outcome !== EAuthOutcome.Pending) {
            throw new AuthError(
                `Cannot create message. Auth challenge is NOT in pending state, rather it is in ${challenge.outcome} state`
            );
        }
        const challengeType = challenge.challengeDef.challengeType;
        const messageDefs = AuthChallengeMessage.getMessageDefs(challengeType);
        const messageDefIndex = messageDefs.indexOf(messageName);
        if (-1 === messageDefIndex) {
            throw new InvalidArgError(
                "Cannot create message. Passed message name " +
                `'${messageName}' is not appropriatet for the challenge.`
            );
        }
        const previousMessages = messageDefs.slice(0, messageDefIndex);
        const nextMessages = messageDefs.slice(messageDefIndex);
        debugAssert(
            assert => assert(previousMessages.length + nextMessages.length === messageDefs.length)
        );
        // lock session, challenge, and every previous ok message.
        const result = await conn`
            WITH cte_pending_session AS (
                SELECT FROM berytus_account_auth_session
                WHERE SessionID = ${toPostgresBigInt(sessionId)}
                AND Outcome = ${EAuthOutcome.Pending}
                FOR UPDATE
            ), cte_pending_challenge AS (
                SELECT FROM berytus_account_auth_challenge
                WHERE SessionID = ${toPostgresBigInt(sessionId)}
                AND   ChallengeID = ${challengeId}
                AND   Outcome = ${EAuthOutcome.Pending}
                FOR UPDATE
            ), cte_previous_ok_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, StatusMsg
                FROM berytus_account_auth_challenge_message
                WHERE SessionID = ${toPostgresBigInt(sessionId)}
                AND ChallengeID = ${challengeId}
                AND StatusMsg = 'Ok'
                ORDER BY CreatedAt ASC
                FOR UPDATE
            ),
            cte_previous_ok_messages_agg AS (
                SELECT c.SessionID, c.ChallengeID,
                       COALESCE(
                           jsonb_agg(m.MessageName) FILTER (WHERE m.MessageName IS NOT NULL),
                           '[]'::jsonb
                       ) AS MessageNames,
                       'Ok' AS StatusMsg
                FROM berytus_account_auth_challenge AS c
                LEFT JOIN cte_previous_ok_messages AS m
                ON m.SessionID = c.SessionID
                AND m.ChallengeID = c.ChallengeID
                WHERE c.SessionID = ${toPostgresBigInt(sessionId)}
                AND c.ChallengeID = ${challengeId}
                GROUP BY c.SessionID, c.ChallengeID
            )
            INSERT INTO berytus_account_auth_challenge_message
            (SessionID, ChallengeID, MessageName,
            Request, Expected, Response)
            SELECT ${toPostgresBigInt(sessionId)}, ${challengeId}, ${messageName},
                    ${conn.json(request)}, ${conn.json(expected)},
                    ${conn.json(response)}
            WHERE (
                SELECT TRUE FROM cte_pending_session
            ) AND (
                SELECT TRUE FROM cte_pending_challenge
            ) AND (
                SELECT TRUE FROM cte_previous_ok_messages_agg
                WHERE MessageNames = ${conn.json(previousMessages)}
            )
        `;
        // TODO(berytus): Minor -- catch any PostgresError regarding
        // duplicates and wrap it in an AuthError.
        if (result.count === 0) {
            throw new AuthError(
                'Cannot create message. Challenge message integrity check failed.'
            );
        }
        return new AuthChallengeMessage(
            sessionId,
            challengeId,
            messageName,
            request,
            expected,
            response,
            null
        );
    }

    async updateResponseAndStatus(
        response: MessagePayload,
        statusMsg: NonNullable<ChallengeMessageStatus>,
        existingConn?: PoolConnection
    ): Promise<void> {
        if (existingConn) {
            return this.#updateResponseAndStatus(
                existingConn,
                response,
                statusMsg
            );
        }
        return useConnection(async conn =>
            this.#updateResponseAndStatus(
                conn,
                response,
                statusMsg
            )
        );
    }

    async #updateResponseAndStatus(
        conn: PoolConnection,
        response: MessagePayload,
        statusMsg: "Ok" | `Error:${string}`,
    ) {
        if (
            typeof statusMsg !== "string" || (
                statusMsg !== 'Ok'
                && (
                    ! statusMsg.startsWith('Error:')
                    || statusMsg.slice(6).trim().length === 0
                )
            )
        ) {
            throw new InvalidArgError(
                "statusMsg is malformed"
            );
        }
        if (this.statusMsg !== null) {
            throw new InvalidArgError(
                'statusMsg is already set; Refusing to update message status'
            );
        }
        // lock session, lock challenge.
        const result = await conn<PUpdateResponseAndStatus[]>`
            WITH cte_pending_session AS (
                SELECT SessionID, Outcome
                FROM   berytus_account_auth_session
                WHERE  SessionID = ${toPostgresBigInt(this.sessionId)}
                AND    Outcome = ${EAuthOutcome.Pending}
                FOR UPDATE
            ), cte_pending_challenge AS (
                SELECT SessionID, ChallengeID, Outcome
                FROM berytus_account_auth_challenge
                WHERE SessionID   = ${toPostgresBigInt(this.sessionId)}
                AND   ChallengeID = ${this.challengeId}
                AND   Outcome = ${EAuthOutcome.Pending}
                AND   (SELECT TRUE FROM cte_pending_session)
                FOR UPDATE
            ), cte_do_update_message AS (
                UPDATE berytus_account_auth_challenge_message
                SET Response = ${conn.json(response)},
                    StatusMsg = ${statusMsg}
                WHERE SessionID = ${toPostgresBigInt(this.sessionId)}
                AND   ChallengeID = ${this.challengeId}
                AND   MessageName = ${this.messageName}
                AND   StatusMsg IS NULL
                AND   (SELECT TRUE FROM cte_pending_challenge)
                RETURNING TRUE AS MessageUpdated
            ), cte_maybe_abort_challenge AS (
                UPDATE berytus_account_auth_challenge
                SET   Outcome = 'Aborted'
                WHERE (SELECT MessageUpdated FROM cte_do_update_message)
                AND   'Ok' <> ${statusMsg}
                AND   SessionID = ${toPostgresBigInt(this.sessionId)}
                AND   ChallengeID = ${this.challengeId}
                AND   Outcome = 'Pending'
                RETURNING TRUE AS ChallengeAborted
            )
            SELECT
                COALESCE(
                    (SELECT MessageUpdated FROM cte_do_update_message),
                    FALSE
                ) AS MessageUpdated,
                COALESCE(
                    (SELECT ChallengeAborted FROM cte_maybe_abort_challenge),
                    FALSE
                ) AS ChallengeAborted
        `;
        debugAssert((assert: (v: unknown, msg?: string) => void) => {
            assert(result.length === 1, "result.length === 1");
            if (! result[0].messageupdated) {
                assert(! result[0].challengeaborted, "! result[0].challengeaborted");
                return;
            }
            if (statusMsg !== 'Ok') {
                assert(result[0].challengeaborted, "result[0].challengeaborted");
                return;
            }
            assert(! result[0].challengeaborted, "! result[0].challengeaborted");
        });
        if (! result[0].messageupdated) {
            throw new AuthError(
                'Cannot update response and status. '
                + 'Integrity validation failed'
            );
        }
        this.statusMsg = statusMsg;
        this.response = response;
    }

    static getMessageDefs(challengeType: EChallengeType): AuthChallengeMessageName[] {
        debugAssert(assert => assert(challengeType in ValidMessages));
        if (!(challengeType in ValidMessages)) {
            throw new InvalidArgError(
                `Failed to retrieve message definitions `
                + `for challenge type "${challengeType}". `
                + `Reason: Unrecognised challenge type.`
            );
        }
        return ValidMessages[challengeType];
    }
}