/// <reference path="../../../../src/generated/berytus.d.ts" />
import { toPostgresBigInt, useConnection } from "../pool.js";
import type { PoolConnection } from "../pool.js";
import { EChallengeType } from "./AccountDefAuthChallenge.js";
import { AuthChallenge, EAuthOutcome } from "./AuthChallenge.js";
import type { JSONValue } from "../types.js";
import { AuthSession } from "./AuthSession.js";

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
    "SecureRemotePassword": ["ExchangePublicKeys", "ComputeClientProof", "VerifyServerProof"],
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
        // TODO(berytus): have a think about concurrency
        const session = await AuthSession.getSession(sessionId, conn);
        if (session.outcome !== EAuthOutcome.Pending) {
            throw new Error(
                `Cannot create message. Auth session is NOT in pending state, rather it is in ${session.outcome} state`
            );
        }
        const challenge = (await AuthChallenge.getChallenge(
            sessionId,
            challengeId,
            conn
        ));
        if (challenge.outcome !== EAuthOutcome.Pending) {
        throw new Error(
                `Cannot create message. Auth challenge is NOT in pending state, rather it is in ${challenge.outcome} state`
            );
        }
        const challengeType = challenge.challengeDef.challengeType;
        if (-1 === ValidMessages[challengeType].indexOf(messageName)) {
            throw new Error(
                "Cannot create message. Passed message name " +
                `'${messageName}' is not appropriatet for the challenge.`
            );
        }
        // TODO(berytus): We should also validate
        // that the message name is the expected next message name.
        await conn`
            INSERT INTO berytus_account_auth_challenge_message
            (SessionID, ChallengeID, MessageName, Request, Expected, Response)
            VALUES (${toPostgresBigInt(sessionId)}, ${challengeId}, ${messageName},
                    ${conn.json(request)}, ${conn.json(expected)},
                    ${conn.json(response)})
        `;
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
        // todo: use transaction
        await conn`
            UPDATE berytus_account_auth_challenge_message
            SET Response = ${conn.json(response)},
                StatusMsg = ${statusMsg}
            WHERE SessionID = ${toPostgresBigInt(this.sessionId)}
            AND   ChallengeID = ${this.challengeId}
            AND   MessageName = ${this.messageName}
        `;
        if (statusMsg !== 'Ok') {
            await conn`
                UPDATE berytus_account_auth_challenge SET Outcome = 'Aborted'
                WHERE SessionID = ${toPostgresBigInt(this.sessionId)}
                AND   ChallengeID = ${this.challengeId}
                AND   Outcome = 'Pending'
            `;
        }
        this.statusMsg = statusMsg;
        this.response = response;
    }
}