import { readdir, readFile } from "fs/promises";
import { join } from "node:path";
import { createPrivateKey, createPublicKey } from "node:crypto";

export const unarmor = (str: string) => {
    return str.replace('-----BEGIN PRIVATE KEY-----', '')
        .replace('-----BEGIN PUBLIC KEY-----', '')
        .replace('-----END PRIVATE KEY-----', '')
        .replace('-----END PUBLIC KEY-----', '')
        .replaceAll(/(\n|\r|\s)/g, '');
}

export const parseKeys = async (keysDir = 'server/') => {
    const patt = /^berytus\.([0-9])+\.privkey\.pem$/;
    const files = await readdir(keysDir, { recursive: false });
    const keys: Array<string> = [];
    for (const file of files) {
        const match = file.match(patt);
        if (! match) {
            continue;
        }
        const [_, index] = match;
        const content = await readFile(
            join(keysDir, file), { encoding: 'ascii' }
        );
        const privateKey = createPrivateKey(content);
        const publicKey = createPublicKey(privateKey);

        keys[index] = {
            private: unarmor(
                privateKey.export({ type: "pkcs8", format: "pem" }).toString()
            ),
            public: unarmor(
                publicKey.export({ type: "spki", format: "pem" }).toString()
            )
        }
    }
    console.log(keys);
    return keys;
}