import { ab2base64, formatBase64AsPem } from "@root/key-utils";
import ApproveOrReject from "@root/ui/components/ApproveOrReject";
import ChallengeButtonTextWithIcon from "@root/ui/components/ChallengeButtonTextWithIcon";
import LabeledInput from "@root/ui/components/LabeledInput";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import { useState, useEffect } from "react";
import { faPaperPlane } from "@fortawesome/free-solid-svg-icons";
import { useTryOnce } from "@root/hooks";
import { atLeastOneErrorSet } from "@root/utils";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";

export interface FormProps {
    nonce: ArrayBuffer;
    privateKey: CryptoKey;
    seamless: boolean;
    onSubmit(signedMessage: ArrayBuffer): void;
}

export default function Form({ nonce, privateKey, onSubmit, seamless }: FormProps) {
    const [privateKeyPem, setPrivateKeyPem] = useState<string>();
    const [signedMessage, setSignedMessage] = useState<ArrayBuffer>();
    const [error, setError] = useState<Error>();
    useEffect(() => {
        const run = async () => {
            try {
                const res = await window.crypto.subtle.sign(
                    "RSASSA-PKCS1-v1_5",
                    privateKey,
                    nonce
                );
                setSignedMessage(res);
            } catch (e) {
                setError(e as Error);
            }
        }
        run();
    }, []);
    const { tried, error: seamlessError } = useTryOnce(seamless, async () => {
        if (! signedMessage) {
            return false;
        }
        onSubmit(signedMessage);
        return true;
    }, [signedMessage]);
    useEffect(() => {
        if (! privateKey) {
            setPrivateKeyPem('');
            return;
        }
        const run = async() => {
            const buf = await window.crypto.subtle.exportKey(
                "pkcs8",
                privateKey
            );
            const pem = formatBase64AsPem(ab2base64(buf), false);
            setPrivateKeyPem(pem);
        };
        run();
    }, [privateKey]);
    const loading = ! signedMessage || ! tried;
    const errors = [error, seamlessError];
    if (atLeastOneErrorSet(errors)) {
        return <ConcatErrorMessages errors={errors} />
    }
    if (loading) {
        return <MdCenteredSpinner className="mb-4" />;
    }
    return (
        <div>
            {/* <LabeledInput
                className="mb-4"
                inputClassName="cursor-not-allowed"
                type="textarea"
                label="#signingKey"
                value={privateKeyPem || ""}
                disabled
            /> */}
            <LabeledInput
                className="mb-4"
                type="textarea"
                label="signature (base64 encoded)"
                inputClassName="!cursor-not-allowed"
                value={ab2base64(signedMessage)}
                disabled
            />
            <ApproveOrReject
                approveText={
                    <ChallengeButtonTextWithIcon
                        text="Send"
                        icon={faPaperPlane}
                    />
                }
                onApprove={async () => {
                    try {
                        await Promise.resolve(onSubmit(signedMessage));
                    } catch (e) {
                        setError(e as Error);
                    }
                }}
                order="rejectFirst"
                rejectProps={{
                    color: "teal"
                }}
            />
        </div>
    );
}