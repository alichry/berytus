import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import { object, string } from "yup";
import { BaseMessageProps } from "../../../common/types";
import Form from './Form';
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import ErrorNotice from "@root/ui/components/ErrorNotice";
import { useYupValidation } from "@root/hooks";

export interface MessageProps extends BaseMessageProps {
    onSubmit(otp: string): void;
}

const parametersSchema = object({
    field: string().required()
});

export default function Message({
    session,
    challenge,
    message,
    onSubmit
}: MessageProps) {
    const {
        loading: validationLoading,
        error: validationError,
        value: parameters
    } = useYupValidation(parametersSchema, challenge.parameters);

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError ? (
                <ErrorNotice error={validationError} />
            ) : validationLoading ? (
                <MdCenteredSpinner />
            ) : <Form
                    accountId={session.selectedAccountId}
                    foreignIdentityFieldId={parameters.field}
                    onSubmit={(v) => onSubmit(v)}
                />
            }
        </RespondToMessageView>
    )
}