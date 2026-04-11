import type { FieldInput } from "@root/backend/db/types";
import { FetchError } from "@root/backend/errors/FetchError";
import { Result as NewSessionResult } from "@root/pages/channel/[channelId]/login/[category]/[version]/id/schema";
import { Result as NewChallengeResult } from "@root/pages/channel/[channelId]/login/[category]/[version]/auth/[sessionId]/challenge/[challengeId]/new/schema";
import { Result as PendingResult } from "@root/pages/channel/[channelId]/login/[category]/[version]/auth/[sessionId]/challenge/[challengeId]/pending-message/schema";
import { Result as ProcessMessageResult } from "@root/pages/channel/[channelId]/login/[category]/[version]/auth/[sessionId]/challenge/[challengeId]/respond-message/schema";
import { Result as FinishResult } from "@root/pages/channel/[channelId]/login/[category]/[version]/auth/[sessionId]/finish/schema";
import { buildRequestBodyAndHeaders, type TargetContentType } from "@root/berytus/fetch-utils.js";

// TODO: Check if we still have to change prototype.name
export class AuthError extends Error {}
AuthError.prototype.name = 'AuthError';
export class AuthAccountNotFoundError extends AuthError {}
AuthAccountNotFoundError.prototype.name = 'AuthAccountNotFoundError';
export class AuthIncorrectResponseError extends AuthError {}
AuthIncorrectResponseError.prototype.name = "AuthIncorrectResponseError";

export class AuthSessionHandler {
    readonly channelId: string;
    readonly accountVersion: number;
    readonly accountCategory: string;
    readonly sessionId: BigInt;
    currentChallengeId?: string;
    currentChallengeParameters?: object;
    lastChallengeOutcome?: ProcessMessageResult['outcome'];

    protected constructor(
        channelId: string,
        accountVersion: number,
        accountCategory: string,
        sessionId: BigInt
    ) {
        this.channelId = channelId;
        this.accountVersion = accountVersion;
        this.accountCategory = accountCategory;
        this.sessionId = sessionId;
    }

    static async create(
        channelId: string,
        accountVersion: number,
        accountCategory: string,
        accountIdentity: FieldInput[],
        targetContentType: TargetContentType = "multipart"
    ) {
        const resp = await fetch(
            `/channel/${channelId}/login/${accountCategory}/${accountVersion}/id`,
            {
                method: "POST",
                ...buildRequestBodyAndHeaders({
                    fields: accountIdentity
                }, targetContentType)
            }
        )
        if (! resp.ok) {
            const base =  new FetchError(
                resp,
                'Unable to create AuthSessionHandler. Failing HTTP status code.'
            );
            if (resp.status === 404) {
                throw new AuthAccountNotFoundError(
                    "Account Not Found.",
                    { cause: base }
                );
            }
            throw base;
        }
        let data: NewSessionResult;
        try {
            data = NewSessionResult.parse(await resp.json());
        } catch (e) {
            throw new Error(
                'Unable to create AuthSessionHandler. Malformed HTTTP response.'
            );
        }
        return new AuthSessionHandler(
            channelId,
            accountVersion,
            accountCategory,
            BigInt(data.sessionId),
        );
    }

    async newChallenge(
        challengeId: string
    ) {
        if (this.currentChallengeId) {
            throw new Error(
                "Cannot start a new challenge. An existing challenge is pending"
            );
        }
        const resp = await fetch(
            `/channel/${this.channelId}/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${challengeId}/new`,
            {
                method: "POST"
            }
        )
        if (! resp.ok) {
            throw new FetchError(
                resp,
                'Unable to start a new challenge, received failing HTTP status code.'
            );
        }
        const result = await NewChallengeResult.parseAsync(
            await resp.json()
        );
        this.currentChallengeId = challengeId;
        this.currentChallengeParameters = result.parameters;
        this.lastChallengeOutcome = "Pending";
        return result.parameters;
    }

    async pendingMessage() {
        if (! this.currentChallengeId) {
            throw new Error(
                "Cannot rerieve pending message, challenge is not active"
            );
        }
        const resp = await fetch(
            `/channel/${this.channelId}/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${this.currentChallengeId}/pending-message`,
            {
                method: "GET"
            }
        )
        if (! resp.ok) {
            throw new FetchError(
                resp,
                'Unable to retrieve pending message, got failing HTTP status.'
            );
        }
        let data: PendingResult;
        try {
            data = PendingResult.parse(await resp.json());
        } catch (e) {
            throw new Error(
                'Unable to retrieve pending message, malformed HTTTP response.'
            );
        }
        return data;
    }

    async sendResponse(
        response: unknown,
        targetContentType: TargetContentType = "multipart"
    ) {
        if (! this.currentChallengeId) {
            throw new Error(
                "Cannot send response, challenge is not active"
            );
        }

        const resp = await fetch(
            `/channel/${this.channelId}/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${this.currentChallengeId}/respond-message`,
            {
                method: "POST",
                ...buildRequestBodyAndHeaders(response, targetContentType)
            }
        )
        if (! resp.ok) {
            throw new FetchError(
                resp,
                'Send message response error, got failing HTTP status.'
            );
        }
        // response was accepted, let's check the outcome
        let data: ProcessMessageResult;
        try {
            data = ProcessMessageResult.parse(await resp.json());
        } catch (e) {
            throw new Error(
                'Unable to parse challenge outcome, malformed HTTTP response.'
            );
        }
        this.lastChallengeOutcome = data.outcome;
        if (data.outcome !== "Pending") {
            this.currentChallengeId = undefined;
            this.currentChallengeParameters = undefined;
        }
        if (data.statusMsg !== 'Ok') {
            throw new AuthIncorrectResponseError(data.statusMsg);
        }
    }

    async finish(): Promise<FinishResult> {
        const resp = await fetch(
            `/channel/${this.channelId}/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/finish`,
            {
                method: "POST"
            }
        )
        if (! resp.ok) {
            throw new FetchError(
                resp,
                'Send finish error, got failing HTTP status.'
            );
        }
        let data: FinishResult;
        try {
            data = FinishResult.parse(await resp.json());
        } catch (e) {
            throw new Error(
                'Unable to send finish, malformed HTTTP response.'
            );
        }
        return data;
    }
}