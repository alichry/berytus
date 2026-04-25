import { SRP, SrpServer as LibSrpServer } from 'fast-srp-hap';
import { debugAssert, releaseAssert } from '../utils/assert.js';
import { randomBytes as nodeRandomBytes } from "node:crypto";

// TODO(berytus): Avoid repetition/boilerplate code
// when reconstructing server

/**
 * Given a NodeJS Buffer, it's .buffer (ArrayBuffer) member
 * may hold additional bytes that are not part of the buffer's
 * content. This method safely converts a NodeJS Buffer
 * into an ArrayBuffer, stricly including the buffer's content
 * instead of including all of its allocated memory.
 */
const nodeBufferToArrayBuffer = (buf: Buffer): ArrayBuffer => {
    const res = buf.buffer.slice(
        buf.byteOffset,
        buf.byteOffset + buf.byteLength
    );
    if (!(res instanceof ArrayBuffer)) {
        throw new Error("Expected non-shared array buffer to be returned");
    }
    return res;
}

interface BaseMaterial {
    parameterSet: 4096;
    // if identity is provided, extended mode is assumed,
    // otherwise srp6a
    identity?: ArrayBuffer;
    salt: ArrayBuffer;
    verifier: ArrayBuffer;
    secret: ArrayBuffer;
    valueA?: ArrayBuffer;
}

interface BaseMaterialWithValueA extends BaseMaterial {
    valueA: ArrayBuffer;
}

interface ComputeBInput extends Omit<BaseMaterial, 'salt' | 'identity'> {}

interface ComputeBResult {
    valueB: ArrayBuffer;
}

interface ComputeExpectedM1Input extends BaseMaterialWithValueA {}

interface ComputeExpectedM1Result {
    expectedValueM1: ArrayBuffer;
}

interface CheckM1Input extends BaseMaterialWithValueA {
    valueM1: ArrayBuffer;
}

interface ComputeKInput extends BaseMaterialWithValueA {}

interface ComputeKResult {
    valueK: ArrayBuffer;
}

interface ComputeM2Input extends BaseMaterialWithValueA {}

interface ComputeM2Result {
    valueM2: ArrayBuffer;
}


const computeB = async ({
    parameterSet, verifier, secret
}: ComputeBInput): Promise<ComputeBResult> => {
    const params = SRP.params[parameterSet];
    // verifier is needed to compute B. identity/salt is never used
    // in srp6a nor in the extended mode when computing B.
    const server = new LibSrpServer(
        params, Buffer.from(verifier), Buffer.from(secret)
    );
    const valueB = server.computeB();
    return {
        valueB: nodeBufferToArrayBuffer(valueB)
    };
}

const checkM1 = async ({
    parameterSet, salt, identity,
    verifier, secret, valueM1, valueA
}: CheckM1Input): Promise<void> => {
    const params = SRP.params[parameterSet];
    let server;
    if (identity) {
        server = new LibSrpServer(
            params, {
                username: Buffer.from(identity),
                salt: Buffer.from(salt),
                verifier: Buffer.from(verifier)
            }, Buffer.from(secret)
        );
    } else {
        server = new LibSrpServer(
            params, Buffer.from(verifier), Buffer.from(secret)
        );
    }
    server.setA(Buffer.from(valueA));
    server.checkM1(Buffer.from(valueM1));
}

const computeExpectedM1 = async ({
    parameterSet, salt, identity,
    verifier, secret, valueA
}: ComputeExpectedM1Input): Promise<ComputeExpectedM1Result> => {
    const params = SRP.params[parameterSet];
    let server;
    if (identity) {
        server = new LibSrpServer(
            params, {
                salt: Buffer.from(salt),
                username: Buffer.from(identity),
                verifier: Buffer.from(verifier),
            }, Buffer.from(secret)
        );
    } else {
        server = new LibSrpServer(
            params, Buffer.from(verifier), Buffer.from(secret)
        );
    }
    server.setA(Buffer.from(valueA));
    const expectedValueM1 = server._M1;
    releaseAssert(expectedValueM1, "expectedValueM1");
    return {
        expectedValueM1: nodeBufferToArrayBuffer(expectedValueM1)
    }
}

const computeK = async ({
    parameterSet, salt, identity,
    verifier, secret, valueA
}: ComputeKInput): Promise<ComputeKResult> => {
    const params = SRP.params[parameterSet];
    let server;
    if (identity) {
        server = new LibSrpServer(
            params, {
                salt: Buffer.from(salt),
                username: Buffer.from(identity),
                verifier: Buffer.from(verifier)
            }, Buffer.from(secret)
        );
    } else {
        server = new LibSrpServer(
            params, Buffer.from(verifier), Buffer.from(secret)
        );
    }
    server.setA(Buffer.from(valueA));
    const valueK = server.computeK();
    return {
        valueK: nodeBufferToArrayBuffer(valueK)
    };
}

const computeM2 = async ({
    parameterSet, salt, identity,
    verifier, secret, valueA
}: ComputeM2Input): Promise<ComputeM2Result> => {
    const params = SRP.params[parameterSet];
    let server;
    if (identity) {
        server = new LibSrpServer(
            params, {
                salt: Buffer.from(salt),
                username: Buffer.from(identity),
                verifier: Buffer.from(verifier)
            }, Buffer.from(secret)
        );
    } else {
        server = new LibSrpServer(
            params, Buffer.from(verifier), Buffer.from(secret)
        );
    }
    server.setA(Buffer.from(valueA));
    const valueM2 = server.computeM2();
    return {
        valueM2: nodeBufferToArrayBuffer(valueM2)
    }
}

