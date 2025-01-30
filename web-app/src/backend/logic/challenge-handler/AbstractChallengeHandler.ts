import { AuthChallenge, EAuthOutcome } from "@root/backend/db/models/AuthChallenge";
import type { ChallengePendingMessage, IChallengeHandler, InitiateAuthChallengeResullt, ProcessChallengeMessageResult } from "./types";
import { AuthChallengeMessage, type AuthChallengeMessageName, type ChallengeMessageStatus, type MessagePayload } from "@root/backend/db/models/AuthChallengeMessage";
import { pool } from "@root/backend/db/pool";
import type { PoolConnection } from "mysql2/promise";
import { EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge";
import { AuthSession } from "@root/backend/db/models/AuthSession";

export interface MessageDraft<MN extends AuthChallengeMessageName> {
    messageName: MN;
    request: MessagePayload;
    expected: MessagePayload;
}

interface CommonChallengeHandlerConstructor {
    prototype: AbstractChallengeHandler<AuthChallengeMessageName>;
    new(conn: PoolConnection, session: AuthSession, challenge: AuthChallenge): AbstractChallengeHandler<AuthChallengeMessageName>;
}

export type Message<MN extends AuthChallengeMessageName> = Omit<AuthChallengeMessage, "messageName"> & { messageName: MN };

export type MessageDictionary<MN extends AuthChallengeMessageName> = Partial<Record<MN, Message<MN>>>

interface HandlerMessageDictionary<MN extends AuthChallengeMessageName> {
    pendingMessage: Message<MN> | null;
    processedMessages: MessageDictionary<MN>;
}

export abstract class AbstractChallengeHandler<MN extends AuthChallengeMessageName>
    implements IChallengeHandler {
    protected readonly conn: PoolConnection;
    readonly session: AuthSession;
    readonly challenge: AuthChallenge;
    private destroyed = false;

    /**
     * @param conn Should be in transaction mode
     * @param challenge
     */
    public constructor(
        conn: PoolConnection,
        session: AuthSession,
        challenge: AuthChallenge,
    ) {
        this.conn = conn;
        this.session = session;
        this.challenge = challenge;
    }

    abstract get handlerType(): EChallengeType;

    /**
     * Draft the next message. return null if there
     * are no more messages to be drafted.
     */
    protected abstract draftNextMessage(
        processedMessages: MessageDictionary<MN>
    ): Promise<MessageDraft<MN> | null>;

    protected abstract validateMessageResponse(
        processedMessages: MessageDictionary<MN>,
        pendingMessage: Message<MN>,
        response: MessagePayload
    ): Promise<NonNullable<ChallengeMessageStatus>>;

    static async initiateChallenge(
        sessionId: number,
        challengeId: string,
        handlerCtor: CommonChallengeHandlerConstructor
    ) {
        const conn = await pool.getConnection();
        try {
            await conn.beginTransaction();
            const session = await AuthSession.getSession(
                sessionId,
                conn
            );
            const challenge = await AuthChallenge.createChallenge(
                sessionId,
                challengeId,
                conn
            );
            if (
                handlerCtor.prototype.handlerType !==
                challenge.challengeDef.challengeType
            ) {
                throw new Error('ChallengeDef/Handler mismatch');
            }
            const handler = new handlerCtor(
                conn,
                session,
                challenge,
            );
            await handler.getPendingMessage();
            return handler;
        } catch (e) {
            conn.release();
            throw e;
        }
    }

    static async loadChallenge(
        sessionId: number,
        challengeId: string,
        handlerCtor: CommonChallengeHandlerConstructor
    ) {
        const conn = await pool.getConnection();
        try {
            await conn.beginTransaction();
            const session = await AuthSession.getSession(
                sessionId,
                conn
            );
            const challenge = await AuthChallenge.getChallenge(
                sessionId,
                challengeId,
                conn
            );
            if (challenge.outcome !== EAuthOutcome.Pending) {
                throw new Error("Cannot load challenge in a non-pending state");
            }
            if (
                handlerCtor.prototype.handlerType !==
                challenge.challengeDef.challengeType
            ) {
                throw new Error('ChallengeDef/Handler mismatch');
            }
            const handler = new handlerCtor(
                conn,
                session,
                challenge
            );
            await handler.getPendingMessage();
            return handler;
        } catch (e) {
            conn.release();
            throw e;
        }
    }

    async getPendingMessage(): Promise<ChallengePendingMessage<MN> | null> {
        const { processedMessages, pendingMessage } = await this.getMessages();
        if (pendingMessage) {
            return pendingMessage;
        }
        // no pending message was found in the db,
        // let's request a draft and save it.
        const nextMessageDraft = await this.draftNextMessage(processedMessages);
        if (nextMessageDraft === null) {
            if (Object.values(processedMessages).length === 0) {
                throw new Error(
                    'Expecting initial message draft to be non-null!'
                );
            }
            return null; // no more messages
        }
        await AuthChallengeMessage.createMessage(
            this.challenge.sessionId,
            this.challenge.challengeId,
            nextMessageDraft.messageName,
            nextMessageDraft.request,
            nextMessageDraft.expected,
            null,
            this.conn
        );
        return nextMessageDraft;
    }

    async processPendingMessageResponse(
        response: MessagePayload
    ): Promise<NonNullable<ChallengeMessageStatus>> {
        const { processedMessages, pendingMessage } = await this.getMessages();
        if (! pendingMessage) {
            throw new Error(
                "There are no pending message to process. Did we call " +
                "getPendingMessage() before processPendingMessageResponse()?"
            );
        }
        const statusMsg = await this.validateMessageResponse(
            processedMessages,
            pendingMessage,
            response
        );
        await pendingMessage.updateResponseAndStatus(
            response,
            statusMsg,
            this.conn
        );
        // save next pending message.
        const next = await this.getPendingMessage();
        if (! next) {
            await this.challenge.updateOutcome(
                statusMsg === `Ok`
                    ? EAuthOutcome.Succeeded : EAuthOutcome.Aborted,
                this.conn
            );
        }
        return statusMsg;
    }

    async getMessages(): Promise<HandlerMessageDictionary<MN>> {
        const messages = await AuthChallengeMessage.getAllMessages(
            this.challenge.sessionId,
            this.challenge.challengeId,
            this.conn
        ) as unknown as Message<MN>[];
        let pendingMessage: HandlerMessageDictionary<MN>['pendingMessage'] = null;
        let processedMessages:  HandlerMessageDictionary<MN>['processedMessages'] = {};
        for (let i = 0; i < messages.length; i++) {
            const msg = messages[i];
            if (msg.statusMsg !== null) {
                processedMessages[msg.messageName] = msg;
                continue;
            }
            if (pendingMessage !== null) {
                throw new Error(
                    'Integrity validation failed. More than one pending message were found.'
                );
            }
            pendingMessage = msg;
        }
        return { pendingMessage, processedMessages };
    }

    destroy() {
        if (this.destroyed) {
            return;
        }
        this.conn.release();
        this.destroyed = true;
    }

    async save() {
        // make sure that there are at least one message before saving.
        const messages = await this.getMessages();
        if (
            messages.pendingMessage ||
            Object.values(messages.processedMessages).length > 0
        ) {
            await this.conn.commit();
            return;
        }
        throw new Error("Cannot save challenge. No messages were created.");
    }
}