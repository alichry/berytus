import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import { string } from "yup";
import { BaseMessageProps } from "../../../common/types";
import Form from './Form';
import MdCenteredSpinner from "@root/ui/components/MdCenteredSpinner";
import ErrorNotice from "@root/ui/components/ErrorNotice";
import { useYupValidation } from "@root/hooks";

export interface MessageProps extends BaseMessageProps {
    onSubmit(privateKeyBuf: ArrayBuffer, publicKeyBuf: ArrayBuffer): void;
}

const payloadSchema = string()
    .label("KeyFieldId")
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
        value: keyFieldId
    } = useYupValidation(payloadSchema, message.payload);

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError ? (
                <ErrorNotice error={validationError} />
            ) : validationLoading ? (
                <MdCenteredSpinner />
            ) : <Form
                    accountId={session.selectedAccountId}
                    keyFieldId={keyFieldId}
                    seamless={settings.seamless.login}
                    onSubmit={(privateKeyBuf, publicKeyBuf) => {
                        onSubmit(privateKeyBuf, publicKeyBuf);
                    }}
                />
            }
        </RespondToMessageView>
    )
}