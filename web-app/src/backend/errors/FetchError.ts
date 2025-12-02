import { HttpError } from "./HttpError";

export class FetchError extends HttpError {
    contentType: string | null;
    body: Promise<object | string>;

    constructor(
        resp: Response,
        msg?: string,
        options?: ErrorOptions
    ) {
        super(resp.status, msg, options);
        this.contentType = resp.headers.get('content-type');
        this.body = resp.text()
            .then((text) => {
                try {
                    const data = JSON.stringify(text);
                    return data;
                } catch (_) {
                    return text;
                }
            })
            .catch(() => "");
    }

    async getServerErrorMessage(): Promise<string | undefined> {
        const data = await this.body;
        if (
            typeof data !== "object" ||
            data === null ||
            !("error" in data) ||
            typeof data.error !== "string"
        ) {
            return;
        }
        return data.error;
    }
}


FetchError.prototype.name = "FetchError";