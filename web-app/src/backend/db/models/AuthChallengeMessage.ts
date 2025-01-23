import type { PoolConnection, RowDataPacket } from "mysql2/promise";
import { useConnection } from "../pool";
import { AccountDefAuthChallenge, EChallengeType } from "./AccountDefAuthChallenge";
import { AuthChallenge } from "./AuthChallenge";
import { EntityNotFoundError } from "../errors/EntityNotFoundError";
import type { JSONString } from "./types";

export type MessagePayload = object | null | string | number | boolean;
export type AuthChallengeMessageName =
    BerytusPasswordChallengeMessageName |
    BerytusSecureRemotePasswordChallengeMessageName |
    BerytusDigitalSignatureChallengeMessageName |
    BerytusForeignIdentityOtpChallengeMessageName;

const ValidMessages: Record<
    EChallengeType,
    AuthChallengeMessageName[]
> = {
    "Password": ["GetPasswordFields"],
    "SecureRemotePassword": ["ExchangePublicKeys", "ComputeClientProof", "VerifyServerProof"],
    "DigitalSignature": ["GetPublicKey", 'SignNonce'],
    "ForeignIdentityOtp": ['GetOtp']
}

export type ChallengeMessageStatus = "Ok" | `Error:${string}` | null;

interface PGetChallengeMessage extends RowDataPacket {
    MessageName: AuthChallengeMessageName;
    Request: JSONString;
    Expected: JSONString;
    Response: JSONString;
    StatusMsg: ChallengeMessageStatus
}

export class AuthChallengeMessage {
    readonly sessionId: number;
    readonly challengeId: string;
    readonly messageName: AuthChallengeMessageName;
    readonly request: MessagePayload;
    readonly expected: MessagePayload;
    response: MessagePayload;
    statusMsg: ChallengeMessageStatus;

    constructor(
        sessionId: number,
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
        sessionId: number,
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
        sessionId: number,
        challengeId: string
    ): Promise<AuthChallengeMessage[]> {
        const [res] = await conn.query<PGetChallengeMessage[]>(
            'SELECT MessageName, Request, Expected, Response, StatusMsg ' +
            'FROM berytus_account_auth_challenge_message ' +
            'WHERE SessionID = ? AND ChallengeID = ?',
            [
                sessionId,
                challengeId,
            ]
        );
        // if (res.length === 0) {
        //     throw EntityNotFoundError.default(
        //         AuthChallengeMessage.name,
        //         `${sessionId},${challengeId}`,
        //         "SessionID,ChallengeID",
        //         "Unable to retrieve list of messages. " +
        //         "Auth Challenge was not found"
        //     );
        // }
        return res.map(packet =>
            new AuthChallengeMessage(
                sessionId,
                challengeId,
                packet.MessageName,
                JSON.parse(packet.Request),
                JSON.parse(packet.Expected),
                JSON.parse(packet.Response),
                packet.StatusMsg
            )
        );
    }

    static async createMessage(
        sessionId: number,
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
        sessionId: number,
        challengeId: string,
        messageName: AuthChallengeMessageName,
        request: MessagePayload,
        expected: MessagePayload,
        response: MessagePayload
    ) {
        const challengeDef = (await AuthChallenge.getChallenge(
            sessionId,
            challengeId,
            conn
        )).challengeDef;
        const challengeType = challengeDef.challengeType;
        if (-1 === ValidMessages[challengeType].indexOf(messageName)) {
            throw new Error(
                "Cannot createt message. Passed message name " +
                `'${messageName}' is not appropriatet for the challenge.`
            );
        }
        await conn.query(
            'INSERT INTO berytus_account_auth_challenge_message ' +
            '(SessionID, ChallengeID, MessageName, Request, Expected, Response) ' +
            'VALUES (?, ?, ?, ?, ?, ?)',
            [
                sessionId,
                challengeId,
                messageName,
                JSON.stringify(request),
                JSON.stringify(expected),
                JSON.stringify(response)
            ]
        );
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
        await conn.query(
            'UPDATE berytus_account_auth_challenge_message ' +
            'SET Response = ?, StatusMsg = ? ' +
            'WHERE SessionID = ? AND ChallengeID = ? AND MessageName = ?',
            [
                JSON.stringify(response),
                statusMsg,
                this.sessionId,
                this.challengeId,
                this.messageName
            ]
        );
        if (statusMsg !== 'Ok') {
            await conn.query(
                'UPDATE berytus_account_auth_challenge SET Outcome = ? ' +
                'WHERE SessionID = ? AND ChallengeID = ? AND Outcome = "Pending"',
                [
                    'Aborted',
                    this.sessionId,
                    this.challengeId,
                ]
            );
        }
        this.statusMsg = statusMsg;
        this.response = response;
    }
}