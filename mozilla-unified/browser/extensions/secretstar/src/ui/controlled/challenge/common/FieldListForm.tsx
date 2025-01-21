import { ClassNameProp } from "@root/ui/props/ClassNameProp";
import { useAccount, useAccountFields, useSettings, useTryOnce } from "@root/hooks";
import EditableLabeledInputListForm, { EditableLabeledInputListFormProps } from "./EditableLabeledInputListForm";
import { getFieldInputType } from "@root/ui/components/FieldInput";
import { useState } from "react";
import { Field } from "@root/db";

export interface FieldListFormProps
    extends ClassNameProp, Omit<EditableLabeledInputListFormProps,
        'loading' | 'errors' | 'items' | 'onApprove'
    > {
    defaultAccountId?: string;
    fieldIds: Array<string>;
    seamless: boolean;
    onSubmit(desiredFieldValues: Record<string, string>): void;
}

export const mapFieldsToEditableLabeledInputList = (fields: Array<Field>) => {
    return fields.map(f => ({
        type: getFieldInputType(f),
        id: f.id,
        label: `#${f.id}`,
        defaultValue: f.value
    }));
}

export default function FieldListForm({ defaultAccountId, fieldIds, onSubmit, seamless, ...otherProps }: FieldListFormProps) {
    const [accountId, setAccountId] = useState<string | undefined>(defaultAccountId);
    const { account, error: accountError } = useAccount(accountId);
    const { fields, error: fieldsError } = useAccountFields(account, fieldIds);
    const loading = ! account || ! fields;
    const errors = [accountError, fieldsError];
    const { tried, error: seamlessError } = useTryOnce(seamless, async () => {
        if (loading) {
            return false;
        }
        const values: Record<string, string> = {};
        for (let i = 0; i < fields.length; i++) {
            values[fields[i].id] = fields[i].value;
        }
        onSubmit(values);
        return true;
    }, [loading]);

    return (
        <EditableLabeledInputListForm
            {...otherProps}
            loading={loading || ! tried}
            errors={errors}
            items={fields ? mapFieldsToEditableLabeledInputList(fields) : []}
            onApprove={(desiredValues) => {
                onSubmit(desiredValues);
            }}
        />
    );
}