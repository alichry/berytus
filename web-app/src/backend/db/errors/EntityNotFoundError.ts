import { EntityError } from "./EntityError.js";

export class EntityNotFoundError extends EntityError {
    static default(
        type: string,
        index: string | number,
        indexName?: string,
        taskErrMsg?: string
    ): EntityNotFoundError {
        return new EntityNotFoundError(
            (taskErrMsg ? taskErrMsg + ' | ' : '') +
            'Entity ' +
            type +
            (indexName !== undefined ? ':' + indexName : '')
            + ':' + index +
            ' was not found! ',
            type,
            index,
            indexName
        );
    }

    /**
     * Clone the passed EntityNotFoundError object but set a
     * new task error message.
     */
    static from(e: EntityNotFoundError, newTaskErrMsg: string) {
        return EntityNotFoundError.default(
            e.entityType,
            e.key,
            e.keyName,
            newTaskErrMsg
        );
    }
}

EntityNotFoundError.prototype.name = "EntityNotFoundError";