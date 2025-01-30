import { forwardRef } from "react";
import { ClassNameProp } from "../props/ClassNameProp";
import { Field } from "@root/db";
import { getFieldInputType } from "./FieldInput";
import LabeledInput, { LabeledInputProps } from "./LabeledInput";

export interface ChallengeFieldProps
    extends ClassNameProp, Omit<LabeledInputProps, 'label' | 'type'> {
    field: Field;
}

const ChallengeField = forwardRef<
    HTMLInputElement & HTMLTextAreaElement,
    ChallengeFieldProps
>(function ChallengeField({ field, ...inputProps }, ref) {
    const type = getFieldInputType(field);
    return (
        <LabeledInput
            {...inputProps}
            label={`#${field.id}`}
            type={type}
        />
    )
});

export default ChallengeField;