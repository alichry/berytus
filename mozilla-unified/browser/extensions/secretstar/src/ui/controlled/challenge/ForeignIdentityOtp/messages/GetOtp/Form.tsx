import { ClassNameProp } from "@root/ui/props/ClassNameProp";
import { useAccount, useAccountFields } from "@root/hooks";
import LabeledInput from "@root/ui/components/LabeledInput";
import { useState } from "react";
import EditableLabeledInputListForm from "../../../common/EditableLabeledInputListForm";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faEnvelope, faHashtag, faQuestion } from "@fortawesome/free-solid-svg-icons";
import ErrorNotice from "@root/ui/components/ErrorNotice";
import type { BerytusForeignIdentityFieldOptions } from "@berytus/types-extd";

export interface FormProps extends ClassNameProp {
    accountId?: string;
    foreignIdentityFieldId: string;
    onSubmit(otp: string): void;
}

export default function Form({ accountId, foreignIdentityFieldId, className, onSubmit }: FormProps) {
    const { account, error: accountError } = useAccount(accountId);
    const { fields: field, error: fieldsError } = useAccountFields(account, foreignIdentityFieldId);
    const [submissionError, setSubmissionError] = useState<Error>();

    const loading = ! account || ! field;
    const errors = [accountError, fieldsError];

    const foreignIdentityField = field as undefined | { options: BerytusForeignIdentityFieldOptions; value: string } ;

    return (
        <EditableLabeledInputListForm
            loading={loading}
            errors={errors}
            items={[
                {
                    id: "otp",
                    defaultValue: "",
                    required: true,
                    type: "text",
                    label: "OTP"
                }
            ]}
            header={
                <>
                    {submissionError ? <ErrorNotice error={submissionError} /> : null}
                    {foreignIdentityField
                    ?   <>
                            <p className="text-sm italic mb-2 ml-1">
                                One-time password sent to:
                            </p>
                            <LabeledInput
                                label={
                                    <FontAwesomeIcon
                                        icon={
                                            foreignIdentityField.options.kind === "EmailAddress"
                                            ? faEnvelope
                                            : foreignIdentityField.options.kind === "PhoneNumber"
                                            ? faHashtag
                                            : faQuestion
                                        }
                                    />
                                }
                                type="text"
                                defaultValue={foreignIdentityField.value}
                                disabled
                                inputClassName="!cursor-not-allowed"
                                className="mb-2"
                            />
                        </>
                    : null}
                </>
            }
            onApprove={({ otp }) => {
                if (otp === undefined || otp.length === 0) {
                    setSubmissionError(new Error(
                        'OTP is unset. Kindly enter an OTP code.'
                    ));
                    return;
                }
                onSubmit(otp);
            }}
        />
    );
}

// export default function Form({ accountId, foreignIdentityFieldId, className, onSubmit }: FormProps) {
//     const { account, error: accountError } = useAccount(accountId);
//     const { fields, error: fieldsError } = useAccountFields(account, [foreignIdentityFieldId]);
//     const [submissionError, setSubmissionError] = useState<Error>();
//     const [otp, setOtp] = useState<string>();
//     if (fieldsError || accountError || submissionError) {
//         return <Notice
//             className={className}
//             text={`Error: ${concatErrorMessages(
//                 fieldsError,
//                 accountError,
//                 submissionError
//             )}`}
//             type="error"
//         />;
//     }

//     const loading = ! account || ! fields;
//     if (loading) {
//         return <MdCenteredSpinner />;
//     }
//     const foreignIdentityField = fields[0];
//     return (
//         <div className={className}>
//             <p className="text-sm text-center">Please enter the OTP code you have received for {foreignIdentityField.value}</p>
//             <LabeledInput
//                 type="text"
//                 label="OTP"
//                 className="mb-5"
//                 onChange={(e) => {
//                     setOtp(e.target.value);
//                 }}
//             />
//             <ApproveOrReject
//                 approveText={
//                     <ChallengeButtonTextWithIcon
//                         text="Send"
//                         icon={faPaperPlane}
//                     />
//                 }
//                 order="rejectFirst"
//                 onApprove={() => {
//                     if (otp === undefined) {
//                         setSubmissionError(new Error(
//                             'OTP is unset. Kindly input an OTP code.'
//                         ));
//                         return;
//                     }
//                     onSubmit(otp);
//                 }}
//             />
//         </div>
//     )
// }