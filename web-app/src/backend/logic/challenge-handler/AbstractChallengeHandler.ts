import {
    AuthChallenge,
    EAuthOutcome
} from "@root/backend/db/models/AuthChallenge.js";
import type {
    ChallengePendingMessage,
    IChallengeHandler
} from "./types";
import {
    AuthChallengeMessage,
    type AuthChallengeMessageName,
    type ChallengeMessageStatus,
    type MessagePayload
} from "@root/backend/db/models/AuthChallengeMessage.js";
import { pool, type PoolConnection } from "@root/backend/db/pool.js";
import { AccountDefAuthChallenge, EChallengeType } from "@root/backend/db/models/AccountDefAuthChallenge.js";
import { AuthSession } from "@root/backend/db/models/AuthSession.js";
import { InternalError } from "@root/backend/errors/InternalError.js";
import { debugAssert } from "@root/backend/utils/assert.js";
import { UpsertChallengeAndMessages } from "@root/backend/db/actions/UpsertChallengeAndMessages.js";
import { EntityNotFoundError } from "@root/backend/db/errors/EntityNotFoundError.js";
import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";

export interface MessageDraft<MN extends AuthChallengeMessageName> {
    messageName: MN;
    request: MessagePayload;
    expected: MessagePayload;
}

export interface CommonChallengeHandlerConstructor<MN extends AuthChallengeMessageName> {
    prototype: AbstractChallengeHandler<MN>;
    new(
        conn: PoolConnection,
        session: AuthSession,
        challengeDef: AccountDefAuthChallenge,
        existingMessages: ReadonlyArray<Message<AuthChallengeMessageName>>
    ): AbstractChallengeHandler<MN>;
}

export interface Message<MN extends AuthChallengeMessageName>
    extends MessageDraft<MN> {
    response: MessagePayload;
    statusMsg: ChallengeMessageStatus
}

export type MessageDictionary<MN extends AuthChallengeMessageName> =
    Partial<Record<MN, Message<MN>>>;

interface HandlerMessageDictionary<MN extends AuthChallengeMessageName> {
    pendingMessage: Message<MN> | null;
    processedMessages: MessageDictionary<MN>;
}

