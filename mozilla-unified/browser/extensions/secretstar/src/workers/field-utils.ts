import { ab2base64 } from "@root/key-utils";
import type { ArrayBufferOrBerytusEncryptedPacket, BerytusEncryptedPacket, BerytusFieldUnion } from "@berytus/types-extd";

export const stringifyEncryptedPacket = (value: BerytusEncryptedPacket) => {
    const parameters: Record<string, string | number | boolean | undefined> = {};
    (Object.keys(value.parameters) as Array<keyof typeof value.parameters>).forEach(key => {
        const prop = value.parameters[key];
        if (prop === undefined || typeof prop === "string" || typeof prop === "number" || typeof prop === "boolean") {
            parameters[key] = prop;
            return;
        }
        if (prop instanceof ArrayBuffer) {
            parameters[key] = ab2base64(prop);
            return;
        }
        // TODO(berytus): Remove this when ArrayBufferViews are removed
        parameters[key] = ab2base64(prop.buffer);
    })
    return JSON.stringify({
        ciphertext: ab2base64(value.ciphertext),
        parameters
    })
}

export const stringifyArrayBufferOrEncryptedPacket = (value: ArrayBufferOrBerytusEncryptedPacket) => {
    if (value instanceof ArrayBuffer) {
        return ab2base64(value);
    }
    return stringifyEncryptedPacket(value);
}

export const stringifyFieldValue = (fieldValue: BerytusFieldUnion['value']) =>  {
    if (typeof fieldValue === "string") {
        return fieldValue;
    }
    if (fieldValue === null) {
        return "";
    }
    if ("salt" in fieldValue) {
        return JSON.stringify({
            salt: stringifyArrayBufferOrEncryptedPacket(fieldValue.salt),
            verifier: stringifyArrayBufferOrEncryptedPacket(fieldValue.verifier)
        })
    }
    if ("privateKey" in fieldValue) {
        return stringifyArrayBufferOrEncryptedPacket(fieldValue.privateKey);
    }
    if ("publicKey" in fieldValue) {
        return stringifyArrayBufferOrEncryptedPacket(fieldValue.publicKey);
    }
    return stringifyArrayBufferOrEncryptedPacket(fieldValue);
}