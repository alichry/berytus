import { AbstractChannelHandler, type WebAppActor } from "../AbstractChannelHandler.js";
import { IllegalStateError } from "@root/backend/errors/IllegalStateError.js";

export class E2EEHandler extends AbstractChannelHandler {
    async prepare(): Promise<WebAppActor> {
        const resp = await this.prepareImpl({
            type: "E2EE"
        });
        if (!("ed25519Key" in resp.webAppActor)) {
            throw new Error(
                "Backend sent an invalid web app actor; expected ed25519Key to be set."
            );
        }
        return resp.webAppActor;
    }

    async init(nativeChannel: BerytusChannel): Promise<void> {
        if (! nativeChannel.constraints.enableEndToEndEncryption) {
            throw new IllegalStateError(
                `Expected Channel to be an E2EE Channel; got otherwise.`
            );
        }
        await super.init(nativeChannel);
    }
}