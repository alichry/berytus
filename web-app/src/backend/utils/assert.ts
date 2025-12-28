import { strict as assert, AssertionError } from "node:assert";
import { isDev } from "../env/app.js";
import { InternalError } from "../errors/InternalError.js";

export type Assert = typeof assert;

export const debugAssert = (cb: (assert: Assert) => void) => {
    if (! isDev()) {
        return;
    }
    cb(assert);
}

export const releaseAssert: Assert = new Proxy(assert, {
    get(target, prop: keyof typeof assert, receiver) {
        const value = target[prop];
        if (typeof value !== "function") {
            return value;
        }
        return function (...args: any[]) {
            try {
                // @ts-ignore
                return value.apply(this === receiver ? target : this, args);
            } catch (e) {
                if (e instanceof AssertionError) {
                    throw new InternalError(
                        `Release assertion failed for `
                        + `assert.${prop}(${args.join(', ')})`,
                        { cause: e }
                    );
                }
                throw e;
            }

        };
    },
    apply(
        target,
        thisArg,
        argArray: [value: unknown, message?: string | Error | undefined]
    ) {
        try {
            return target.apply(thisArg, argArray);
        } catch (e) {
            if (e instanceof AssertionError) {
                throw new InternalError(
                    `Release assertion failed for `
                    + `assert(${argArray.join(', ')})`,
                    { cause: e }
                );
            }
            throw e;
        }
    },
});