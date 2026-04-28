import { AccountDefField, EFieldType } from "@root/backend/db/models/AccountDefField";
import type { UserFieldInput } from "./types";
import { PasswordHandler } from "./PasswordHandler";
import { DigitalSignatureHandler } from "./DigitalSignatureHandler";
import type { FieldInput as DbFieldInput } from "@root/backend/db/types";
import * as z from "zod";
import { SecurePasswordHandler } from "./SecurePasswordHandler";

const jsonValueLeafSchema = z.union([
    z.string(),
    z.number(),
    z.boolean(),
    z.null(),
]);
const jsonValueSchema = z.union([
    jsonValueLeafSchema,
    z.array(jsonValueLeafSchema),
    z.record(jsonValueLeafSchema)
]);

export const transformField = async (
    accountVersion: number,
    fieldInput: UserFieldInput
): Promise<DbFieldInput> => {
    const fieldDef = await AccountDefField.getField(
        accountVersion,
        fieldInput.id
    );
    switch (fieldDef.fieldType) {
        case EFieldType.Password:
            return new PasswordHandler().transform(fieldInput);
        case EFieldType.Key:
            return new DigitalSignatureHandler().transform(fieldInput);
        case EFieldType.SecurePassword:
            return new SecurePasswordHandler().transform(fieldInput);
        default:
            return await z.object({
                id: z.string(),
                value: jsonValueSchema
            }).parseAsync(fieldInput);
    }
}