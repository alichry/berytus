export class EntityError extends Error {
    public readonly entityType: string;
    public readonly key: string | number;
    public readonly keyName?: string;

    constructor(
        msg: string,
        entityType: string,
        key: string | number,
        keyName?: string
    ) {
        super(msg);
        this.entityType = entityType;
        this.key = key;
        this.keyName = keyName;
    }
}

EntityError.prototype.name = "EntityError";