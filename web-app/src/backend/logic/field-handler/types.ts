import type { FieldInput } from "@root/backend/db/types";

export interface IFieldHandler {
    transform(field: FieldInput): Promise<FieldInput>;
}