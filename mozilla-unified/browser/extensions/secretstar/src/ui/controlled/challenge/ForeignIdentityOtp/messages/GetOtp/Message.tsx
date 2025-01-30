import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import { string } from "yup";
import { BaseMessageProps } from "../../../common/types";
import Form from './Form';
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import ErrorNotice from "@root/ui/components/ErrorNotice";
import { useYupValidation } from "@root/hooks";

export interface MessageProps extends BaseMessageProps {
    onSubmit(otp: string): void;
}

const payloadSchema = string()
    .label("ForeignIdentityFieldId")
    .required();

export default function Message({
    session,
    challenge,
    message,
    onSubmit
}: MessageProps) {
    const {
        loading: validationLoading,
        error: validationError,
        value: foreignIdentityFieldId
    } = useYupValidation(payloadSchema, message.payload);

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError ? (
                <ErrorNotice error={validationError} />
            ) : validationLoading ? (
                <MdCenteredSpinner />
            ) : <Form
                    accountId={session.selectedAccountId}
                    foreignIdentityFieldId={foreignIdentityFieldId}
                    onSubmit={(v) => onSubmit(v)}
                />
            }
        </RespondToMessageView>
    )
}