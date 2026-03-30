import {
    Body as CreateChannelRequestBody,
    Result as CreateChannelRequestResult
} from "@root/pages/channel/request/create/schema.js";
import {
    Body as CreateChannelBody
} from "@root/pages/channel/create/schema.js";
import { IllegalStateError } from "@root/backend/errors/IllegalStateError";
import { FetchError } from "@root/backend/errors/FetchError";

export enum EChannelHandlerState {
    Default = 'Default',
    Prepared = 'Prepared',
    Inited = 'Inited',
    Closed = 'Closed'
};

export type WebAppActor = CreateChannelRequestResult["webAppActor"];

export abstract class AbstractChannelHandler {
    state: EChannelHandlerState = EChannelHandlerState.Default;

    /**
     * The web app actor that must be used when creating the
     * native channel. This is provided by the web app backend.
     * It is set after calling prepare().
     */
    webAppActor?: WebAppActor;

    /**
     * The channel request id. It is set after calling prepare()
     */
    channelRequestId?: string;

    /**
     * The native channel. It set after calling init()
     */
    channel?: BerytusChannel;

    /**
     * Prepare for a native channel instantiation. This step
     * sends a channel creation request to the backend and
     * returns the web app actor that must be used to instantiate
     * the channel.
     */
    abstract prepare(): Promise<WebAppActor>;

    protected async prepareImpl(
        body: CreateChannelRequestBody
    ): Promise<CreateChannelRequestResult> {
        if (this.state !== EChannelHandlerState.Default) {
            throw new IllegalStateError(
                "Cannot prepare channel, handler state is not Default; got "
                + this.state
            );
        }
        const resp = await this.createRequest(body);
        this.webAppActor = resp.webAppActor;
        this.state = EChannelHandlerState.Prepared;
        this.channelRequestId = resp.channelRequestId;
        return resp;
    }

    public async init(nativeChannel: BerytusChannel): Promise<void> {
        if (this.state !== EChannelHandlerState.Prepared) {
            throw new IllegalStateError(`Invalid state: expected ${EChannelHandlerState.Prepared}, got ${this.state}`);
        }
        await this.createChannel({
            channelId: nativeChannel.id,
            channelRequestId: this.channelRequestId!,
            // TODO(berytus): Update WebIDL, Channel.secretManager is always set
            scmActor: { ed25519Key: nativeChannel.secretManager!.ed25519Key }
        });
        this.state = EChannelHandlerState.Inited;
        this.channel = nativeChannel;
    }

    public async close(): Promise<void> {
        if (! this.channel) {
            throw new IllegalStateError("Cannot close channel, no channel has been initialised yet.");
        }
        if (this.channel.active === true) {
            throw new IllegalStateError("Cannot close channel, channel is not active.");
        }
        if (this.state !== EChannelHandlerState.Inited) {
            throw new IllegalStateError("Cannot close channel, handler state is not Inited; got " + this.state);
        }
        const result = await fetch(
            `/channel/${this.channel.id}/close`,
            {
                method: "POST"
            }
        );
        if (! result.ok) {
            throw new FetchError(
                result,
                `Failed to close channel`
            );
        }
        this.state = EChannelHandlerState.Closed;
    }

    private async createRequest(
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
            throw new FetchError(response, `Failed to create channel request`);
        }
        const responseBody =
            await response.json();
        return await CreateChannelRequestResult.parseAsync(responseBody);
    }

    private async createChannel(
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
            throw new FetchError(response, `Failed to create channel`);
        }
    }
}