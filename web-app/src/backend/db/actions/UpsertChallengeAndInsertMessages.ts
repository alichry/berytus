import type {
    MessageDraft
} from "@root/backend/logic/challenge-handler/AbstractChallengeHandler";
import {
    AuthChallengeMessage,
    type AuthChallengeMessageName,
    type ChallengeMessageStatus,
    type MessagePayload
} from "@root/backend/db/models/AuthChallengeMessage.js";
import { toPostgresBigInt, useConnection, type PoolConnection } from "../pool.js";
import { EAuthOutcome } from "../models/AuthChallenge.js";
import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";
import { AccountDefAuthChallenge } from "../models/AccountDefAuthChallenge.js";
import { AuthSession } from "../models/AuthSession.js";
import { debugAssert, type Assert } from "@root/backend/utils/assert.js";
import { AuthError } from "../errors/AuthError.js";

export interface MessageInput<MN extends AuthChallengeMessageName>
    extends MessageDraft<MN> {
    response: MessagePayload;
    statusMsg: ChallengeMessageStatus;
}

export interface ActionInput {
    sessionId: BigInt;
    challengeId: string;
    messages: Array<MessageInput<AuthChallengeMessageName>>;
};

export class UpsertChallengeAndInsertMessages {
    readonly input: ActionInput;

    public constructor(input: ActionInput) {
        this.input = input;
    }

