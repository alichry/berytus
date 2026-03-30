import { table, toPostgresBigInt, useConnection, type PoolConnection } from "@root/backend/db/pool.js";
import type { JSONValue } from "../types";
import { ConditionalCheckError } from "../errors/ConditionalCheckError.js";
import { ChannelRequest } from "./ChannelRequest.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { isDeepStrictEqual } from 'node:util';
import { IllegalStateError } from "@root/backend/errors/IllegalStateError.js";

export enum EChannelType {
    NonE2EE = "NonE2EE",
    E2EE = "E2EE",
}

export enum EChannelStatus {
    Active = 'Active',
    Closed = 'Closed'
}

type ScmActor = Pick<BerytusSecretManagerActor, 'ed25519Key'>;

export type KeyAgreementParametersJson = {
    [key: string]: JSONValue;
    readonly session: Omit<BerytusKeyExchangeSession, 'fingerprint' | 'unmaskAllowlist'> & {
        fingerprint: Omit<BerytusKeyExchangeSession['fingerprint'], 'salt' | 'value'> & {
	        salt: string; // base64 encoded
	        value: string; // base64 encoded
        };
        unmaskAllowlist?: ReadonlyArray<string>;
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
    keyagreementparameters: KeyAgreementParametersJson | null;
    keyagreementsignatures: KeyAgreementSignatures | null;
    sessionkey: SessionKey | null;
    channelstatus: EChannelStatus;
}

export class Channel {
    public readonly id: string;
    public readonly requestId: BigInt;
    public readonly type: EChannelType;
    public readonly scmActor: ScmActor;
    #status: EChannelStatus;
    #keyAgreementParameters: KeyAgreementParametersJson | null;
    #keyAgreementSignatures: KeyAgreementSignatures | null;
    #sessionKey: SessionKey | null;


    protected constructor(
        id: string,
        requestId: BigInt,
        type: EChannelType,
        scmActor: ScmActor,
        status: EChannelStatus,
        keyAgreementParameters: KeyAgreementParametersJson | null = null,
        keyAgreementSignatures: KeyAgreementSignatures | null = null,
        sessionKey: SessionKey | null = null,
    ) {
        this.id = id;
        this.requestId = requestId;
        this.type = type;
        this.#status = status;
        this.scmActor = Object.freeze({ ...scmActor});
        this.#keyAgreementParameters = keyAgreementParameters;
        this.#keyAgreementSignatures = keyAgreementSignatures;
        this.#sessionKey = sessionKey;
    }

    get keyAgreementParameters() {
        return this.#keyAgreementParameters;
    }

    get keyAgreementSignatures() {
        return this.#keyAgreementSignatures;
    }

    get sessionKey() {
        return this.#sessionKey;
    }

    public e2eeEstablished(): this is Channel & {
        keyAgreementParameters: KeyAgreementParametersJson;
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
                   ScmActor, KeyAgreementParameters,
                   KeyAgreementSignatures, SessionKey,
                   ChannelStatus
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
            res[0].channelstatus,
            res[0].keyagreementparameters,
            res[0].keyagreementsignatures,
            res[0].sessionkey
        );
    }

    public static async create(
        channelId: string,
        channelRequestId: BigInt,
        scmActor: ScmActor,
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
            (ChannelID, ChannelType, ChannelRequestID, ScmActor, ChannelStatus)
            SELECT  ${channelId},
                    ${type},
                    ${toPostgresBigInt(channelRequestId)},
                    ${conn.json(scmActor)},
                    ${EChannelStatus.Active}
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
            scmActor,
            EChannelStatus.Active
        );
    }

    public async setKeyAgreementParameters(
        params: KeyAgreementParametersJson,
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
        params: KeyAgreementParametersJson,
    ): Promise<void> {
        if (this.#status !== EChannelStatus.Active) {
            throw new IllegalStateError("Channel is not in an activate state");
        }
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
            ['unmaskAllowlist', params.session.unmaskAllowlist, request.unmaskAllowlist],
            ['keyExchAuthAlg', params.authentication.name, 'Ed25519'],
            ['webAppEd25519', params.authentication.public.webApp, request.webAppActor.ed25519Key],
            ['scmEd25519', params.authentication.public.scm, this.scmActor.ed25519Key],
            ['webAppX25519', params.exchange.public.webApp, request.webAppX25519.public],
            ['keyExchAlg', params.exchange.name, 'X25519'],
            ['keyDerivAlg', params.derivation.name, 'HKDF'],
            ['keyDerivHash', params.derivation.hash, 'SHA-256'],
            ['keyGenAlg', params.generation.name, 'AES-GCM'],
            ['keyGenKeyLength', params.generation.length, 256],
        ];
        for (const [paramName, actual, expected] of checks) {
            if (! isDeepStrictEqual(actual, expected)) {
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
                AND ChannelStatus = ${this.#status}
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
            );
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
        if (this.#status !== EChannelStatus.Active) {
            throw new IllegalStateError("Channel is not in an activate state");
        }
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
                AND ChannelStatus = ${this.#status}
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
        if (this.#status !== EChannelStatus.Active) {
            throw new IllegalStateError("Channel is not in an activate state");
        }
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
                AND ChannelStatus = ${this.#status}
                AND KeyAgreementParameters = ${conn.json(this.keyAgreementParameters)}
                AND keyAgreementSignatures is NOT NULL
                AND keyAgreementSignatures->>'webApp' = ${this.keyAgreementSignatures.webApp}
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
        if (this.#status !== EChannelStatus.Active) {
            throw new IllegalStateError("Channel is not in an activate state");
        }
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
                AND ChannelStatus = ${this.#status}
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

    public async closeChannel(
        existingConn?: PoolConnection
    ) {
        if (existingConn) {
            return this.#closeChannel(existingConn);
        }
        return useConnection(
            conn => this.#closeChannel(conn)
        );
    }

    async #closeChannel(conn: PoolConnection) {
        if (this.#status !== EChannelStatus.Active) {
            throw new IllegalStateError("Channel is not in an activate state");
        }
        const res = await conn`
            WITH cte_channel AS (
                SELECT * FROM ${table('berytus_channel')}
                WHERE ChannelID = ${this.id}
                AND ChannelType = ${this.type}
                AND ChannelRequestID = ${toPostgresBigInt(this.requestId)}
                AND ScmActor = ${conn.json(this.scmActor)}
                AND ChannelStatus = ${this.#status}
                AND KeyAgreementParameters IS NOT DISTINCT FROM ${conn.json(this.keyAgreementParameters)}
                AND keyAgreementSignatures IS NOT DISTINCT FROM ${conn.json(this.keyAgreementSignatures)}
                AND SessionKey IS NOT DISTINCT FROM ${conn.json(this.sessionKey)}
                FOR UPDATE
            )
            UPDATE ${table('berytus_channel')}
            SET ChannelStatus = ${EChannelStatus.Closed}
            WHERE ChannelID = ${this.id}
            AND (SELECT TRUE FROM cte_channel)
        `;
        if (res.count === 0) {
            throw ConditionalCheckError.default(
                Channel.name,
                this.id,
                "ChannelID",
                "Failed to close channel. Either the channel " +
                "does not exist (anymore), or the channel is not (or no longer due " +
                "to a concurrent update) in its expected state"
            );
        }
    }

    public toJSON() {
        return {
            id: this.id,
            requestId: this.requestId,
            type: this.type,
            scmActor: this.scmActor,
            keyAgreementParameters: this.keyAgreementParameters,
            keyAgreementSignatures: this.keyAgreementSignatures,
            sessionKey: this.sessionKey
        }
    }
}