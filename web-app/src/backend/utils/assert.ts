import { strict as assert } from "node:assert";
import { isDev } from "../env/app.js";
import { InternalError } from "../errors/InternalError.js";

export type Assert = typeof assert;

export const debugAssert = (cb: (assert: Assert) => void) => {
    if (! isDev()) {
        return;
    }
    cb(assert);
}

export const releaseAssert = (cb: (assert: Assert) => void) => {
    try {
        const ret = cb(assert);
        debugAssert(assert => assert(!(typeof ret === "object" && "then" in ret)));
    } catch (e) {
        return new InternalError(
            `Release assertion failed for `
            + typeof cb === "function" ? cb.toString().slice(0, 64).replace("\n", "\\n") : "??",
            { cause: e }
        );
    }
}