import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";
import { AbstractChannelHandler, EChannelState, type WebAppActor } from "../AbstractChannelHandler.js";

export class E2EEHandler extends AbstractChannelHandler {
    #state: EChannelState = EChannelState.Default;
    #webAppActor: WebAppActor | null = null;
    #channelRequestId: string | null = null;

    get state(): EChannelState {
        return this.#state
    }

    get webAppActor(): WebAppActor | null {
        return this.#webAppActor;
    }

    async prepare(): Promise<WebAppActor> {
        if (this.#state !== EChannelState.Default) {
            throw new Error(`Invalid state: expected ${EChannelState.Default}, got ${this.#state}`);
        }
        const resp = await this.createRequest({
            type: "E2EE"
        });
        if (!("ed25519Key" in resp.webAppActor)) {
            throw new InvalidArgError("Invalid webAppActor format; expected ed25519Key to be set.");
        }
        this.#webAppActor = resp.webAppActor;
        this.#state = EChannelState.Prepared;
        this.#channelRequestId = resp.channelRequestId;
        return this.#webAppActor;
    }

    async init(nativeChannel: BerytusChannel): Promise<void> {
        if (this.#state !== EChannelState.Prepared) {
            throw new Error(`Invalid state: expected ${EChannelState.Prepared}, got ${this.#state}`);
        }
        if (nativeChannel.secretManager === null) {
            throw new Error(`Expected Crypto SCM Actor to be set, got otherwise.`);
        }
        await this.createChannel({
            channelId: nativeChannel.id,
            channelRequestId: this.#channelRequestId!,
            scmActor: { ed25519Key: nativeChannel.secretManager.ed25519Key }
        });
        this.#state = EChannelState.Inited;
    }
}