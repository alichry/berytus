import { EntityError } from "./EntityError.js";

export class EntityAlreadyExistsError extends EntityError {
    public static default(
        type: string,
        key: string | number,
        keyName?: string,
        taskErrMsg?: string
    ): EntityAlreadyExistsError {
        return new EntityAlreadyExistsError(
            (taskErrMsg ? taskErrMsg + ' | ' : '') +
            'Entity ' +
            type +
            (keyName !== undefined ? ':' + keyName : '')
            + ':' + key +
            ' already exists! ',
            type,
            key,
            keyName
        );
    }
}

EntityAlreadyExistsError.prototype.name = "EntityAlreadyExistsError";