import type { IFieldHandler, UserFieldInput } from "./types";
import { scrypt as scryptCb, randomBytes } from "crypto";
import { promisify } from "util";
import { z } from "zod";
const scrypt = promisify(scryptCb);

export const StoredPassword = z.object({
    hash: z.string(),
    salt: z.string()
});

export type StoredPassword = z.infer<typeof StoredPassword>;

export class PasswordHandler implements IFieldHandler {

    static async hash(password: string): Promise<StoredPassword> {
        const salt = randomBytes(16);
        const key = await scrypt(
            password,
            new Uint8Array(salt),
            64
        ) as Buffer;
        return {
            salt: salt.toString('hex'),
            hash: key.toString('hex')
        }
    }

    static async compare(passedPassword: string, storedPassword: StoredPassword) {
        const salt = Buffer.from(storedPassword.salt, 'hex');
        const key = await scrypt(
            passedPassword,
            new Uint8Array(salt),
            64
        ) as Buffer;
        return key.toString('hex') === storedPassword.hash;
    }

    async transform(field: UserFieldInput) {
        const password = await z.string().parseAsync(field.value);
        const hashed = await PasswordHandler.hash(password);
        return {
            id: field.id,
            value: hashed
        };
    }
}