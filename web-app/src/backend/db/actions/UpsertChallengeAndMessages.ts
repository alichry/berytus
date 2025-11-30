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
    messages: ReadonlyArray<MessageInput<AuthChallengeMessageName>>;
};

export class UpsertChallengeAndMessages {
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
            AuthChallengeMessage.validateStatusMsg(msg.statusMsg);
        }
        const expectedPreviousMessages = messageDefs.slice(
            0,
            firstMessageDefIndex as number);
        debugAssert(assert =>
            this.input.messages.slice(0, this.input.messages.length - 1)
                .forEach(m => assert(m.statusMsg === "Ok", "m.statusMsg === 'Ok'"))
        );
        debugAssert(assert =>
            assert(expectedPreviousMessages.length + this.input.messages.length
                <= messageDefs.length)
        );
        const lastStatusMsg = this.input.messages[this.input.messages.length - 1]
            .statusMsg;
        if (lastStatusMsg === null) {
            return {
                authOutcome: EAuthOutcome.Pending,
                expectedPreviousMessages
            };
        }
        if (lastStatusMsg !== 'Ok') {
            return {
                authOutcome: EAuthOutcome.Aborted,
                expectedPreviousMessages
            };
        }
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
            ),  cte_existing_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, Request,
                       Expected, Response,
                       StatusMsg, CreatedAt
                FROM berytus_account_auth_challenge_message
                WHERE SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND ChallengeID = ${this.input.challengeId}
                FOR UPDATE
            ), cte_previous_nonok_message AS (
                SELECT SessionID, ChallengeID,
                       MessageName, StatusMsg
                FROM cte_existing_messages
                WHERE StatusMsg <> 'Ok'
            ), cte_previous_ok_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, StatusMsg
                FROM cte_existing_messages
                WHERE StatusMsg = 'Ok'
                ORDER BY CreatedAt ASC
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
            ), cte_previous_ok_messages_agg2 AS (
                -- same as the above, but returns
                -- an empty array if the challenge dne
                SELECT ${toPostgresBigInt(this.input.sessionId)} AS SessionID,
                       ${this.input.challengeId} AS ChallengeID,
                       COALESCE(
                            (
                                SELECT MessageNames
                                FROM cte_previous_ok_messages_agg
                            ),
                            '[]'::jsonb
                       ) AS MessageNames
            ), cte_completed_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, Request,
                       Expected, Response,
                       StatusMsg
                FROM cte_existing_messages
                WHERE StatusMsg IS NOT NULL
            ), cte_pending_messages AS (
                SELECT SessionID, ChallengeID,
                       MessageName, Request,
                       Expected, Response,
                       StatusMsg
                FROM cte_existing_messages
                WHERE StatusMsg IS NULL
            ), cte_messages_to_process AS (
                SELECT * FROM
                    (VALUES ${conn(this.input.messages.map(msg => [
                        toPostgresBigInt(this.input.sessionId),
                        this.input.challengeId,
                        msg.messageName,
                        conn.json(msg.request),
                        conn.json(msg.expected),
                        conn.json(msg.response),
                        msg.statusMsg
                    ] as const))}) AS message (
                        SessionID, ChallengeID, MessageName,
                        Request, Expected, Response, StatusMsg
                    )
            ), cte_messages_to_insert AS (
                SELECT pm.SessionID, pm.ChallengeID,
                       pm.MessageName, pm.Request,
                       pm.Expected, pm.Response,
                       pm.StatusMsg
                FROM cte_messages_to_process AS pm
                LEFT JOIN cte_existing_messages AS em
                    ON em.MessageName = pm.MessageName
                WHERE em.MessageName IS NULL
            ), cte_messages_to_update AS (
                SELECT pm.SessionID, pm.ChallengeID,
                       pm.MessageName, pm.Request,
                       pm.Expected, pm.Response,
                       pm.StatusMsg
                FROM cte_messages_to_process AS pm
                LEFT JOIN cte_pending_messages AS em
                    ON em.MessageName = pm.MessageName
                WHERE em.MessageName IS NOT NULL
            ), cte_messages_to_write AS (
                SELECT * FROM cte_messages_to_insert
                UNION ALL
                SELECT * FROM cte_messages_to_update
            ), cte_rejected_messages AS (
                SELECT pm.*
                FROM cte_messages_to_process AS pm
                LEFT JOIN cte_messages_to_write AS wm
                    ON wm.MessageName = pm.MessageName
                WHERE wm.MessageName IS NULL
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
                AND (
                    SELECT TRUE FROM cte_previous_ok_messages_agg2
                    WHERE MessageNames = ${conn.json(expectedPreviousMessages)}
                ) AND NOT EXISTS (
                    SELECT FROM cte_rejected_messages
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
                SELECT SessionID, ChallengeID, MessageName,
                       Request, Expected, Response, StatusMsg
                FROM cte_messages_to_insert
                WHERE (
                    SELECT TRUE FROM cte_should_write
                ) AND (
                    (SELECT TRUE FROM cte_insert_challenge_if_dne)
                    OR TRUE
                )
                RETURNING SessionID, ChallengeID, MessageName,
                Request, Expected, Response, StatusMsg
            ), cte_update_messages AS (
                --  Note(berytus): We can restrict updates to
                -- { Response, StatusMsg } by modifying the logic
                -- in cte_messages_to_update to not include
                -- existing, pending messages with a non-matching
                -- { Request, Expected }. And cte_rejected_messages
                -- would pick it up.
                UPDATE berytus_account_auth_challenge_message em
                SET Request = um.Request,
                    Expected = um.Expected,
                    Response = um.Response,
                    StatusMsg = um.StatusMsg
                FROM cte_messages_to_update um
                WHERE em.SessionID = ${toPostgresBigInt(this.input.sessionId)}
                AND em.ChallengeID = ${this.input.challengeId}
                AND em.MessageName = um.MessageName
                AND (
                    SELECT TRUE FROM cte_should_write
                )
                RETURNING em.SessionID, em.ChallengeID, em.MessageName,
                          em.Request, em.Expected, em.Response,
                          em.StatusMsg
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
                    SELECT COALESCE(jsonb_agg(to_jsonb(t)), '[]'::jsonb)
                    FROM cte_update_messages AS t
                ) AS UpdatedMessages,
                (
                    SELECT to_jsonb(t)
                    FROM cte_insert_challenge_if_dne AS t
                ) AS InsertedChallenge,
                (
                    SELECT to_jsonb(t)
                    FROM cte_previous_ok_messages_agg2 AS t
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
        debugAssert((assert: Assert) => {
            assert(
                res.length === 1,
                "Expected exactly one result row"
            );
            const row = res[0];

            assert(
                row.written === null || row.written === true,
                "Written should be null or true"
            );
            assert(
                row.pendingchallengeordne === null
                || row.pendingchallengeordne === true,
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
            assert(row.updatedmessages !== null,
                "UpdatedMessages should not be null");
            assert(Array.isArray(row.updatedmessages),
                "UpdatedMessages should be an array");
            assert(row.previousokmessages !== null,
                "PreviousOkMessages should not be null");
            assert(row.previousnonokmessages !== null,
                "PreviousNonOkMessages should not be null");
            assert(Array.isArray(row.previousnonokmessages),
                "PreviousNonOkMessages should be an array");

            const written = Boolean(row.written);
            const challengeIsPendingOrDne = Boolean(row.pendingchallengeordne);

            if (! written) {
                assert(
                    row.insertedchallenge === null,
                    "If Written is false, InsertedChallenge must be null"
                );
                assert(
                    row.insertedmessages.length === 0,
                    "If Written is false, InsertedMessages must be empty"
                );
                assert(
                    row.updatedmessages.length === 0,
                    "If Written is false, UpdatedMessages must be empty"
                );
                assert(
                    row.updatedchallengeoutcome === null,
                    "If Written is false, UpdatedChallengeOutcome must be null"
                );
            } else {
                assert(
                    row.insertedmessages.length + row.updatedmessages.length
                        === this.input.messages.length,
                    "If Written is true, InsertedMessages length + "
                    + "UpdatedMessages length should equal input messages "
                    + "length"
                );
                assert(
                    row.previousnonokmessages.length === 0,
                    "If Written is true, PreviousNonOkMessages length should be 0"
                );
            }

            if (row.insertedchallenge) {
                assert(
                    written === true,
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
                assert(
                    row.insertedchallenge.challengeid === this.input.challengeId,
                    "InsertedChallenge.challengeid should equal input.challengeId"
                );
                assert(
                    row.insertedchallenge.outcome === EAuthOutcome.Pending,
                    "InsertedChallenge.outcome should be Pending"
                );
            }
            if (row.updatedchallengeoutcome !== null) {
                assert(written === true,
                    "If UpdatedChallengeOutcome is not null, Written must be true"
                );
                assert(
                    BigInt(row.updatedchallengeoutcome.sessionid) == this.input.sessionId,
                    "UpdatedChallengeOutcome.sessionid should equal input.sessionId"
                );
                assert(
                    row.updatedchallengeoutcome.challengeid === this.input.challengeId,
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
                assert(
                    row.insertedchallenge === null,
                    "If ExistingChallenge is not null, InsertedChallenge must be null"
                );
            }
            if (row.existingpendingchallenge !== null) {
                assert(
                    row.insertedchallenge === null,
                    "If ExistingPendingChallenge is not null, InsertedChallenge must be null"
                );
                assert(
                    row.existingchallenge !== null,
                    "If ExistingPendingChallenge is not null, ExistingChallenge must be not null"
                );
                assert(
                    BigInt(row.existingpendingchallenge.sessionid) === this.input.sessionId,
                    "ExistingPendingChallenge.sessionid should equal input.sessionId"
                );
                assert(
                    row.existingpendingchallenge.challengeid === this.input.challengeId,
                    "ExistingPendingChallenge.challengeid should equal input.challengeId"
                );
                assert(
                    row.existingpendingchallenge.outcome === EAuthOutcome.Pending,
                    "ExistingPendingChallenge.outcome should be Pending"
                );
                assert.deepEqual(
                    row.existingchallenge, row.existingpendingchallenge,
                    "ExistingChallenge should equal ExistingPendingChallenge"
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
            if (updatedChallengeOutcome) {
                assert(
                    challengeOutcome === authOutcome,
                    "If updatedChallengeOutcome is true, "
                    + "challengeOutcome === authOutcome"
                );
                assert(
                    challengeOutcome !== null,
                    "If updatedChallengeOutcome is true, "
                    + "challengeOutcome should not be null"
                );
                assert(
                    challengeOutcome === EAuthOutcome.Aborted
                            || challengeOutcome === EAuthOutcome.Succeeded,
                    "If updatedChallengeOutcome is true, challengeOutcome "
                    + "must be Aborted or Succeeded"
                );
            }
            if (! written) {
                return;
            }
            for (let i = 0; i < this.input.messages.length; i++) {
                const inputMsg = this.input.messages[i];
                const insertedMsg = row.insertedmessages.find(
                    m => m.messagename === inputMsg.messageName
                );
                const updatedMsg = row.updatedmessages.find(
                    m => m.messagename === inputMsg.messageName
                );
                assert(
                    Number(Boolean(insertedMsg)) ^ Number(Boolean(updatedMsg)),
                    "inserted ^ updated"
                );
                const upsertedMsg: any = insertedMsg || updatedMsg;
                assert(BigInt(upsertedMsg.sessionid) === this.input.sessionId,
                    `Inserted message at index ${i} has incorrect SessionID`);
                assert(upsertedMsg.challengeid === this.input.challengeId,
                    `Inserted message at index ${i} has incorrect ChallengeID`);
                assert(upsertedMsg.messagename === inputMsg.messageName,
                    `Inserted message at index ${i} has incorrect MessageName`);
                assert.deepEqual(upsertedMsg.request, inputMsg.request,
                    `Inserted message at index ${i} has incorrect Request`);
                assert.deepEqual(upsertedMsg.expected, inputMsg.expected,
                    `Inserted message at index ${i} has incorrect Expected`);
                assert.deepEqual(upsertedMsg.response, inputMsg.response,
                    `Inserted message at index ${i} has incorrect Response`);
                assert(upsertedMsg.statusmsg === inputMsg.statusMsg,
                    `Inserted message at index ${i} has incorrect StatusMsg`);
            }
        });
        const { written } = res[0];
        if (! written) {
            throw new AuthError(
                `Cannot upsert challenge and insert messages, `
                + `integrity validation failed.`
            );
        }
    }
}