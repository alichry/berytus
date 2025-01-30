import { ClassNameProp } from "@root/ui/props/ClassNameProp";
import ApproveOrReject from "@root/ui/components/ApproveOrReject";
import ChallengeButtonTextWithIcon from "@root/ui/components/ChallengeButtonTextWithIcon";
import { faPaperPlane } from "@fortawesome/free-solid-svg-icons";
import { useAccount, useAccountFields, useTryOnce } from "@root/hooks";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import LabeledInput from "@root/ui/components/LabeledInput";
import { useState, useEffect } from "react";
import ConcatErrorMessages from "@root/ui/components/ConcatErrorMessages";
import { faClockRotateLeft } from "@fortawesome/free-solid-svg-icons";
import { base64ToArrayBuffer, extractPemBase64Body, formatBase64AsPem, pemToBuf, privateKeyBufToPublicKeyBuf } from "@root/key-utils";

export interface FormProps extends ClassNameProp {
    accountId?: string;
    keyFieldId: string;
    seamless: boolean;
    onSubmit(privateKeyBuf: ArrayBuffer, publicKeyBuf: ArrayBuffer): void;
}

export default function Form({ accountId, keyFieldId, className, seamless, onSubmit }: FormProps) {
    const { account, error: accountError } = useAccount(accountId);
    const { fields: keyField, error: fieldsError } = useAccountFields(account, keyFieldId);
    const [submissionError, setSubmissionError] = useState<Error>();
    const [customPrivateKeyStr, setCustomPrivateKeyStr] = useState<string>();
    const [publicKey, setPublicKey] = useState<string>();

    const submit = async (privateKeyPem: string) => {
        try {
            const base64Body = extractPemBase64Body(privateKeyPem, false);
            const privateKeyBuf = base64ToArrayBuffer(base64Body);
            // BRTTODO: for some reason, pemToBuf(privateKeyPem) produces a bigger buffer (500 extra bytes)
            // than privateKeyBuf. Using UTF-8 decoder to convert a binary string to
            // buffer seems like a bad idea. base64ToArrayBuffer avoids thads.
            const publicKeyBuf = await privateKeyBufToPublicKeyBuf(privateKeyBuf);
            onSubmit(privateKeyBuf, publicKeyBuf);
        } catch (e) {
            setSubmissionError(e as Error);
        }
    }

    useEffect(() => {
        if (customPrivateKeyStr || ! keyField) {
            return;
        }
        const privateKeyPem = formatBase64AsPem(keyField.value, false);
        setCustomPrivateKeyStr(privateKeyPem);
    }, [keyField]);
    const { tried, error: seamlessError } = useTryOnce(seamless, async () => {
        if (! keyField) {
            return false;
        }
        const privateKeyPem = formatBase64AsPem(keyField.value, false);
        submit(privateKeyPem);
        return true;
    }, [keyField]);
    const loading = ! account || ! keyField || ! tried;
    const errors = [accountError, fieldsError, submissionError, seamlessError];

    if (loading) {
        return <MdCenteredSpinner />;
    }
    const showReset = customPrivateKeyStr !== formatBase64AsPem(keyField.value, false);

    return (
        <div>
            {
                keyField ? <p className="text-sm mb-4">
                    The web application requested the stored public key. Send public key?
                </p> : null
            }
            { errors.find(e => !!e) ? (
                <ConcatErrorMessages errors={errors} />
            ) : null}
            <LabeledInput
                className="mb-5"
                label="#privateKey"
                type="textarea"
                textAreaColor="light"
                value={customPrivateKeyStr}
                onChange={(e) => {
                    setCustomPrivateKeyStr(e.target.value);
                }}
                required
            />
            <ApproveOrReject
                approveText={
                    <ChallengeButtonTextWithIcon
                        text="Send"
                        icon={faPaperPlane}
                    />
                }
                onApprove={() => {
                    if (! customPrivateKeyStr) {
                        setSubmissionError(new Error("Invalid private key string!"));
                        return;
                    }
                    submit(customPrivateKeyStr);
                }}
                order="rejectFirst"
                onReject={showReset ? () => {
                    setCustomPrivateKeyStr(formatBase64AsPem(keyField.value, false))
                } : undefined}
                rejectText={
                    showReset ? <ChallengeButtonTextWithIcon
                        text="Reset"
                        icon={faClockRotateLeft}
                    /> : undefined
                }
                rejectProps={{
                    color: "teal"
                }}
            />
        </div>
    )
    // return (
    //     <EditableLabeledInputListForm
    //         loading={loading}
    //         errors={errors}
    //         items={[
    //             {
    //                 id: "privateKey",
    //                 defaultValue: "",
    //                 required: true,
    //                 type: "textarea",
    //                 label: "OTP"
    //             }
    //         ]}
    //         header={
    //             keyField ? <p className="text-sm">
    //                 The web application requested the stored public key.
    //             </p> : null
    //         }
    //         onApprove={({ otp }) => {
    //             if (otp === undefined) {
    //                 setSubmissionError(new Error(
    //                     'OTP is unset. Kindly input an OTP code.'
    //                 ));
    //                 return;
    //             }
    //             onSubmit(otp);
    //         }}
    //     />
    // );
}