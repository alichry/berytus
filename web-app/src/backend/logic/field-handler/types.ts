import type { FieldInput as DbFieldInput } from "@root/backend/db/types";
import type { JSONValue } from "@root/shared-types";

export type UserFieldValue = JSONValue
    | Blob
    | {
      readonly [prop: string | number]:
      | undefined
      | UserFieldValue
    };

export interface UserFieldInput {
    id: string;
    value: UserFieldValue;
};

export interface IFieldHandler {
    transform(field: UserFieldInput): Promise<DbFieldInput>;
}