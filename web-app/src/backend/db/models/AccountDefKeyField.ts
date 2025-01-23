import type { RowDataPacket } from "mysql2";

export interface PAccountDefKeyField extends RowDataPacket {
    FieldID: string;
}

export class AccountDefKeyField {
    fieldId: string;

    constructor(fieldId: string) {
        this.fieldId = fieldId;
    }

}