import type { FieldInput } from "@root/backend/db/types";
import { FetchError } from "@root/backend/errors/FetchError";
import { Result as NewResult } from "@root/pages/login/[category]/[version]/id/schema";
import { Result as PendingResult } from "@root/pages/login/[category]/[version]/auth/[sessionId]/challenge/[challengeId]/pending-message/schema";
import { Result as ProcessMessageResult } from "@root/pages/login/[category]/[version]/auth/[sessionId]/challenge/[challengeId]/respond-message/schema";
import { Result as FinishResult } from "@root/pages/login/[category]/[version]/auth/[sessionId]/finish/schema";

// TODO: Check if we still have to change prototype.name
export class AuthError extends Error {}
AuthError.prototype.name = 'AuthError';
export class AuthAccountNotFoundError extends AuthError {}
AuthAccountNotFoundError.prototype.name = 'AuthAccountNotFoundError';
export class AuthIncorrectResponseError extends AuthError {}
AuthIncorrectResponseError.prototype.name = "AuthIncorrectResponseError";

const populateFormData = (fdata: FormData, value: unknown, key?: string) => {
    if (typeof value === "object" && value !== null && !(value instanceof Blob)) {
        for (const [k, v] of Object.entries(value)) {
            if (/(\[|\])/.test(String(k))) {
                throw new Error(
                    `Keys cannot contain square brackets when `
                    + `using multipart form data. Invalid key: ${k}`
                );
            }
            populateFormData(fdata, v, key ? `${key}[${k}]` : k);
        }
        return;
    }
    if (value instanceof ArrayBuffer) {
        fdata.append(key!, new Blob([value], { type: "application/octet-stream" }));
        return;
    }
    if (ArrayBuffer.isView(value) || value instanceof DataView) {
        throw new Error(
            "Expecting ArrayBuffer to be passed for binary data. "
            + "Typed arrays and DataViews are not supported."
        );
    }
    // @ts-ignore: Browser implementation should
    // convert value to string if necessary.
    fdata.append(key!, value);
}

export class AuthSessionHandler {
    readonly accountVersion: number;
    readonly accountCategory: string;
    readonly sessionId: BigInt;
    currentChallengeId?: string;
    lastChallengeOutcome?: ProcessMessageResult['outcome'];

    protected constructor(
        accountVersion: number,
        accountCategory: string,
        sessionId: BigInt
    ) {
        this.accountVersion = accountVersion;
        this.accountCategory = accountCategory;
        this.sessionId = sessionId;
    }

    static async create(
        accountVersion: number,
        accountCategory: string,
        accountIdentity: FieldInput[]
    ) {
        const resp = await fetch(
            `/login/${accountCategory}/${accountVersion}/id`,
            {
                method: "POST",
                body: JSON.stringify({
                    accountVersion,
                    fields: accountIdentity
                })
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
        let data: NewResult;
        try {
            data = NewResult.parse(await resp.json());
        } catch (e) {
            throw new Error(
                'Unable to create AuthSessionHandler. Malformed HTTTP response.'
            );
        }
        return new AuthSessionHandler(
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
            `/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${challengeId}/new`,
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
        this.currentChallengeId = challengeId;
        this.lastChallengeOutcome = "Pending";
    }

    async pendingMessage() {
        if (! this.currentChallengeId) {
            throw new Error(
                "Cannot rerieve pending message, challenge is not active"
            );
        }
        const resp = await fetch(
            `/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${this.currentChallengeId}/pending-message`,
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
        contentType: "json" | "multipart" | "blob" = "multipart"
    ) {
        if (! this.currentChallengeId) {
            throw new Error(
                "Cannot send response, challenge is not active"
            );
        }
        let body, contentTypeHeader;
        if (contentType === "json") {
            body = JSON.stringify(response);
            contentTypeHeader = "application/json";
        } else if (contentType === "multipart") {
            if (typeof response !== "object" || response === null) {
                throw new Error(
                    "Expecting response to be a non-null object "
                    + "when contentType is multipart."
                );
            }
            body = new FormData();
            contentTypeHeader = "multipart/form-data";
            populateFormData(body, response);
        } else {
            if (! (response instanceof Blob)) {
                throw new Error(
                    "Expecting response to be a Blob when contentType is blob."
                );
            }
            body = response;
            contentTypeHeader = response.type;
        }
        const resp = await fetch(
            `/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/challenge/${this.currentChallengeId}/respond-message`,
            {
                method: "POST",
                headers: {
                    ['Content-Type']: contentTypeHeader
                },
                body
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
        }
        if (data.statusMsg !== 'Ok') {
            throw new AuthIncorrectResponseError(data.statusMsg);
        }
    }

    async finish(): Promise<FinishResult> {
        const resp = await fetch(
            `/login/${this.accountCategory}/${this.accountVersion}/auth/${this.sessionId}/finish`,
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