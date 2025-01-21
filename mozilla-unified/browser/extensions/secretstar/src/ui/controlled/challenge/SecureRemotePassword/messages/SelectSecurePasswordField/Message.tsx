import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import Form, { FormProps } from "./Form";
import { string } from "yup";
import Notice from "@root/ui/components/Notice";
import Spinner from "@root/ui/components/Spinner";
import { BaseMessageProps } from "../../../common/types";
import { useYupValidation } from "@root/hooks";
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";

export interface MessageProps extends BaseMessageProps {
    onSubmit: FormProps['onSubmit'];
}

const payloadSchema = string()
    .label("SecurePasswordFieldId")
    .required();

export default function Message({
    session,
    challenge,
    message,
    settings,
    onSubmit
}: MessageProps) {
    const {
        loading: validationLoading,
        error: validationError,
        value: securePasswordFieldId
    } = useYupValidation(payloadSchema, message.payload);

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError ? (
                <Notice type="error" text={`Error: ${validationError.message}`} />
            ) : validationLoading ? (
                <MdCenteredSpinner />
            ) : ! session.selectedAccountId ? (
                <Notice type="error" text={`Error: There is no selected account for this challenge!`} />
            ) : <Form
                    accountId={session.selectedAccountId}
                    securePasswordFieldId={securePasswordFieldId}
                    seamless={settings.seamless.login}
                    onSubmit={(v) => onSubmit(v)}
                />
            }
        </RespondToMessageView>
    )
}
