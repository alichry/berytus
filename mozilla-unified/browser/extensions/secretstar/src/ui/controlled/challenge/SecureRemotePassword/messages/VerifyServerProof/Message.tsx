import { isSrpChallenge } from "@root/db";
import { BaseMessageProps } from "../../../common/types";
import { useEffect, useState } from "react";
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
    .label("ServerProofM2")
    .required();

export default function Message({ challenge, message, settings, onSubmit }: MessageProps) {
    const [error, setError] = useState<Error>();
    const [processed, setProcessed] = useState<boolean>();
    const [proofOk, setProofOk] = useState<boolean>();
    const { value: serverProofM2, error: validationError, loading: validationLoading  } = useYupValidation(payloadSchema, message.payload);
    useEffect(() => {
        if (! serverProofM2 || processed) {
            return;
        }
        const run = async () => {
            try {
                if (! isSrpChallenge(challenge)) {
                    setError(new Error("Passed challenge is not an SRP challenge"));
                    return;
                }
                const { clientPrivateKeya, serverPublicKeyB, salt } = challenge.srpState;
                if (! clientPrivateKeya) {
                    setError(new Error("Passed challenge does not have the client private key set!"));
                    return;
                }
                if (! serverPublicKeyB) {
                    setError(new Error("Passed challenge does not have the server public key set!"));
                    return;
                }
                if (! salt) {
                    setError(new Error("Passed challenge does not have the server salt set!"));
                    return;
                }

                const client = new SrpClient(SRP.params[4096],
                    Buffer.from(salt, 'base64'),
                    Buffer.from(challenge.srpState.fields.username, 'ascii'),
                    Buffer.from(challenge.srpState.fields.password, 'ascii'),
                    Buffer.from(clientPrivateKeya, 'base64')
                );

                client.setB(Buffer.from(serverPublicKeyB, 'base64'));
                try {
                    client.checkM2(Buffer.from(serverProofM2, 'base64'));
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
    }, [serverProofM2]);
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