import { EntityError } from "./EntityError.js";

export class ConditionalCheckError extends EntityError {
    public static default(
        type: string,
        key: string | number,
        keyName?: string,
        taskErrMsg?: string
    ): ConditionalCheckError {
        return new ConditionalCheckError(
            (taskErrMsg ? taskErrMsg + ' | ' : '') +
            'Entity ' +
            type +
            (keyName !== undefined ? ':' + keyName : '')
            + ':' + key +
            ' failed conditional check! ',
            type,
            key,
            keyName
        );
    }
}

ConditionalCheckError.prototype.name = "ConditionalCheckError";