    async #validateInput(conn: PoolConnection) {
        if (this.input.messages.length === 0) {
            throw new InvalidArgError(
                "At least one message must be provided"
            );
        }
        const session = await AuthSession.getSession(
            this.input.sessionId, conn
        );
        const challengeDef = await AccountDefAuthChallenge.getChallengeDef(
            this.input.challengeId,
            session.accountVersion,
            conn
        );
        const messageDefs = AuthChallengeMessage.getMessageDefs(
            challengeDef.challengeType
        );
        debugAssert(
            assert => assert(messageDefs.length > 0, "messageDefs.length > 0")
        );
        let firstMessageDefIndex: number | null = null;
        let lastMessageDefIndex: number | null = null;
        for (let i = 0; i < this.input.messages.length ; i++) {
            const msg = this.input.messages[i];
            const msgDefIndex = messageDefs.indexOf(msg.messageName);
            if (-1 === msgDefIndex) {
                throw new InvalidArgError(
                    `Passed message name '${msg.messageName}' ` +
                    ` is not appropriate for the challenge.`
                );
            }
            if (firstMessageDefIndex === null) {
                firstMessageDefIndex = msgDefIndex;
            }
            if (lastMessageDefIndex !== null &&
                msgDefIndex <= lastMessageDefIndex) {
                throw new InvalidArgError(
                    `Messages are out of order. ` +
                    `Message at index ${i} with name '${msg.messageName}' ` +
                    `is out of order.`
                );
            }
            lastMessageDefIndex = msgDefIndex;
            if (msg.statusMsg !== 'Ok' && i < this.input.messages.length - 1) {
                throw new InvalidArgError(
                    `Only the last message can have a non-Ok status, ` +
                    `but message at index ${i}/${this.input.messages.length} ` +
                    `has status ${msg.statusMsg}`
                );
            }
        }
        const expectedPreviousMessages = messageDefs.slice(
            0,
            firstMessageDefIndex as number);
        if (this.input.messages[this.input.messages.length - 1].statusMsg !== 'Ok') {
            return {
                authOutcome: EAuthOutcome.Aborted,
                expectedPreviousMessages
            };
        }
        debugAssert(assert =>
            assert(expectedPreviousMessages.length + this.input.messages.length
                <= messageDefs.length)
        );
        if (
            expectedPreviousMessages.length + this.input.messages.length === messageDefs.length
        ) {
            return {
                authOutcome: EAuthOutcome.Succeeded,
                expectedPreviousMessages
            };
        }
        return {
            authOutcome: EAuthOutcome.Pending,
            expectedPreviousMessages
        };
    }

    public async execute(existingConn?: PoolConnection) {
        if (existingConn) {
            return this.#execute(existingConn);
        }
        return useConnection(conn => this.#execute(conn));
    }

    async #execute(conn: PoolConnection) {
        const { authOutcome, expectedPreviousMessages } = await this.#validateInput(conn);
        const res = await conn`
            WITH cte_pending_session AS (
                SELECT SessionID, Outcome FROM berytus_account_auth_session
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND Outcome = ${EAuthOutcome.Pending}
                FOR UPDATE
            ), cte_existing_challenge AS (
                SELECT SessionID, ChallengeID, Outcome FROM berytus_account_auth_challenge
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND ChallengeID = ${this.input.challengeId}
                FOR UPDATE
            ), cte_existing_pending_challenge AS (
                SELECT SessionID, ChallengeID, Outcome FROM cte_existing_challenge
                WHERE Outcome = ${EAuthOutcome.Pending}
            ), cte_pending_challenge_or_dne AS (
                SELECT TRUE
                WHERE (
                    SELECT TRUE FROM cte_existing_pending_challenge
                ) OR NOT EXISTS (
                    SELECT FROM cte_existing_challenge
                )
            ), cte_previous_nonok_message AS (
                SELECT SessionID, ChallengeID, MessageName, StatusMsg
                FROM berytus_account_auth_challenge_message
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND ChallengeID = ${this.input.challengeId}
                AND StatusMsg <> 'Ok'
            ), cte_previous_ok_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, StatusMsg
                FROM berytus_account_auth_challenge_message
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND ChallengeID = ${this.input.challengeId}
                AND StatusMsg = 'Ok'
                ORDER BY CreatedAt ASC
                FOR UPDATE
            ), cte_previous_ok_messages_agg AS (
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
                WHERE c.SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND c.ChallengeID = ${this.input.challengeId}
                GROUP BY c.SessionID, c.ChallengeID
            ), cte_should_write AS (
                SELECT TRUE
                WHERE (
                    SELECT TRUE FROM cte_pending_session
                )
                AND (
                    SELECT TRUE FROM cte_pending_challenge_or_dne
                )
                AND NOT EXISTS (
                    SELECT FROM cte_previous_nonok_message
                )
                AND NOT EXISTS (
                    SELECT TRUE FROM cte_previous_ok_messages_agg
                    WHERE MessageNames <> ${conn.json(expectedPreviousMessages)}
                )
            ), cte_insert_challenge_if_dne AS (
                INSERT INTO berytus_account_auth_challenge
                (SessionID, ChallengeID, Outcome)
                SELECT  ${toPostgresBigInt(this.input.sessionId)},
                        ${this.input.challengeId},
                        ${EAuthOutcome.Pending}
                WHERE (
                    SELECT TRUE FROM cte_should_write
                ) AND NOT EXISTS (
                    SELECT FROM cte_existing_challenge
                )
                RETURNING SessionID, ChallengeID, Outcome
            ), cte_insert_messages AS (
                INSERT INTO berytus_account_auth_challenge_message
                (SessionID, ChallengeID, MessageName,
                Request, Expected, Response, StatusMsg)
                SELECT * FROM
                    (VALUES ${conn(this.input.messages.map(msg => [
                        toPostgresBigInt(this.input.sessionId),
                        this.input.challengeId,
                        msg.messageName,
                        conn.json(msg.request),
                        conn.json(msg.expected),
                        conn.json(msg.response),
                        msg.statusMsg
                    ] as const))}) AS insert_data (
                        SessionID, ChallengeID, MessageName,
                        Request, Expected, Response, StatusMsg
                    )
                WHERE (
                    SELECT TRUE FROM cte_should_write
                ) AND (
                    (SELECT TRUE FROM cte_insert_challenge_if_dne)
                    OR TRUE
                )
                RETURNING SessionID, ChallengeID, MessageName,
                Request, Expected, Response, StatusMsg
            ), cte_update_challenge_outcome AS (
                UPDATE berytus_account_auth_challenge
                SET Outcome = ${authOutcome},
                    UpdatedAt = NOW()
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND ChallengeID = ${this.input.challengeId}
                AND Outcome = ${EAuthOutcome.Pending}
                AND Outcome <> ${authOutcome}
                AND (
                    SELECT TRUE FROM cte_should_write
                ) AND EXISTS (
                    SELECT FROM cte_insert_messages
                )
                RETURNING SessionID, ChallengeID, Outcome
            ) -- result:
            SELECT
                (
                    SELECT TRUE
                    FROM cte_should_write
                ) AS Written,
                (
                    SELECT to_jsonb(t)
                    FROM cte_update_challenge_outcome AS t
                ) AS UpdatedChallengeOutcome,
                (
                    SELECT COALESCE(jsonb_agg(to_jsonb(t)), '[]'::jsonb)
                    FROM cte_insert_messages AS t
                ) AS InsertedMessages,
                (
                    SELECT to_jsonb(t)
                    FROM cte_insert_challenge_if_dne AS t
                ) AS InsertedChallenge,
                (
                    SELECT COALESCE(jsonb_agg(to_jsonb(t)), '[]'::jsonb)
                    FROM cte_previous_ok_messages_agg AS t
                ) AS PreviousOkMessages,
                (
                    SELECT COALESCE(jsonb_agg(to_jsonb(t)), '[]'::jsonb)
                    FROM cte_previous_nonok_message AS t
                ) AS PreviousNonOkMessages,
                (
                    SELECT TRUE
                    FROM cte_pending_challenge_or_dne
                ) AS PendingChallengeOrDne,
                (
                    SELECT to_jsonb(t)
                    FROM cte_existing_pending_challenge AS t
                ) AS ExistingPendingChallenge,
                (
                    SELECT to_jsonb(t)
                    FROM cte_existing_challenge AS t
                ) AS ExistingChallenge,
                (
                    SELECT to_jsonb(t)
                    FROM cte_pending_session AS t
                ) AS ExistingPendingSession
        `;
        const { written } = res[0];
        if (! written) {
            throw new AuthError(
                `Cannot upsert challenge and insert messages, `
                + `integrity validation failed.`
            );
        }
        debugAssert((assert: Assert) => {
            assert(res.length === 1, "Expected exactly one result row");
            const row = res[0];

            assert(row.written === null || row.written === true,
                "Written should be null or true"
            );
            assert(row.pendingchallengeordne === null || row.pendingchallengeordne === true,
                "PendingChallengeOrDne should be null or true"
            );
            assert(
                typeof row.insertedchallenge === "object",
                "InsertedChallenge should be an object"
            );
            assert(
                ! Array.isArray(row.insertedchallenge),
                "InsertedChallenge should not be an array"
            );
            assert(
                typeof row.updatedchallengeoutcome === "object",
                "UpdatedChallengeOutcome should be an object"
            );
            assert(
                ! Array.isArray(row.updatedchallengeoutcome),
                "UpdatedChallengeOutcome should not be an array"
            );
            assert(
                typeof row.existingpendingchallenge === "object",
                "ExistingPendingChallenge should be an object"
            );
            assert(
                ! Array.isArray(row.existingpendingchallenge),
                "ExistingPendingChallenge should not be an array"
            );
            assert(
                typeof row.existingchallenge === "object",
                "ExistingChallenge should be an object"
            );
            assert(
                ! Array.isArray(row.existingchallenge),
                "ExistingChallenge should not be an array"
            );
            assert(
                typeof row.existingpendingsession === "object",
                "ExistingPendingSession should be an object"
            );
            assert(
                ! Array.isArray(row.existingpendingsession),
                "ExistingPendingSession should not be an array"
            );
            assert(row.insertedmessages !== null,
                "InsertedMessages should not be null");
            assert(Array.isArray(row.insertedmessages),
                "InsertedMessages should be an array");
            assert(row.previousokmessages !== null,
                "PreviousOkMessages should not be null");
            assert(Array.isArray(row.previousokmessages),
                "PreviousOkMessages should be an array");
            assert(row.previousnonokmessages !== null,
                "PreviousNonOkMessages should not be null");
            assert(Array.isArray(row.previousnonokmessages),
                "PreviousNonOkMessages should be an array");

            const written = Boolean(row.written);
            const challengeIsPendingOrDne = Boolean(row.pendingchallengeordne);

            if (! written) {
                assert(row.insertedchallenge === null,
                    "If Written is false, InsertedChallenge must be null"
                );
                assert(row.insertedmessages.length === 0,
                    "If Written is false, InsertedMessages must be empty"
                );
                assert(row.updatedchallengeoutcome === null,
                    "If Written is false, UpdatedChallengeOutcome must be null"
                );
            } else {
                assert(
                    row.insertedmessages.length === this.input.messages.length,
                    "If Written is true, InsertedMessages length should match input messages length"
                );
            }

            if (row.insertedchallenge) {
                assert(written === true,
                    "If InsertedChallenge is not null, Written must be true"
                );
                assert(
                    row.existingpendingchallenge === null,
                    "If InsertedChallenge is not null, ExistingPendingChallenge should be null"
                );
                assert(
                    BigInt(row.insertedchallenge.sessionid) === this.input.sessionId,
                    "InsertedChallenge.sessionid should equal input.sessionId"
                );
                assert(row.insertedchallenge.challengeid === this.input.challengeId,
                    "InsertedChallenge.challengeid should equal input.challengeId"
                );
                assert(row.insertedchallenge.outcome === EAuthOutcome.Pending,
                    "InsertedChallenge.outcome should be Pending"
                );
            }
            if (row.updatedchallengeoutcome !== null) {
                assert(written === true,
                    "If UpdatedChallengeOutcome is not null, Written must be true"
                );
                assert(
                    row.updatedchallengeoutcome.sessionid !== this.input.sessionId,
                    "UpdatedChallengeOutcome.sessionid should equal input.sessionId"
                );
                assert(
                    row.updatedchallengeoutcome.challengeid !== this.input.challengeId,
                    "UpdatedChallengeOutcome.challengeid should equal input.challengeId"
                );
                assert(
                    row.updatedchallengeoutcome.outcome !== null,
                    "UpdatedChallengeOutcome.outcome should not be null"
                );
                assert(
                    row.updatedchallengeoutcome.outcome in EAuthOutcome,
                    "UpdatedChallengeOutcome.outcome should be in EAuthOutcome"
                );
                assert(
                    row.updatedchallengeoutcome.outcome !== EAuthOutcome.Pending,
                    "UpdatedChallengeOutcome.outcome must not be Pending"
                );
                assert(
                    row.updatedchallengeoutcome.outcome === authOutcome,
                    "UpdatedChallengeOutcome.outcome must equal authOutcome"
                );
            }
            if (row.existingchallenge !== null) {
                assert(row.insertedchallenge === null,
                    "If ExistingChallenge is not null, InsertedChallenge must be null"
                );
                assert(BigInt(row.existingpendingchallenge.sessionid) === this.input.sessionId,
                    "ExistingPendingChallenge.sessionid should equal input.sessionId"
                );
                assert(row.existingpendingchallenge.challengeid === this.input.challengeId,
                    "ExistingPendingChallenge.challengeid should equal input.challengeId"
                );
            }
            if (row.existingpendingchallenge !== null) {
                assert(row.insertedchallenge === null,
                    "If ExistingPendingChallenge is not null, InsertedChallenge must be null"
                );
                assert(BigInt(row.existingpendingchallenge.sessionid) === this.input.sessionId,
                    "ExistingPendingChallenge.sessionid should equal input.sessionId"
                );
                assert(row.existingpendingchallenge.challengeid === this.input.challengeId,
                    "ExistingPendingChallenge.challengeid should equal input.challengeId"
                );
                assert(row.existingpendingchallenge.outcome === EAuthOutcome.Pending,
                    "ExistingPendingChallenge.outcome should be Pending"
                );
            }
            const challengeInserted =
                row.insertedchallenge !== null;
            const messagesInserted = row.insertedmessages.length > 0;
            const updatedChallengeOutcome = row.updatedchallengeoutcome !== null
                ? true
                : false;
            const challengeOutcome = row.updatedchallengeoutcome !== null
                ? row.updatedchallengeoutcome.outcome
                : row.insertedchallenge !== null
                ? row.insertedchallenge.outcome
                : row.existingpendingchallenge !== null
                ? row.existingpendingchallenge.outcome
                : null;
            assert(
                challengeOutcome === authOutcome,
                "challengeOutcome === authOutcome"
            );
            assert(
                challengeOutcome !== null,
                "challengeOutcome should not be null"
            );
            assert(
                updatedChallengeOutcome ?
                    (
                        challengeOutcome === EAuthOutcome.Aborted
                        || challengeOutcome === EAuthOutcome.Succeeded
                    )
                    : (challengeOutcome === EAuthOutcome.Pending),
                "If updatedChallengeOutcome is true, challengeOutcome "
                + "must be Aborted or Succeeded; otherwise, it must "
                + "be Pending"
            );
            for (let i = 0; i < row.insertedmessages.length; i++) {
                const insertedMsg: any = row.insertedmessages[i];
                const inputMsg = this.input.messages[i];
                assert(BigInt(insertedMsg.sessionid) === this.input.sessionId,
                    `Inserted message at index ${i} has incorrect SessionID`);
                assert(insertedMsg.challengeid === this.input.challengeId,
                    `Inserted message at index ${i} has incorrect ChallengeID`);
                assert(insertedMsg.messagename === inputMsg.messageName,
                    `Inserted message at index ${i} has incorrect MessageName`);
                assert.deepEqual(insertedMsg.request, inputMsg.request,
                    `Inserted message at index ${i} has incorrect Request`);
                assert.deepEqual(insertedMsg.expected, inputMsg.expected,
                    `Inserted message at index ${i} has incorrect Expected`);
                assert.deepEqual(insertedMsg.response, inputMsg.response,
                    `Inserted message at index ${i} has incorrect Response`);
                assert(insertedMsg.statusmsg === inputMsg.statusMsg,
                    `Inserted message at index ${i} has incorrect StatusMsg`);
            }
        });
    }
}