import { FieldInfo } from "@berytus/types";

type FieldOptions = FieldInfo['options'];

type KeysOfUnion<T> = T extends T ? keyof T: never;

export const fieldOptionsLabels: Record<
    KeysOfUnion<FieldOptions>, string
> = {
    category: "Field Category",
    private: "Private",
    alg: "Algorithm",
    allowedCharacters: "Allowed Characters",
    kind: "External Identity Kind",
    humanReadable: "Human-readable",
    maxLength: "Maximum Length",
    identityFieldId: "Identity Field ID",
    passwordRules: "Password Rules",
}