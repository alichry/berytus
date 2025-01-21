import { forwardRef } from "react";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import { faEnvelope, faHashtag, faEllipsis } from "@fortawesome/free-solid-svg-icons";
import SuperInput, { SuperInputProps } from "./SuperInput";
import { FieldInfo } from "@berytus/types";
import { EBerytusFieldType } from "@berytus/enums";

export interface FieldInputProps extends Omit<SuperInputProps, 'type'> {
    field: FieldInfo;
}
const FieldInput = forwardRef<
    HTMLInputElement,
    FieldInputProps
>(function FieldInput({ field, ...otherProps }: FieldInputProps, ref) {
    const type = getFieldInputType(field);
    return <SuperInput
        ref={ref}
        {...otherProps}
        type={type}
        inputIcon={type === "email" ? (
            <FontAwesomeIcon
                icon={faEnvelope}
            />
        ) : type === "password" ? (
            <FontAwesomeIcon
                icon={faEllipsis}
            />
        ) : field.type === EBerytusFieldType.Identity ? (
            <FontAwesomeIcon
                icon={faHashtag}
            />
        ) : null}

    />
})

export default FieldInput;

export function getFieldInputType(field: FieldInfo) {
    switch (field.type) {
        case EBerytusFieldType.Password:
        case EBerytusFieldType.SecurePassword:
            return "password";
        case EBerytusFieldType.ForeignIdentity:
            // @ts-ignore TODO(berytus): Generate BerytusIdentityFieldInfo,
            // BerytusForeignIdentityFieldInfo, etc. And 
            // BerytusFieldInfoUnion = A | B | ... | C;
            // FieldInfo should omit "value" property. This would
            // replace FieldInfo defined in types.ts
            if (field.options.kind === 'EmailAddress') {
                return "email";
            }
            return "text";
        default:
            return "text";
    }
}