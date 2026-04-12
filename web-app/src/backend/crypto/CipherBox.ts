/**
 * Based on Secret*'s AbstractCipherBox, but with some modifications:
 * - path parameter is an array of strings instead of a dot-separated string,
 *   to avoid issues with dots in property names.
 * - isCiphertextType() method accepts the path parameter to allow for more
 *   context-aware checks.
 */

type EncryptFunction<CT> = (datum: string | ArrayBufferLike, propPath?: ReadonlyArray<string>) => Promise<CT>;
type DecryptFunction<CT, DT> = (datum: CT, propPath?: ReadonlyArray<string>) => Promise<DT>;
type TransformDecrytpedFunction<DT> = (value: DT, propPath?: ReadonlyArray<string>) => Promise<unknown>;

export interface CipherBoxOptions<CipherType, DecipherType> {
    ignoreProp?: (propPath: ReadonlyArray<string>) => boolean;
    ignoreValue?: (value: unknown) => boolean;
    encrypt: EncryptFunction<CipherType>;
    decrypt: DecryptFunction<CipherType, DecipherType>;
    /**
     * Only applies when calling decryptDictionary()
     */
    transformDecrypted?: TransformDecrytpedFunction<DecipherType>;
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

    abstract isCiphertextType(ciph: unknown, path?: ReadonlyArray<string>): ciph is CipherType;

    async #encryptDictionary<O extends object>(
        input: O,
        output: Record<string, any>,
        path: ReadonlyArray<string> = []
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
            if (this.#options.ignoreProp && this.#options.ignoreProp(path.concat(key))) {
                continue;
            }
            if (this.#options.ignoreValue && this.#options.ignoreValue(input[key])) {
                continue;
            }
            if (this.isCiphertextType(input[key], path.concat(key))) {
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
                        path.concat(key)
                    )
                );
                continue;
            }
            promises.push(
                this.encrypt(input[key], path.concat(key))
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
        path: ReadonlyArray<string> = []
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
            if (this.#options.ignoreProp && this.#options.ignoreProp(path.concat(key))) {
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
                        path.concat(key)
                    )
                );
                continue;
            }
            if (! this.isCiphertextType(input[key])) {
                output[key] = input[key];
                continue;
            }
            promises.push(
                this.decrypt(input[key] as any, path.concat(key))
                    .then(async decrypted => {
                        if (decrypted === null) {
                            output[key] = input[key];
                            return;
                        }
                        if (this.#options.transformDecrypted) {
                            output[key] = await this.#options.transformDecrypted(decrypted, path);
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
        path: ReadonlyArray<string> = []
    ): Promise<null | DecipherType> {
        if (this.#options.ignoreValue && this.#options.ignoreValue(datum)) {
            return null;
        }
        if (! this.isCiphertextType(datum, path)) {
            throw new Error(`decrypt() cannot decrypt '${typeof datum}' type in ${path}.`);
        }
        return this.#options.decrypt(
            datum,
            path.length === 0 ? undefined: path
        );
    }

    async encrypt(
        datum: unknown,
        path: ReadonlyArray<string> = []
    ): Promise<null | CipherType> {
        if (this.#options.ignoreValue && this.#options.ignoreValue(datum)) {
            return null;
        }
        if (datum === null || datum === undefined) {
            console.warn("refusing to encrypt null/undefined values");
            return null;
        }
        let input;
        switch (typeof datum) {
            case 'string':
                input = datum;
                break;
            case 'number':
                input = datum.toString(10);
                break;
            case 'boolean':
                input = datum ? '1' : '0';
                break;
            case 'object':
                if (datum instanceof ArrayBuffer) {
                    input = datum;
                    break;
                }
                if (ArrayBuffer.isView(datum)) {
                    input = datum.buffer;
                    break;
                }
            default:
                throw new Error(`encrypt() cannot encrypt '${typeof datum}' type in ${path}.`);
        }
        return this.#options.encrypt(
            input,
            path.length === 0 ? undefined : path
        );
    }
}

