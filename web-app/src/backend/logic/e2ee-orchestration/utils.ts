import { InvalidArgError } from "@root/backend/errors/InvalidArgError.js";

export const uniformReturn =
    <T>(publicKey?: T, privateKey?: T): { publicKey: T } | { privateKey: T } | { publicKey: T; privateKey: T } => {
    if (publicKey !== undefined && privateKey !== undefined) {
        return { publicKey, privateKey };
    }
    if (publicKey !== undefined && privateKey === undefined) {
        return { publicKey };
    }
    if (publicKey === undefined && privateKey !== undefined) {
        return { privateKey };
    }
    throw new InvalidArgError("Either publicKey or privateKey must be set");
}