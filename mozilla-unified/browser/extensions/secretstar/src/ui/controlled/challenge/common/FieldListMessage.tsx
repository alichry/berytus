import RespondToMessageView from "@root/ui/components/RespondToMessageView";
import FieldListForm from "./FieldListForm";
import { array, object, string } from "yup";
import Notice from "@root/ui/components/Notice";
import Spinner from "@root/ui/components/Spinner";
import { BaseMessageProps } from "./types";
import { useYupValidation } from "@root/hooks";

export interface FieldListMessageProps extends BaseMessageProps {
    onSubmit(fieldValues: Record<string, string>): void;
}

const parametersSchema = object({
    fields: array()
        .of(string().required())
        .min(1)
        .required()
});

export default function FieldListMessage({
    session,
    challenge,
    message,
    settings,
    onSubmit
}: FieldListMessageProps) {
    const {
        loading: validationLoading,
        error: validationError,
        value: parameters
    } = useYupValidation(parametersSchema, challenge.parameters);

    return (
        <RespondToMessageView challengeType={challenge.type} onCancel={() => window.close()}>
            { validationError ? (
                <Notice type="error" text={`Error: ${validationError.message}`} />
            ) : validationLoading ? (
                <Spinner width="2rem" className="mx-auto" />
            ) : <FieldListForm
                    seamless={settings.seamless.login}
                    defaultAccountId={session.selectedAccountId}
                    fieldIds={parameters.fields}
                    onSubmit={(v) => onSubmit(v)}
                />
            }
        </RespondToMessageView>
    )
}