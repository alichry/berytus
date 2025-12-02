export class UserError extends Error {

    constructor(msg: string, options?: ErrorOptions) {
        super(msg, options);
    }
}

UserError.prototype.name = "UserError";