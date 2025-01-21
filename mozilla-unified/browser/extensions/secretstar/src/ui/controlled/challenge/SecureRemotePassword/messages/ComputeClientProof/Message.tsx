import { isSrpChallenge } from "@root/db";
import { BaseMessageProps } from "../../../common/types";
import { useEffect, useState } from "react";
import JsrpClient from "@root/JsrpClient";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import { SRP, SrpClient } from "fast-srp-hap";
import { string } from "yup";
import { useYupValidation } from "@root/hooks";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";

export interface MessageProps extends BaseMessageProps {
    onSubmit(salt: string, clientProof: string): void;
}

const payloadSchema = string()
    .label("Salt")
    .required();

export default function Message({ challenge, message, onSubmit }: MessageProps) {
    const [error, setError] = useState<Error>();
    const [processed, setProcessed] = useState<boolean>();
    const { value: salt, error: validationError, loading: validationLoading  } = useYupValidation(payloadSchema, message.payload);
    useEffect(() => {
        if (! salt || processed) {
            return;
        }
        const run = async () => {
            try {
                if (! isSrpChallenge(challenge)) {
                    setError(new Error("Passed challenge is not an SRP challenge"));
                    return;
                }
                const { clientPrivateKeyHexa, serverPublicKeyHexB } = challenge.srpState;
                if (! clientPrivateKeyHexa) {
                    setError(new Error("Passed challenge does not have the client private key set!"));
                    return;
                }
                if (! serverPublicKeyHexB) {
                    setError(new Error("Passed challenge does not have the server public key set!"));
                    return;
                }

                const client = new SrpClient(SRP.params[4096],
                    Buffer.from(salt, 'hex'),
                    Buffer.from(challenge.srpState.fields.username, 'ascii'),
                    Buffer.from(challenge.srpState.fields.password, 'ascii'),
                    Buffer.from(clientPrivateKeyHexa, 'hex'),
                    false
                );

                client.setB(Buffer.from(serverPublicKeyHexB, 'hex'));
                const clientProof = client.computeM1();

                onSubmit(salt, clientProof.toString('hex'));
            } catch (e) {
                console.error(e);
                setError(e as Error);
            } finally {
                setProcessed(true);
            }
        }
        run();
    }, [salt]);
    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { error || validationError ? (
                <ConcatErrorMessages errors={[error, validationError]} />
            ) : <MdCenteredSpinner className="mb-4" />
            }
        </RespondToMessageView>
    )
}