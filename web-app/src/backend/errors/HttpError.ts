export class HttpError extends Error {
    status: number;

    constructor(
        status: number,
        msg?: string
    ) {
        super(msg);
        this.status = status;
    }

    isClientError() {
        return this.status >= 400 && this.status < 500;
    }

    isServerError() {
        return this.status >= 500;
    }
}


HttpError.prototype.name = "HttpError";