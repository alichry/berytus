import{ generate } from "do_username";
import { EBerytusFieldType } from "@berytus/enums";

export const randomPassword = () => {
    const bytes = new Uint8Array(16);
    window.crypto.getRandomValues(bytes);
    let blocks: Array<string> = [];
    const lang = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9']
    for (let i = 0; i < bytes.length; i++) {
        const num = bytes[i];
        blocks[i % 4] = (blocks[i % 4] || "") +
            lang[num % lang.length];
    }
    return blocks.join('-');
}

export const randomUsername = () => {
    const suffix = randomPassword().substring(0, 4);
    return generate(8) + suffix;
}

export type BgColor = `bg-${string}`;


export async function randomFieldValue(fieldType: typeof EBerytusFieldType[keyof typeof EBerytusFieldType]): Promise<string | ArrayBuffer> {
    switch (fieldType) {
        case EBerytusFieldType.Identity:
            return randomUsername();
        case EBerytusFieldType.Password:
        case EBerytusFieldType.SecurePassword:
            return randomPassword();
        case EBerytusFieldType.Key:
            const genKey = await crypto.subtle.generateKey(
                {
                    name: "RSA-OAEP",
                    modulusLength: 4096,
                    publicExponent: new Uint8Array([1, 0, 1]),
                    hash: "SHA-256",
                },
                true,
                ["encrypt", "decrypt"]
            );
            const privKeyBuf = await window.crypto.subtle.exportKey("pkcs8", genKey.privateKey);
            return privKeyBuf;
        default:
            throw new Error('error: generateStr received an invalid fieldType ' + EBerytusFieldType[fieldType]);
    }
}

export const notEmpty = (str: string | undefined): boolean => {
    if (str === undefined) {
        return false;
    }
    return str.length > 0;
}

export const scrollTopSmooth = () => {
    window.scrollTo({
        top: 0,
        left: 0,
        behavior: 'smooth'
    });
};

export const concatErrorMessages = (...args: Array<Error | undefined>): string => {
    let res: Array<string> = [];
    for (let i = 0; i < args.length; i++) {
        const err = args[i];
        if (! err) {
            continue;
        }
        res.push(err.message);
    }
    return res.join(' && ');
}

export function atLeastOneErrorSet(errors?: Array<Error | undefined>): boolean {
    if (! errors) {
        return false;
    }
    for (let i = 0; i < errors.length; i++) {
        if (errors[i]) {
            return true;
        }
    }
    return false;
}