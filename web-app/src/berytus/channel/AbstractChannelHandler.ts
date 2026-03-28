import {
    Body as CreateChannelRequestBody,
    Result as CreateChannelRequestResult
} from "@root/pages/channel/request/create/schema.js";
import {
    Body as CreateChannelBody
} from "@root/pages/channel/create/schema.js";

export enum EChannelState {
    Default = 'Default',
    Prepared = 'Prepared',
    Inited = 'Inited'
};

export type WebAppActor = CreateChannelRequestResult["webAppActor"];

export abstract class AbstractChannelHandler {
    abstract get state(): EChannelState;

    abstract get webAppActor(): WebAppActor | null;

    abstract prepare(): Promise<WebAppActor>;

    abstract init(nativeChannel: BerytusChannel): Promise<void>;

    protected async createRequest(
        body: CreateChannelRequestBody
    ): Promise<CreateChannelRequestResult> {
        const response = await fetch(
            "/channel/request/create",
            {
                method: "POST",
                body: JSON.stringify(body),
                headers: {
                    "Content-Type": "application/json"
                }
            }
        );
        if (! response.ok) {
            throw new Error(`Failed to create channel request: ${response.statusText}`);
        }
        const responseBody =
            await response.json();
        return await CreateChannelRequestResult.parseAsync(responseBody);
    }

    protected async createChannel(
        body: CreateChannelBody
    ): Promise<void> {
        const response = await fetch(
            "/channel/create",
            {
                method: "POST",
                body: JSON.stringify(body),
                headers: {
                    "Content-Type": "application/json"
                }
            }
        );
        if (! response.ok) {
            throw new Error(`Failed to create channel: ${response.statusText}`);
        }
    }
}