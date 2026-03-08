import { table, toPostgresBigInt, useConnection, type PoolConnection } from "@root/backend/db/pool.js";
import { releaseAssert } from "@root/backend/utils/assert.js";
import type { JSONValue } from "../types";
import { ConditionalCheckError } from "../errors/ConditionalCheckError.js";
import { ChannelRequest } from "./ChannelRequest.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { webcrypto } from "node:crypto";

export enum EChannelType {
    NonE2EE = "NonE2EE",
    E2EE = "E2EE",
}

type ScmActor = Pick<BerytusSecretManagerActor, 'ed25519Key'>;

type KeyAgreementParameters = {
    [key: string]: JSONValue;
    readonly session: Omit<BerytusKeyExchangeSession, 'fingerprint'> & {
        fingerprint: Omit<BerytusKeyExchangeSession['fingerprint'], 'salt' | 'value'> & {
	        salt: string; // base64 encoded
	        value: string; // base64 encoded
        };
    };
    readonly authentication: BerytusKeyExchangeAuthentication & { [key: string]: JSONValue };
    readonly exchange: BerytusKeyExchangeParams & { [key: string]: JSONValue };
    readonly derivation: Omit<BerytusKeyDerivationParams, 'salt' | 'info'> & {
	    salt: string; // base64 encoded
	    info: string; // base64 encoded
    };
    readonly generation: BerytusKeyGenParams & { [key: string]: JSONValue };
}

type KeyAgreementSignatures = {
    webApp: string; // base64 encoded
    scm: string | null; // base64 encoded, can be null as
                        // parameters are signed by the web app first.
}

type SessionKey = {
    [key: string]: JSONValue;
};

interface PGetChannel {
    channelid: string;
    channeltype: EChannelType;
    channelrequestid: BigInt;
    scmactor: ScmActor;
    keyagreementparameters: KeyAgreementParameters | null;
    keyagreementsignatures: KeyAgreementSignatures | null;
    sessionkey: SessionKey | null;
}

export class Channel {
    public readonly id: string;
    public readonly requestId: BigInt;
    public readonly type: EChannelType;
    public readonly scmActor: ScmActor;
    #keyAgreementParameters: KeyAgreementParameters | null;
    #keyAgreementSignatures: KeyAgreementSignatures | null;
    #sessionKey: SessionKey | null;

    protected constructor(
        id: string,
        requestId: BigInt,
        type: EChannelType,
        scmActor: ScmActor,
        keyAgreementParameters: KeyAgreementParameters | null = null,
        keyAgreementSignatures: KeyAgreementSignatures | null = null,
        sessionKey: SessionKey | null = null
    ) {
        this.id = id;
        this.requestId = requestId;
        this.type = type;
        this.scmActor = Object.freeze({ ...scmActor});
        this.#keyAgreementParameters = keyAgreementParameters;
        this.#keyAgreementSignatures = keyAgreementSignatures;
        this.#sessionKey = sessionKey;
    }

    get keyAgreementParameters() {
        return this.#keyAgreementSignatures;
    }

    get keyAgreementSignatures() {
        return this.#keyAgreementSignatures;
    }

    get sessionKey() {
        return this.#sessionKey;
    }

    public e2eeEstablished(): this is Channel & {
        keyAgreementParameters: KeyAgreementParameters;
        keyAgreementSignatures: KeyAgreementSignatures;
        sessionKey: SessionKey;
    } {
        return this.#keyAgreementParameters !== null
            && this.#keyAgreementSignatures !== null
            && this.#keyAgreementSignatures.scm !== null
            && this.#sessionKey !== null;
    }

