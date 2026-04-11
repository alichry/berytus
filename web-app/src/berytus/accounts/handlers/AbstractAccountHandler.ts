import { EStageHandlerType, type IAccountStageHandler, type IAccountStageState, type StepResult } from "@root/berytus/types";
import { FetchError } from "@root/backend/errors/FetchError";
import type { Body as CreateBody } from "@root/pages/channel/[channelId]/login/[category]/[version]/create/schema";
import type { AbstractChannelHandler } from "@root/berytus/channel/AbstractChannelHandler.js";
import { buildRequestBodyAndHeaders, type TargetContentType } from "@root/berytus/fetch-utils.js";

export abstract class AbstractAccountStageHandler<Step extends string> implements IAccountStageHandler {
    protected channelHandler: AbstractChannelHandler;
    protected channel?: BerytusChannel;
    protected operation?: BerytusAccountCreationOperation | BerytusAccountAuthenticationOperation;
    public readonly type!: EStageHandlerType.Account;
    protected readonly loginState: Omit<IAccountStageState, 'category' | 'version' | 'status'> = {
        userAttributes: {},
        identityFields: [],
        credentialFields: []
    };

    constructor(channelHandler: AbstractChannelHandler) {
        this.channelHandler = channelHandler;
    }

    get label(): string {
        return `${this.category}.V${this.version}`;
    }

    abstract get version(): number;

    abstract get category(): string;

    abstract get description(): string;

    abstract get steps(): readonly Step[];

    getChannel() {
        return this.channel;
    }

    async createChannel(): Promise<StepResult<Step>> {
        /*! Domain-based credential mapping actor */
        const actor = new BerytusAnonymousWebAppActor();
        //!
        const channel = await BerytusChannel.create({
            webApp: actor
        });
        //!
        //! EXPORT_FN_IGNORE_START
        this.channel = channel;
        return { finished: true as const };
        //! EXPORT_FN_IGNORE_END
    }

    getState() {
        if (! this.operation) {
            return;
            // return {
            //     channel: this.channel,
            //     userAttributes: {},
            //     identityFields: [],
            //     credentialFields: [],
            //     category: "",
            //     version: 0,
            //     status: "Pending" as const
            // };
        }
        let userAttrs: Record<string, string> = {};
        let identityFields: { id: string; value: string}[] = [];
        let credentialFields: { id: string; value: string}[] = [];
        if (this.operation.intent === "Register") {
            for (const [_k, { id, value }] of this.operation.userAttributes) {
                userAttrs[id] = typeof value === "string"
                    ? value
                    : JSON.stringify(value);
            }
            for (const [id, field] of this.operation.fields) {
                const value = field.value;
                if (field.type === "Identity" || field.type === "ForeignIdentity") {
                    identityFields.push({
                        id,
                        value: typeof value === "string"
                            ? value : JSON.stringify(value),
                    });
                    continue;
                }
                credentialFields.push({
                    id,
                    value: typeof value === "string"
                        ? value : JSON.stringify(value),
                });
            }
            return {
                channel: this.channel,
                userAttributes: userAttrs,
                identityFields,
                credentialFields,
                category: this.operation.category,
                version: this.operation.version,
                status: this.operation.status,
            };
        }
        return {
            channel: this.channel,
            category: this.operation.category,
            version: this.operation.version,
            status: this.operation.status,
            ...this.loginState
        }
    }

    async accountExists(
        fields: BerytusField[],
        targetContentType: TargetContentType = "multipart"
    ): Promise<boolean> {
        if (fields.some(field => field.type !== "ForeignIdentity" && field.type !== "Identity")) {
            throw new Error('Bad field passed. Only Identity and ForeignIdentity fields are allowed.');
        }
        const res = await fetch(
            `/channel/${this.channel!.id}/login/${this.category}/${this.version}/exists`,
            {
                method: "POST",
                ...buildRequestBodyAndHeaders({
                    fields: fields.map(field => ({ id: field.id, value: field.value }))
                }, targetContentType)
            }
        );
        if (! res.ok) {
            throw new Error('HTTP request failed.');
        }
        const body = await res.json();
        if (!("exists" in body) || typeof body.exists !== "boolean") {
            throw new Error('Malformed HTTP response');
        }
        return body.exists;
    }

    async createAccount(
        fields: CreateBody["fields"],
        userAttributes: Record<string, string>,
        targetContentType: TargetContentType = "multipart"
    ): Promise<void> {
        // for (const field of this.operation!.fields.values()) {
        //     fields.push({
        //         id: field.id,
        //         value: field.value
        //     })
        // }
        const body: CreateBody = {
            fields,
            userAttributes
        };
        const resp = await fetch(
            `/channel/${this.channel!.id}/login/${this.category}/${this.version}/create`,
            {
                method: "POST",
               ...buildRequestBodyAndHeaders(body, targetContentType)
            }
        )
        if (! resp.ok) {
            throw new FetchError(resp, 'Account creation request failed.');
        }
    }

    static assertIsAuthenticationOperation(
        operation?: BerytusAccountCreationOperation | BerytusAccountAuthenticationOperation
    ): asserts operation is BerytusAccountAuthenticationOperation {
        if (! operation) {
            throw new Error('Operation is not initialied');
        }
        if (operation.intent !== "Authenticate") {
            throw new Error('Expecting operation to be an authentication one.');
        }
    }
    static assertIsCreationOperation(
        operation?: BerytusAccountCreationOperation | BerytusAccountAuthenticationOperation
    ): asserts operation is BerytusAccountCreationOperation {
        if (! operation) {
            throw new Error('Operation is not initialied');
        }
        if (operation.intent !== "Register") {
            throw new Error('Expecting operation to be an authentication one.');
        }
    }
}

// @ts-ignore type is readonly
AbstractAccountStageHandler.prototype.type = EStageHandlerType.Account;