import type { UserAttributeKey } from "@berytus/types";
import { UserAttribute } from "./db";

// Not all users have a website, a profile page, or
// ... a picture.
const optionalKeys = ["profile", "picture", "website"] as const;
export function isUserAttributeKeyOptional(key: string): boolean {
    return optionalKeys.indexOf(
        key as OptionalUserAttributeKey
    ) !== -1;
}
type OptionalUserAttributeKey = typeof optionalKeys[number];

export type UserAttributes = Record<
    Exclude<UserAttributeKey, OptionalUserAttributeKey>,
    UserAttribute
> & Partial<Record<OptionalUserAttributeKey, UserAttribute>>;

export interface Identity {
    id: string;
    userAttributes?: UserAttributes;
    emailAddress?: string;
    mobileNumber?: string;
}

export const PRIMARY_IDENTITY_ID = "primary";