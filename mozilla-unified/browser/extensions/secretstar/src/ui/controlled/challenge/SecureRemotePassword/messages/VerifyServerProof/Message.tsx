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
import Notice from "@root/ui/components/Notice";
import ApproveOrReject from "@root/ui/components/ApproveOrReject";

export interface MessageProps extends BaseMessageProps {
    onSubmit(proofValid: boolean): void;
}

const payloadSchema = string()
    .label("ServerProofHexM2")
    .required();

export default function Message({ challenge, message, settings, onSubmit }: MessageProps) {
    const [error, setError] = useState<Error>();
    const [processed, setProcessed] = useState<boolean>();
    const [proofOk, setProofOk] = useState<boolean>();
    const { value: serverProofHexM2, error: validationError, loading: validationLoading  } = useYupValidation(payloadSchema, message.payload);
    useEffect(() => {
        if (! serverProofHexM2 || processed) {
            return;
        }
        const run = async () => {
            try {
                if (! isSrpChallenge(challenge)) {
                    setError(new Error("Passed challenge is not an SRP challenge"));
                    return;
                }
                const { clientPrivateKeyHexa, serverPublicKeyHexB, salt } = challenge.srpState;
                if (! clientPrivateKeyHexa) {
                    setError(new Error("Passed challenge does not have the client private key set!"));
                    return;
                }
                if (! serverPublicKeyHexB) {
                    setError(new Error("Passed challenge does not have the server public key set!"));
                    return;
                }
                if (! salt) {
                    setError(new Error("Passed challenge does not have the server salt set!"));
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
                try {
                    client.checkM2(Buffer.from(serverProofHexM2, 'hex'));
                } catch (e) {
                    setProofOk(false);
                    return;
                }
                setProofOk(true);
            } catch (e) {
                console.error(e);
                setError(e as Error);
            } finally {
                setProcessed(true);
            }
        }
        run();
    }, [serverProofHexM2]);
    useEffect(() => {
        if (proofOk === true && settings.seamless.login) {
            onSubmit(proofOk);
        }
    }, [proofOk]);
    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { error || validationError ? (
                <ConcatErrorMessages errors={[error, validationError]} />
            ) : proofOk === undefined ?
                <MdCenteredSpinner className="mb-4" />
            : proofOk ? (
                <Notice
                    type="info"
                    text="Great news! The SRP authentication succeeded with mutual authentication of both parties 🎉"
                />
            ) : (
                <Notice
                    type="error"
                    text="The web application successfully validated our client proof, however, their proof was invalid ❌"
                />
            )}
            {proofOk !== undefined ? (
                <ApproveOrReject
                    order="rejectFirst"
                    approveText="Ok"
                    onApprove={() => {
                        onSubmit(proofOk);
                    }}
                />
            ) : null}
        </RespondToMessageView>
    )
}