import { isSrpChallenge } from "@root/db";
import { BaseMessageProps } from "../../../common/types";
import { useEffect, useState } from "react";
import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import { SRP, SrpClient } from "fast-srp-hap";
import { string } from "yup";
import { useYupValidation } from "@root/hooks";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";

export interface MessageProps extends BaseMessageProps {
    onSubmit(
        serverPublicKeyB: string,
        clientPublicKeyA: string,
        clientPrivateKeya: string
    ): void;
}

const payloadSchema = string()
    .label("ServerPublicKeyA")
    .required();

export default function Message({ challenge, message, onSubmit }: MessageProps) {
    const [error, setError] = useState<Error>();
    const [processed, setProcessed] = useState<boolean>();
    const { value: serverPublicKeyB, error: validationError, loading: validationLoading  } = useYupValidation(payloadSchema, message.payload);
    useEffect(() => {
        if (! serverPublicKeyB || processed) {
            return;
        }
        const run = async () => {
            try {
                if (! isSrpChallenge(challenge)) {
                    console.log(challenge);
                    setError(new Error("Passed challenge is not an SRP challenge"));
                    return;
                }

                const clientPrivateKey = await SRP.genKey(32);
                const client = new SrpClient(SRP.params[4096],
                    Buffer.from("hehe_library_no_flexi"),
                    Buffer.from(challenge.srpState.fields.username),
                    Buffer.from(challenge.srpState.fields.password),
                    clientPrivateKey,
                    false
                );
                const clientPublicKeyA = client.computeA()
                    .toString('base64');

                onSubmit(
                    serverPublicKeyB,
                    clientPublicKeyA,
                    clientPrivateKey.toString('base64')
                );
            } catch (e) {
                setError(e as Error);
            } finally {
                setProcessed(true);
            }
        }
        run();
    }, [serverPublicKeyB]);
    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { error || validationError ? (
                <ConcatErrorMessages errors={[error, validationError]} />
            ) : <MdCenteredSpinner className="mb-4" />
            }
        </RespondToMessageView>
    )
}