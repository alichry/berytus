import { mapFieldsToEditableLabeledInputList } from "../../../common/FieldListForm";
import { useAccount, useAccountFields, useTryOnce } from "@root/hooks";
import EditableLabeledInputListForm from "../../../common/EditableLabeledInputListForm";
import type { BerytusSecurePasswordFieldOptions } from "@berytus/types-extd";

export interface FormProps {
    accountId: string;
    securePasswordFieldId: string;
    seamless: boolean;
    onSubmit(fields: { username: string; password: string; }): void;
}

export default function Form({ accountId, securePasswordFieldId, seamless, onSubmit }: FormProps) {
    const { account, error: accountError } = useAccount(accountId);
    const { fields: securePasswordField, error: error1 } = useAccountFields(account, securePasswordFieldId);
    const { fields: identityField, error: error2 } = useAccountFields(account,
        securePasswordField
        ? (securePasswordField.options as BerytusSecurePasswordFieldOptions).identityFieldId
        : undefined
    );
    const { tried, error: seamlessError } = useTryOnce(seamless, async () => {
        if (! identityField || ! securePasswordField) {
            return false;
        }
        onSubmit({
            username: identityField.value,
            password: securePasswordField.value
        });
        return true;
    }, [identityField, securePasswordField]);
    const loading = ! account || ! securePasswordField || ! identityField || ! tried;
    const errors = [accountError, error1, error2, seamlessError];

    return (
        <EditableLabeledInputListForm
            loading={loading}
            errors={errors}
            items={! loading
                ? mapFieldsToEditableLabeledInputList(
                    [identityField, securePasswordField]
                )
                : []
            }
            onApprove={(desiredValues) => {
                onSubmit({
                    username: desiredValues[identityField!.id],
                    password: desiredValues[securePasswordField!.id]
                });
            }}
        />
    );
    // return <FieldListForm {...props} />
}