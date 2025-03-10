import { AccountDefField, EFieldType } from "@root/backend/db/models/AccountDefField";
import type { FieldInput } from "@root/backend/db/types";
import { PasswordHandler } from "./PasswordHandler";
import { DigitalSignatureHandler } from "./DigitalSignatureHandler";

export const transformField = async (
    accountVersion: number,
    fieldInput: FieldInput
) => {
    const fieldDef = await AccountDefField.getField(
        accountVersion,
        fieldInput.id
    );
    switch (fieldDef.fieldType) {
        case EFieldType.Password:
            return new PasswordHandler().transform(fieldInput);
        case EFieldType.Key:
            return new DigitalSignatureHandler().transform(fieldInput);
        default:
            return fieldInput;
    }
}