import { Result } from "@root/pages/channel/[channelId]/keyagreement/sign/schema.js";
import { AbstractChannelHandler, type WebAppActor } from "../AbstractChannelHandler.js";
import { IllegalStateError } from "@root/backend/errors/IllegalStateError.js";
import { FetchError } from "@root/backend/errors/FetchError.js";

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

    async signKap(): Promise<ArrayBuffer> {
        if (! this.channel) {
            throw new IllegalStateError(
                "Channel not initialized; cannot sign KAP."
            );
        }
        if (! this.channel.keyAgreementParams) {
            throw new IllegalStateError(
                "Channel KAP not prepared; cannot sign KAP."
            );
        }
        const res = await fetch(`/channel/${this.channel.id}/keyagreement/sign`, {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            },
            body: JSON.stringify({
                canonicalJson: this.channel.keyAgreementParams.toCanonicalJSON()
            })
        });
        if (! res.ok) {
            throw new FetchError(res, "Failed to sign KAP");
        }
        const body = await res.json();
        await Result.parseAsync(body);
        const b64 = body.signature;
        return Uint8Array
            // @ts-ignore: toBase64 is available in modern browsers
            .fromBase64(b64)
            .buffer;
    }

    async verifyScmKapSignature(scmSignature: ArrayBuffer): Promise<void> {
        if (! this.channel) {
            throw new IllegalStateError(
                "Channel not initialized; cannot verify Scm KAP Signature."
            );
        }
        if (! this.channel.keyAgreementParams) {
            throw new IllegalStateError(
                "Channel KAP not prepared; cannot verify Scm KAP Signature."
            );
        }
        const res = await fetch(
            `/channel/${this.channel.id}/keyagreement/verify`,
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    signature: new Uint8Array(scmSignature)
                        // @ts-ignore: toBase64 is available in modern browsers
                        .toBase64()
                })
            }
        );
        if (! res.ok) {
            throw new FetchError(res, "Failed to verify SCM KAP Signature");
        }
    }

    async deriveSessionKey(): Promise<void> {
        if (! this.channel) {
            throw new IllegalStateError(
                "Channel not initialized; cannot derive session key."
            );
        }
        if (! this.channel.keyAgreementParams) {
            throw new IllegalStateError(
                "Channel KAP not prepared; cannot derive session key."
            );
        }
        const res = await fetch(
            `/channel/${this.channel.id}/keyagreement/derive`,
            {
                method: "POST"
            }
        );
        if (! res.ok) {
            throw new FetchError(res, "Failed to derive session key");
        }
    }
}