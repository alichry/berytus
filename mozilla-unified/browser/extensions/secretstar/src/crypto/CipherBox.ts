type EncryptFunction<CT> = (datum: string | ArrayBufferLike, prop?: string) => Promise<CT>;
type DecryptFunction<CT, DT> = (datum: CT, prop?: string) => Promise<DT>;

export interface CipherBoxOptions<CipherType, DecipherType> {
    ignoreProp?: (prop: string) => boolean;
    ignoreValue?: (value: unknown) => boolean;
    encrypt: EncryptFunction<CipherType>;
    decrypt: DecryptFunction<CipherType, DecipherType>;
}

export type EncryptedDictionary<T extends object, CT> =
    CT extends T
    ? T
    : { [K in keyof T]:
        string extends T[K]
        ? CT
        : number extends T[K]
        ? CT
        : boolean extends T[K]
        ? CT
        : ArrayBufferLike extends T[K]
        ? CT
        : ArrayBufferView extends T[K]
        ? CT
        : T[K] extends object
        ? EncryptedDictionary<T[K], CT>
        : T[K]
    };

export type DecryptedDictionary<T extends object, CT, DT> = {
    [K in keyof T]:
        CT extends infer CTI
        ? CTI extends T[K]
        ? DT
        : T[K]
        : never
};

export abstract class AbstractCipherBox<CipherType, DecipherType> {
    #options: CipherBoxOptions<CipherType, DecipherType>;
    constructor(opts: CipherBoxOptions<CipherType, DecipherType>) {
        this.#options = opts;
    }

    get options() {
        return this.#options;
    }

    abstract isCiphertextType(ciph: unknown): ciph is CipherType;

    async #encryptDictionary<O extends object>(
        input: O,
        output: Record<string, any>,
        path: string = '$dict'
    ): Promise<void> {
        if (input === null) {
            throw new Error("Passed dictionary is null.");
        }
        if (typeof input !== "object") {
            throw new Error("Passed dictionary is not an object.");
        }
        if (input instanceof ArrayBuffer || ArrayBuffer.isView(input)) {
            throw new Error('ArrayBuffer(View) must not be passed to encryptDictionary(). Use encrypt() instead.');
        }
        const promises: Promise<void>[] = [];
        for (const key in input) {
            if (this.#options.ignoreProp && this.#options.ignoreProp(key)) {
                continue;
            }
            if (this.#options.ignoreValue && this.#options.ignoreValue(input[key])) {
                continue;
            }
            if (this.isCiphertextType(input[key])) {
                throw new Error(
                    "Encountered an already encrypted field. "
                    + "encryptDictionary() assuems all fields "
                    + "are plaintext."
                );
            }
            if (
                typeof input[key] === 'object' &&
                input[key] !== null &&
                ! (input[key] instanceof ArrayBuffer) &&
                ! ArrayBuffer.isView(input[key])
            ) {
                if (Array.isArray(input[key])) {
                    output[key] = [];
                } else {
                    output[key] = {};
                }
                promises.push(
                    this.#encryptDictionary(
                        input[key] as object,
                        output[key],
                        `${path}.${key}`
                    )
                );
                continue;
            }
            promises.push(
                this.encrypt(input[key], key)
                    .then(encrypted => {
                        if (encrypted === null) {
                            output[key] = input[key];
                            return;
                        }
                        output[key] = encrypted;
                    })
            );
        }
        return Promise.all(promises)
            .then(_ => Promise.resolve());
    }

    async #decryptDictionary<O extends object>(
        input: O,
        output: Record<string, any>,
        path: string = '$dict'
    ): Promise<void> {
        if (input === null) {
            throw new Error("Passed dictionary is null.");
        }
        if (typeof input !== "object") {
            throw new Error("Passed dictionary is not an object.");
        }
        if (input instanceof ArrayBuffer || ArrayBuffer.isView(input)) {
            throw new Error('ArrayBuffer(View) must not be passed to decryptDictionary(). Use decrypt() instead.');
        }
        const promises: Promise<void>[] = []
        for (const key in input) {
            if (this.#options.ignoreProp && this.#options.ignoreProp(key)) {
                continue;
            }
            if (
                typeof input[key] === 'object' &&
                input[key] !== null &&
                ! (input[key] instanceof ArrayBuffer) &&
                ! ArrayBuffer.isView(input[key])
            ) {
                if (Array.isArray(input[key])) {
                    output[key] = [];
                } else {
                    output[key] = {};
                }
                promises.push(
                    this.#decryptDictionary(
                        input[key] as object,
                        output[key],
                        `${path}.${key}`
                    )
                );
                continue;
            }
            promises.push(
                this.decrypt(input[key] as any, key)
                    .then(decrypted => {
                        if (decrypted === null) {
                            output[key] = input[key];
                            return;
                        }
                        output[key] = decrypted;
                    })
            );
        }
        return Promise.all(promises)
            .then(_ => Promise.resolve())
    }

    public async encryptDictionary<O extends object>(
        obj: O
    ): Promise<EncryptedDictionary<O, CipherType>> {
        const output = {};
        await this.#encryptDictionary(
            obj,
            output
        );
        return output as EncryptedDictionary<O, CipherType>;
    }

    public async decryptDictionary<O extends object>(
        obj: O
    ): Promise<DecryptedDictionary<O, CipherType, DecipherType>> {
        const output = {};
        await this.#decryptDictionary(
            obj,
            output
        );
        return output as DecryptedDictionary<O, CipherType, DecipherType>;
    }

    public async decrypt(
        datum: CipherType,
        path?: string
    ): Promise<null | DecipherType> {
        if (this.#options.ignoreValue && this.#options.ignoreValue(datum)) {
            return null;
        }
        if (! this.isCiphertextType(datum)) {
            throw new Error(`decrypt() cannot decrypt '${typeof datum}' type in ${path}.`);
        }
        if (
            typeof datum === "string" ||
            datum instanceof ArrayBuffer ||
            ArrayBuffer.isView(datum)
        ) {
            return this.#options.decrypt(datum, path);
        }
        throw new Error(`Bad isCiphertextType() implementation.`);
    }

    async encrypt(
        datum: unknown,
        path?: string
    ): Promise<null | CipherType> {
        if (this.#options.ignoreValue && this.#options.ignoreValue(datum)) {
            return null;
        }
        if (datum === null || datum === undefined) {
            console.warn("refusing to encrypt null/undefined values");
            return null;
        }
        switch (typeof datum) {
            case 'string':
                return this.#options.encrypt(datum, path);
            case 'number':
                return this.#options.encrypt(datum.toString(10), path);
            case 'boolean':
                return this.#options.encrypt(datum ? '1' : '0', path);
            case 'object':
                if (datum instanceof ArrayBuffer) {
                    return this.#options.encrypt(datum, path);
                }
                if (ArrayBuffer.isView(datum)) {
                    return this.#options.encrypt(datum.buffer, path);
                }
                break;
        }
        throw new Error(`encrypt() cannot encrypt '${typeof datum}' type in ${path}.`);
    }
}