export abstract class AbstractChallengeHandler<MN extends AuthChallengeMessageName>
    implements IChallengeHandler {
    readonly conn: PoolConnection;
    readonly session: AuthSession;
    readonly challengeDef: AccountDefAuthChallenge;
    private readonly existingMessages: ReadonlyArray<Message<MN>>;
    private readonly prospectiveMessages: Array<Message<MN>>;
    private destroyed: boolean;
    #challenge: AuthChallenge | null;

    /**
     * @param conn Should be in transaction mode
     * @param challenge
     */
    public constructor(
        conn: PoolConnection,
        session: AuthSession,
        challengeDef: AccountDefAuthChallenge,
        existingMessages: ReadonlyArray<Message<MN>>
    ) {
        this.conn = conn;
        this.session = session;
        this.challengeDef = challengeDef;
        this.existingMessages = existingMessages;
        this.prospectiveMessages = [];
        this.destroyed = false;
        this.#challenge = null;
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

    static async setupChallenge<MN extends AuthChallengeMessageName>(
        sessionId: BigInt,
        challengeId: string,
        handlerCtor: CommonChallengeHandlerConstructor<MN>,
        conn?: PoolConnection
    ) {
        return AbstractChallengeHandler.#setupChallenge(
            conn || pool,
            sessionId,
            challengeId,
            handlerCtor
        );
    }

    static async #setupChallenge<MN extends AuthChallengeMessageName>(
        conn: PoolConnection,
        sessionId: BigInt,
        challengeId: string,
        handlerCtor: CommonChallengeHandlerConstructor<MN>
    ) {
        const session = await AuthSession.getSession(
            sessionId,
            conn
        );
        if (session.outcome !== EAuthOutcome.Pending) {
            throw new Error(
                `Cannot setup challenge in a non-pendong session.`
            )
        }
        const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
            challengeId,
            session.accountVersion,
            conn
        );
        if (
            handlerCtor.prototype.handlerType !==
            challengeDef.challengeType
        ) {
            throw new Error('ChallengeDef/Handler mismatch');
        }
        try {
            const challenge = await AuthChallenge.getChallenge(
                sessionId,
                challengeId,
                conn
            );
            if (challenge.outcome !== EAuthOutcome.Pending) {
                throw new Error("Cannot setup challenge in a non-pending state");
            }
        } catch (e) {
            if (!(e instanceof EntityNotFoundError)) {
                throw e;
            }
        }
        const messages = await AuthChallengeMessage.getAllMessages(
            sessionId,
            challengeId,
            conn
        );
        const handler = new handlerCtor(
            conn,
            session,
            challengeDef,
            messages.map(m => ({
                messageName: m.messageName,
                request: m.request,
                expected: m.expected,
                response: m.response,
                statusMsg: m.statusMsg
            }))
        );
        await handler.ensurePendingMessage();
        return handler;
    }

    get challenge() {
        return this.#challenge;
    }

    async ensurePendingMessage(): Promise<ChallengePendingMessage<MN> | null> {
        const { processedMessages, pendingMessage } = await this.getMessages();
        if (pendingMessage) {
            return pendingMessage;
        }
        // no pending message was found in the db,
        // let's request a draft and save it.
        const nextMessageDraft = await this.draftNextMessage(processedMessages);
        if (nextMessageDraft === null) {
            if (Object.values(processedMessages).length === 0) {
                throw new InternalError(
                    'Expecting initial message draft to be non-null!'
                );
            }
            return null; // no more messages
        }
        debugAssert(assert =>
            this.prospectiveMessages.forEach(
                m => assert(
                        m.statusMsg !== null,
                        'm.statusMsg !== null'
                    )
            )
        );
        this.prospectiveMessages.push({
            ...nextMessageDraft,
            response: null,
            statusMsg: null
        });
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
        pendingMessage.response = response;
        pendingMessage.statusMsg = statusMsg;
        // append next prospective, pending message.
        await this.ensurePendingMessage();
        return statusMsg;
    }

    async getMessages(): Promise<HandlerMessageDictionary<MN>> {
        let pendingMessage: HandlerMessageDictionary<MN>['pendingMessage'] = null;
        let processedMessages:  HandlerMessageDictionary<MN>['processedMessages'] = {};
        const allMessages = [
            ...this.existingMessages,
            ...this.prospectiveMessages
        ];
        for (let i = 0; i < allMessages.length; i++) {
            const msg = allMessages[i];
            if (msg.statusMsg !== null) {
                processedMessages[msg.messageName] = msg;
                continue;
            }
            if (i !== allMessages.length - 1) {
                throw new InternalError(
                    'Pending message must be the last message.'
                );
            }
            pendingMessage = msg;
        }
        return { pendingMessage, processedMessages };
    }

    async getPendingMessage() {
        const { pendingMessage } = await this.getMessages();
        return pendingMessage;
    }

    destroy() {
        if (this.destroyed) {
            return;
        }
        this.destroyed = true;
    }

    async save() {
        if (this.destroyed) {
            throw new Error("Cannot save. Handler has been destroyed.");
        }
        if (this.prospectiveMessages.length === 0) {
            throw new Error("Cannot save challenge. No messages to write.");
        }
        try {
            const action = new UpsertChallengeAndMessages({
                sessionId: this.session.sessionId,
                challengeId: this.challengeDef.challengeId,
                messages: this.prospectiveMessages
            });
            const outcome = await action.execute();
            this.#challenge = new AuthChallenge(
                this.session.sessionId,
                this.challengeDef.challengeId,
                this.challengeDef,
                outcome
            );
        } finally {
            this.destroy();
        }
    }

    protected static validateMessages<MN extends AuthChallengeMessageName>(
        messageNames: ReadonlyArray<MN>,
        existingMessages: ReadonlyArray<Message<AuthChallengeMessageName>>
    ): asserts existingMessages is ReadonlyArray<Message<MN>> {
        if (existingMessages.length === 0) {
            return;
        }
        const firstMessage = existingMessages[0];
        const len = Math.min(existingMessages.length, messageNames.length);
        const start = messageNames.findIndex(f => f === firstMessage.messageName);
        if (start === -1) {
            throw new InvalidArgError(
                `Bad message list. Message name ` +
                `'${firstMessage.messageName}' is not recognised`
            );
        }
        for (let i = 1; i < len; i++) {
            const {messageName} = existingMessages[i];
            const expectedMessageName = messageNames[start + i];
            if (messageName !== expectedMessageName) {
                throw new InvalidArgError(
                    "Bad message list. Expected message name "
                    + `'${messageName}' to equal '${expectedMessageName}'`
                );
            }
        }
    }
}