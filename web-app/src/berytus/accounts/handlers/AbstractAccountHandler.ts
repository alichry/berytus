import { EStageHandlerType, type IAccountStageHandler, type IAccountStageState, type StepResult } from "@root/berytus/types";
import { FetchError } from "@root/backend/errors/FetchError";
import type { Body as CreateBody } from "@root/pages/login/[category]/[version]/create/schema";

export abstract class AbstractAccountStageHandler<Step extends string> implements IAccountStageHandler {
    protected channel?: BerytusChannel;
    protected operation?: BerytusAccountCreationOperation | BerytusAccountAuthenticationOperation;
    public readonly type!: EStageHandlerType.Account;
    protected readonly loginState: Omit<IAccountStageState, 'category' | 'version' | 'status'> = {
        userAttributes: {},
        identityFields: [],
        credentialFields: []
    };

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
            for (const [k, { id, value }] of this.operation.userAttributes) {
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

    async accountExists(field: BerytusField): Promise<boolean> {
        if (field.type !== "ForeignIdentity" && field.type !== "Identity") {
            throw new Error('Bad field passed.');
        }
        const res = await fetch(
            `/login/${this.category}/${this.version}/exists`,
            {
                method: "POST",
                headers: {
                    "content-type": "application/json"
                },
                body: JSON.stringify({
                    fields: [
                        { id: field.id, value: field.value }
                    ]
                })
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

    async createAccount(fields: { id: string; value: string }[], userAttributes: Record<string, string>): Promise<void> {
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
            `/login/${this.category}/${this.version}/create`,
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify(body)
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