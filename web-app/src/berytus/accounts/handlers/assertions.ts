export function assertIsString(val: unknown): asserts val is string {
    if (typeof val === "string") {
        return;
    }
    throw new Error("Assertion failed." + JSON.stringify(val) + " is not a string.");
}

export function assert(cond: boolean): asserts cond {
    if (cond) {
        return;
    }
    throw new Error("Assertion failed; condition is false");
}