export type TargetContentType = "json" | "multipart" | "blob" | "text";

export const populateFormData = (fdata: FormData, value: unknown, key?: string) => {
    if (value instanceof ArrayBuffer) {
        fdata.append(key!, new Blob([value], { type: "application/octet-stream" }));
        return;
    }
    if (value instanceof Blob) {
        fdata.append(key!, value);
        return;
    }
    if (ArrayBuffer.isView(value) || value instanceof DataView) {
        throw new Error(
            "Expecting ArrayBuffer to be passed for binary data. "
            + "Typed arrays and DataViews are not supported."
        );
    }
    if (typeof value === "object" && value !== null) {
        for (const [k, v] of Object.entries(value)) {
            if (/(\[|\])/.test(String(k))) {
                throw new Error(
                    `Keys cannot contain square brackets when `
                    + `using multipart form data. Invalid key: ${k}`
                );
            }
            populateFormData(fdata, v, key ? `${key}.${k}` : k);
        }
        return;
    }
    // @ts-ignore: Browser implementation should
    // convert value to string if necessary.
    fdata.append(key!, value);
}

export const buildRequestBodyAndHeaders = (
    requestBody: unknown,
    targetContentType: TargetContentType = "multipart"
) => {
    let body, contentTypeHeader;
    if (targetContentType === "json") {
        body = JSON.stringify(requestBody);
        contentTypeHeader = "application/json";
    } else if (targetContentType === "multipart") {
        if (typeof requestBody !== "object" || requestBody === null) {
            throw new Error(
                "Expecting response to be a non-null object "
                + "when contentType is multipart."
            );
        }
        body = new FormData();
        contentTypeHeader = "multipart/form-data";
        populateFormData(body, requestBody);
    } else if (targetContentType === "text") {
        if (typeof requestBody !== "string") {
            throw new Error(
                "Expecting response to be a Blob when contentType is text."
            );
        }
        body = requestBody;
        contentTypeHeader = "text/plain";
    } else {
        if (! (requestBody instanceof Blob)) {
            throw new Error(
                "Expecting response to be a Blob when contentType is blob."
            );
        }
        body = requestBody;
        contentTypeHeader = requestBody.type;
    }
    const headers = {
        ...(contentTypeHeader !== "multipart/form-data" ? {
            // Note(berytus): Do not explicitly set content-type
            // header when sending multipart/form-data. It must
            // be set by the browser as it includes the boundary
            // part.
            ['Content-Type']: contentTypeHeader
        } : null)
    };
    return { body, headers };
}