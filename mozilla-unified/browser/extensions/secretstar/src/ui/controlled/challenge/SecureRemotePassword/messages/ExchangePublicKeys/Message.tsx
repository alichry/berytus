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
    onSubmit(
        serverPublicKeyHexB: string,
        clientPublicKeyHexA: string,
        clientPrivateKeyHexa: string
    ): void;
}

const payloadSchema = string()
    .label("ServerPublicKeyHexA")
    .required();

export default function Message({ challenge, message, onSubmit }: MessageProps) {
    const [error, setError] = useState<Error>();
    const [processed, setProcessed] = useState<boolean>();
    const { value: serverPublicKeyHexB, error: validationError, loading: validationLoading  } = useYupValidation(payloadSchema, message.payload);
    useEffect(() => {
        if (! serverPublicKeyHexB || processed) {
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
                const clientPublicKeyHexA = client.computeA()
                    .toString('hex');
                // const client = new JsrpClient();

                // await client.init({
                //     username: challenge.srpState.fields.username,
                //     password: challenge.srpState.fields.password
                // });

                onSubmit(
                    serverPublicKeyHexB,
                    clientPublicKeyHexA,
                    clientPrivateKey.toString('hex')
                );
            } catch (e) {
                setError(e as Error);
            } finally {
                setProcessed(true);
            }
        }
        run();
    }, [serverPublicKeyHexB]);
    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { error || validationError ? (
                <ConcatErrorMessages errors={[error, validationError]} />
            ) : <MdCenteredSpinner className="mb-4" />
            }
        </RespondToMessageView>
    )
}