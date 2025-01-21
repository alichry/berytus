import { useState, useEffect } from "react";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import { mixed, ValidationError } from "yup";
import { BaseMessageProps } from "../../../common/types";
import Form from './Form';
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import { useYupValidation } from "@root/hooks";
import { isDsChallenge } from "@root/db";
import { base64ToArrayBuffer, extractPemBase64Body, pemToBuf } from "@root/key-utils";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";

export interface MessageProps extends BaseMessageProps {
    onSubmit(nonce: ArrayBuffer, signedMessage: ArrayBuffer): void;
}

const payloadSchema = mixed<ArrayBuffer>()
    .label("Nonce")
    .transform((val) => {
        if (typeof val !== "string") {
            throw new ValidationError('Nonce is not a string (base64)!');
        }
        return base64ToArrayBuffer(val);
    })
    .required();

export default function Message({
    session,
    challenge,
    message,
    settings,
    onSubmit
}: MessageProps) {
    const {
        loading: validationLoading,
        error: validationError,
        value: nonce
    } = useYupValidation(payloadSchema, message.payload);
    const [error, setError] = useState<Error>();
    const [privateKey, setPrivateKey] = useState<CryptoKey>();
    useEffect(() => {
        if (! isDsChallenge(challenge)) {
            setError(new Error("Passed challenge is not a Digital Siganture Challenge!"));
            return;
        }
        let run = async () => {
            const { privateKey } = challenge.dsState;
            const privateKeyBase64Body = extractPemBase64Body(privateKey, false);
            const privateKeyBuf = base64ToArrayBuffer(privateKeyBase64Body);
            //const privateKeyBuf = pemToBuf(privateKey, false);

            const key = await window.crypto.subtle.importKey(
                "pkcs8",
                privateKeyBuf,
                {
                    name: "RSASSA-PKCS1-v1_5",
                    hash: {name: "SHA-256"}
                },
                true,
                ["sign"]
            );
            setPrivateKey(key);
        }
        run();
    }, []);
    // const loading = validationLoading || ! privateKey;

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError || error ? (
                <ConcatErrorMessages errors={[validationError, error]} />
            ) : validationLoading || ! privateKey ? (
                <MdCenteredSpinner />
            ) : <Form
                    nonce={nonce}
                    privateKey={privateKey}
                    seamless={settings.seamless.login}
                    onSubmit={(v) => onSubmit(nonce, v)}
                />
            }
        </RespondToMessageView>
    )
}