export interface ISrpStore {
    set(sessionId: string, material: BaseMaterial): Promise<void>;
    get(sessionId: string): Promise<BaseMaterial | undefined>;
}

export class InMemorySrpStore {
    static #instance: InMemorySrpStore | undefined;
    #data: Map<string, BaseMaterial> = new Map<string, BaseMaterial>();

    public static get instance() {
        if (!InMemorySrpStore.#instance) {
            InMemorySrpStore.#instance = new InMemorySrpStore();
        }
        return InMemorySrpStore.#instance;
    }

    protected constructor() {}

    public async set(sessionId: string, material: BaseMaterial): Promise<void> {
        this.#data.set(sessionId, material);
    }

    public async get(sessionId: string): Promise<BaseMaterial | undefined> {
        return this.#data.get(sessionId);
    }
}

export class InvalidClientProofError extends Error {}
InvalidClientProofError.prototype.name = "InvalidClientProofError";

export class SessionNotFoundError extends Error {}

export class SrpServer {
    #state: BaseMaterial;
    #store: ISrpStore;
    public readonly sessionId: string;

    protected constructor(
        store: ISrpStore,
        sessionId: string,
        parameterSet: 4096,
        identity: ArrayBuffer | undefined,
        salt: ArrayBuffer,
        verifier: ArrayBuffer,
        secret: ArrayBuffer,
        valueA: ArrayBuffer | undefined
    ) {
        this.#store = store;
        this.sessionId = sessionId;
        this.#state = {
            parameterSet,
            identity,
            salt,
            verifier,
            secret,
            valueA
        };
    }

    public async save() {
        this.#store.set(this.sessionId, this.#state);
    }

    public static async create(
        sessionId: string,
        parameterSet: 4096,
        identity: string,
        salt: ArrayBuffer,
        verifier: ArrayBuffer,
        randomBytes: typeof nodeRandomBytes = nodeRandomBytes,
        store: ISrpStore = InMemorySrpStore.instance
    ): Promise<SrpServer> {
        const secret = randomBytes(32);
        return new SrpServer(
            store,
            sessionId,
            parameterSet,
            new TextEncoder().encode(identity).buffer,
            salt,
            verifier,
            nodeBufferToArrayBuffer(secret),
            undefined
        );
    }

    public static async load(
        sessionId: string,
        store: ISrpStore = InMemorySrpStore.instance
    ) {
        const material = await store.get(sessionId);
        if (! material) {
            throw new SessionNotFoundError(`SRP session ${sessionId} not found`);
        }
        return new SrpServer(
            store,
            sessionId,
            material.parameterSet,
            material.identity,
            material.salt,
            material.verifier,
            material.secret,
            material.valueA
        );
    }

    public async computeB(): Promise<ArrayBuffer> {
        const { valueB } = await computeB({
            parameterSet: this.#state.parameterSet,
            verifier: this.#state.verifier,
            secret: this.#state.secret
        });
        return valueB;
    }

    public async setA(valueA: ArrayBuffer): Promise<void> {
        this.#state.valueA = valueA;
    }

    public getA(): ArrayBuffer | undefined {
        return this.#state.valueA;
    }

    public getSalt(): ArrayBuffer {
        return this.#state.salt;
    }

    public getIdentity(): ArrayBuffer | undefined {
        return this.#state.identity;
    }

    public async checkM1(valueM1: ArrayBuffer): Promise<void> {
        if (! this.#state.valueA) {
            throw new Error("valueA is not set.");
        }
        try {
            await checkM1({
                parameterSet: this.#state.parameterSet,
                secret: this.#state.secret,
                identity: this.#state.identity,
                salt: this.#state.salt,
                verifier: this.#state.verifier,
                valueA: this.#state.valueA,
                valueM1: valueM1
            });
        } catch (e) {
            throw new InvalidClientProofError(
                `Bad SRP:M1`
            );
        }
    }

    public async computeExpectedM1(): Promise<ArrayBuffer> {
        if (! this.#state.valueA) {
            throw new Error("valueA is not set.");
        }
        const { expectedValueM1 } = await computeExpectedM1({
            parameterSet: this.#state.parameterSet,
            secret: this.#state.secret,
            identity: this.#state.identity,
            salt: this.#state.salt,
            verifier: this.#state.verifier,
            valueA: this.#state.valueA
        });
        return expectedValueM1;
    }

    public async computeM2(): Promise<ArrayBuffer> {
        if (! this.#state.valueA) {
            throw new Error("valueA is not set.");
        }
        const { valueM2 } = await computeM2({
            parameterSet: this.#state.parameterSet,
            secret: this.#state.secret,
            identity: this.#state.identity,
            salt: this.#state.salt,
            verifier: this.#state.verifier,
            valueA: this.#state.valueA
        });
        return valueM2;
    }

    public async computeK(): Promise<ArrayBuffer> {
        if (! this.#state.valueA) {
            throw new Error("valueA is not set.");
        }
        const { valueK } = await computeK({
            parameterSet: this.#state.parameterSet,
            secret: this.#state.secret,
            identity: this.#state.identity,
            salt: this.#state.salt,
            verifier: this.#state.verifier,
            valueA: this.#state.valueA
        });
        return valueK;
    }
}