    public static async getChannel(
        channelId: string,
        existingConn?: PoolConnection
    ): Promise<Channel> {
        if (existingConn) {
            return this.#getChannel(
                existingConn,
                channelId,
            );
        }
        return useConnection(conn => this.#getChannel(
            conn,
            channelId
        ));
    }

    static async #getChannel(
        conn: PoolConnection,
        channelId: string
    ): Promise<Channel> {
        const res = await conn<PGetChannel[]>`
            SELECT ChannelID, ChannelType, ChannelRequestID,
                   ScmActor, KeyAgreementParameters, SessionKey
            FROM ${table('berytus_channel')}
            WHERE ChannelID = ${channelId}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                Channel.name,
                channelId,
                "ChannelID"
            );
        }
        return new Channel(
            res[0].channelid,
            res[0].channelrequestid,
            res[0].channeltype,
            res[0].scmactor,
            res[0].keyagreementparameters,
            res[0].keyagreementsignatures,
            res[0].sessionkey
        );
    }

    public static async create(
        channelId: string,
        channelRequestId: BigInt,
        scmActor: BerytusSecretManagerActor,
        existingConn?: PoolConnection
    ): Promise<Channel> {
        if (existingConn) {
            return this.#create(
                existingConn,
                channelId,
                channelRequestId,
                scmActor
            );
        }
        return useConnection(conn => this.#create(
            conn,
            channelId,
            channelRequestId,
            scmActor
        ));
    }

    static async #create(
        conn: PoolConnection,
        channelId: string,
        channelRequestId: BigInt,
        scmActor: ScmActor
    ): Promise<Channel> {
        const request = await ChannelRequest.getRequest(channelRequestId, conn);
        const type = request.supportsE2EE()
            ? EChannelType.E2EE
            : EChannelType.NonE2EE;
        const res = await conn`
            WITH cte_request AS (
                SELECT * FROM ${table('berytus_channel_request')}
                WHERE RequestID = ${toPostgresBigInt(channelRequestId)}
                AND WebAppActor = ${conn.json(request.webAppActor)}
                AND WebAppX25519 IS NOT DISTINCT FROM ${conn.json(request.webAppX25519)}
                AND UnmaskAllowlist IS NOT DISTINCT FROM ${conn.json(request.unmaskAllowlist)}
                FOR UPDATE
            ), cte_other_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelRequestID = ${toPostgresBigInt(channelRequestId)}
                FOR UPDATE
            )
            INSERT INTO ${table('berytus_channel')}
            (ChannelID, ChannelType, ChannelRequestID, ScmActor)
            SELECT  ${channelId},
                    ${type},
                    ${toPostgresBigInt(channelRequestId)},
                    ${conn.json(scmActor)}
            WHERE (SELECT TRUE FROM cte_request)
            AND NOT EXISTS (SELECT TRUE FROM cte_other_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                `${channelId},${channelRequestId}`,
                "ChannelID,ChannelRequestID",
                "Failed to create channel. Either the channel request has changed, " +
                "or a channel for the given channel request already exists " +
                "(and thus the request is no longer valid for channel creation)."
            );
        }
        return new Channel(
            channelId,
            channelRequestId,
            type,
            scmActor
        );
    }

    public async setKeyAgreementParameters(
        params: KeyAgreementParameters,
        existingConn?: PoolConnection
    ): Promise<void> {
        if (existingConn) {
            return this.#setKeyAgreementParameters(existingConn, params);
        }
        return useConnection(
            conn => this.#setKeyAgreementParameters(conn, params)
        );
    }

    async #setKeyAgreementParameters(
        conn: PoolConnection,
        params: KeyAgreementParameters,
    ): Promise<void> {
        const request = await ChannelRequest.getRequest(this.requestId, conn);
        if (! request.supportsE2EE()) {
            throw ConditionalCheckError.default(
                ChannelRequest.name,
                String(this.requestId),
                'RequestID',
                "ChannelRequest is not tailored for E2EE, but " +
                "is being used to establish E2EE."
            );
        }
        const checks = [
            ['channelId', params.session.id, this.id],
            ['unmaskAllowlist', params.session.unmaskAllowlist || null, request.unmaskAllowlist],
            ['webAppEd25519', params.authentication.public.webApp, request.webAppActor.ed25519Key],
            ['scmEd25519', params.authentication.public.scm, this.scmActor.ed25519Key],
            ['webAppX25519', params.exchange.public.webApp, request.webAppX25519.public],
            ['keyExchAuthAlg', params.authentication.name, 'Ed25519'],
            ['keyExchAlg', params.exchange.name, 'X25519'],
            ['keyDerivAlg', params.derivation.name, 'HKDF'],
            ['keyDerivHash', params.derivation.hash, 'SHA-256'],
            ['keyGenAlg', params.generation.name, 'AES-GCM'],
            ['keyGenKeyLength', params.generation.keyLength, 256],
        ];
        for (const [paramName, expected, actual] of checks) {
            if (actual !== expected) {
                throw ConditionalCheckError.default(
                    Channel.name,
                    this.id,
                    'ChannelID',
                    `${paramName} parameter verification failed. `
                    + `Expected '${expected}', got '${actual}'`
                );
            }
        }
        const res = await conn`
            WITH cte_request AS (
                SELECT * FROM ${table('berytus_channel_request')}
                WHERE RequestID = ${toPostgresBigInt(this.requestId)}
                AND WebAppActor = ${conn.json(request.webAppActor)}
                AND WebAppX25519 = ${conn.json(request.webAppX25519)}
                AND UnmaskAllowlist = ${conn.json(request.unmaskAllowlist)}
                FOR UPDATE
            ), cte_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelID = ${this.id}
                AND ChannelType = ${EChannelType.E2EE}
                AND ChannelRequestID = ${toPostgresBigInt(this.requestId)}
                AND ScmActor = ${conn.json(this.scmActor)}
                AND KeyAgreementParameters is NULL
                AND keyAgreementSignatures is NULL
                AND SessionKey is NULL
                FOR UPDATE
            )
            UPDATE ${table('berytus_channel')}
            SET KeyAgreementParameters = ${conn.json(params)}
            WHERE ChannelID = ${this.id}
            AND (SELECT TRUE FROM cte_request)
            AND (SELECT TRUE FROM cte_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                JSON.stringify({
                    ChannelID: this.id,
                    ChannelType: EChannelType.E2EE,
                    ChannelRequestID: this.requestId,
                    ScmActor: this.scmActor
                }),
                "*",
                "Failed to set key agreement parameters. Either the channel " +
                "does not exist, or the channel is not (or no longer due " +
                "to a concurrent update) in a valid state for setting " +
                "key agreement parameters."
            )
        }
        this.#keyAgreementParameters = params;
    }

    async setWebAppKapSignature(
        sigBase64: string,
        existingConn?: PoolConnection
    ): Promise<void> {
        if (existingConn) {
            return this.#setWebAppKapSignature(existingConn, sigBase64);
        }
        return useConnection(
            conn => this.#setWebAppKapSignature(conn, sigBase64)
        );
    }

    async #setWebAppKapSignature(
        conn: PoolConnection,
        sigBase64: string,
    ): Promise<void> {
        const signatures: KeyAgreementSignatures = {
            webApp: sigBase64,
            scm: null
        };
        if (this.keyAgreementParameters === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Key agreement parameters have not been set for the channel."
            );
        }
        if (this.keyAgreementSignatures !== null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Key agreement signatures have already been set for the channel."
            );
        }
        const res = await conn`
            WITH cte_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelID = ${this.id}
                AND ChannelType = ${EChannelType.E2EE}
                AND ChannelRequestID = ${toPostgresBigInt(this.requestId)}
                AND ScmActor = ${conn.json(this.scmActor)}
                AND KeyAgreementParameters = ${conn.json(this.keyAgreementParameters)}
                AND keyAgreementSignatures is NULL
                AND SessionKey is NULL
                FOR UPDATE
            )
            UPDATE ${table('berytus_channel')}
            SET keyAgreementSignatures = ${conn.json(signatures)}
            WHERE ChannelID = ${this.id}
            AND (SELECT TRUE FROM cte_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                "ChannelID",
                "Failed to set web app key agreement signature. Either the channel " +
                "does not exist (anymore), or the channel is not (or no longer due " +
                "to a concurrent update) in a valid state for setting the web app " +
                "key agreement signature."
            );
        }
        this.#keyAgreementSignatures = signatures;
    }

    async setScmKapSignature(
        sigBase64: string,
        existingConn?: PoolConnection
    ): Promise<void> {
        if (existingConn) {
            return this.#setScmKapSignature(existingConn, sigBase64);
        }
        return useConnection(
            conn => this.#setScmKapSignature(conn, sigBase64)
        );
    }

    async #setScmKapSignature(
        conn: PoolConnection,
        sigBase64: string
    ): Promise<void> {
        // KAP must be set
        // KAS must be not null
        // KAS->>scm must be null
        if (this.keyAgreementParameters === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Key agreement parameters have not been set for the channel."
            );
        }
        if (this.keyAgreementSignatures === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Web app key agreement signature has not been"
                + "set for the channel."
            );
        }
        if (this.keyAgreementSignatures.scm !== null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "secret manager key agreement signature has already been "
                + "set for the channel."
            );
        }
        const newSignatures: KeyAgreementSignatures = {
            webApp: this.keyAgreementSignatures.webApp,
            scm: sigBase64
        };
        const res = await conn`
            WITH cte_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelID = ${this.id}
                AND ChannelType = ${EChannelType.E2EE}
                AND ChannelRequestID = ${toPostgresBigInt(this.requestId)}
                AND ScmActor = ${conn.json(this.scmActor)}
                AND KeyAgreementParameters = ${conn.json(this.keyAgreementParameters)}
                AND keyAgreementSignatures is NOT NULL
                AND keyAgreementSignatures->>'webApp' = ${conn.json(this.keyAgreementSignatures.webApp)}
                AND keyAgreementSignatures->>'scm' IS NULL
                AND SessionKey is NULL
                FOR UPDATE
            )
            UPDATE ${table('berytus_channel')}
            SET keyAgreementSignatures = ${conn.json(newSignatures)}
            WHERE ChannelID = ${this.id}
            AND (SELECT TRUE FROM cte_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                "ChannelID",
                "Failed to set scm key agreement signature. Either the channel " +
                "does not exist (anymore), or the channel is not (or no longer due " +
                "to a concurrent update) in a valid state for setting the scm " +
                "key agreement signature."
            );
        }
        this.#keyAgreementSignatures = newSignatures;
    }

    async setSessionKey(
        sessionKey: SessionKey,
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return this.#setSessionKey(existingConn, sessionKey);
        }
        return useConnection(
            conn => this.#setSessionKey(conn, sessionKey)
        );
    }

    async #setSessionKey(
        conn: PoolConnection,
        sessionKey: SessionKey
    ): Promise<void> {
        if (this.keyAgreementParameters === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Key agreement parameters have not been set for the channel."
            );
        }
        if (this.keyAgreementSignatures === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Web app key agreement signature has not been "
                + "set for the channel."
            );
        }
        if (this.keyAgreementSignatures.scm === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "secret manager key agreement signature has not been "
                + "set for the channel."
            );
        }
        const res = await conn`
            WITH cte_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelID = ${this.id}
                AND ChannelType = ${EChannelType.E2EE}
                AND ChannelRequestID = ${toPostgresBigInt(this.requestId)}
                AND ScmActor = ${conn.json(this.scmActor)}
                AND KeyAgreementParameters = ${conn.json(this.keyAgreementParameters)}
                AND keyAgreementSignatures = ${conn.json(this.keyAgreementSignatures)}
                AND SessionKey is NULL
                FOR UPDATE
            )
            UPDATE ${table('berytus_channel')}
            SET SessionKey = ${conn.json(sessionKey)}
            WHERE ChannelID = ${this.id}
            AND (SELECT TRUE FROM cte_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                "ChannelID",
                "Failed to set session key. Either the channel " +
                "does not exist (anymore), or the channel is not (or no longer due " +
                "to a concurrent update) in a valid state for setting the session " +
                "key."
            );
        }
        this.#sessionKey = sessionKey;
    }

    // TODO(berytus): Move this to a handler
    async #signKeyAgreementParameters(
        conn: PoolConnection
    ): Promise<ArrayBuffer> {
        if (this.keyAgreementSignatures !== null) {
            return Uint8Array
                // @ts-ignore: Node 25+
                .fromBase64(this.keyAgreementSignatures.webApp)
                .buffer;
        }
        if (this.keyAgreementParameters === null) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                'ChannelID',
                "Key agreement parameters have not been set for the channel."
            );
        }
        // sign the key agreement parameters in the same
        // key order provided. Stringify with no space padding.
        const paramsStr = JSON.stringify(this.keyAgreementParameters);
        const paramsBuf = new TextEncoder().encode(paramsStr);
        const request = await ChannelRequest.getRequest(this.requestId, conn);
        releaseAssert(
            request.supportsE2EE(),
            "ChannelRequest must support E2EE for signing key agreement parameters"
        );
        const webAppEd25519B64 = request.webAppActor.ed25519Key;
        const webAppEd25519Buf: Uint8Array = Uint8Array
            // @ts-ignore: Node 25+
            .fromBase64(webAppEd25519B64);
        const cryptoKey = await webcrypto.subtle.importKey(
            "pkcs8",
            webAppEd25519Buf,
            "Ed25519",
            false,
            ["sign"]
        );
        const signature = await webcrypto.subtle.sign(
            "Ed25519",
            cryptoKey,
            paramsBuf
        );
        return signature;
        //const res = await conn`
    }
}