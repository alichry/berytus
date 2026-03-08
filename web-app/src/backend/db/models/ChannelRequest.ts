import { table, toPostgresBigInt, useConnection, type PoolConnection } from "@root/backend/db/pool.js";
import { releaseAssert } from "@root/backend/utils/assert.js";
import { EntityNotFoundError } from "../errors/EntityNotFoundError.js";
import { InvalidArgumentError } from "../errors/InvalidArgumentError.js";

type WebAppActor = Pick<BerytusCryptoWebAppActor, 'ed25519Key'>
    | { origin: string; }

type WebAppX25519 = {
    public: string;
    private: string;
}

interface PCreateChannelRequest {
    requestid: BigInt;
}

interface PGetChannelRequest {
    requestid: BigInt;
    webappactor: WebAppActor;
    webappx25519: WebAppX25519;
    unmaskallowlist: ReadonlyArray<string>;
}

export class ChannelRequest {
    public readonly id: BigInt;
    public readonly webAppActor: WebAppActor;
    public readonly webAppX25519: WebAppX25519 | null;
    public readonly unmaskAllowlist: ReadonlyArray<string> | null;

    constructor(
        id: BigInt,
        webAppActor: WebAppActor,
        webAppX25519: WebAppX25519 | null,
        unmaskAllowlist: ReadonlyArray<string> | null,
    ) {
        this.id = id;
        this.webAppActor = webAppActor;
        this.webAppX25519 = webAppX25519;
        this.unmaskAllowlist = unmaskAllowlist;
    }

    public supportsE2EE(): this is ChannelRequest & {
        webAppActor: Pick<BerytusCryptoWebAppActor, 'ed25519Key'>
        webAppX25519: WebAppX25519;
    } {
        return "ed25519Key" in this.webAppActor
            && null !== this.webAppX25519;
    }

    public static async create(
        webAppActor: WebAppActor,
        webAppX25519: WebAppX25519 | null,
        unmaskAllowlist: ReadonlyArray<string> | null,
        existingConn?: PoolConnection
    ): Promise<ChannelRequest> {
        if (existingConn) {
            return this.#create(
                existingConn,
                webAppActor,
                webAppX25519,
                unmaskAllowlist,
            );
        }
        return useConnection(conn => this.#create(
            conn,
            webAppActor,
            webAppX25519,
            unmaskAllowlist
        ));
    }

    static async #create(
        conn: PoolConnection,
        webAppActor: WebAppActor,
        webAppX25519: WebAppX25519 | null,
        unmaskAllowlist: ReadonlyArray<string> | null
    ): Promise<ChannelRequest> {
        if ("origin" in webAppActor) {
            if (webAppX25519 !== null) {
                throw new InvalidArgumentError(
                    "webAppX25519 must be null when webAppActor "
                    + "is an origin-based origin."
                );
            }
            if (unmaskAllowlist !== null) {
                throw new InvalidArgumentError(
                    "unmaskAllowlist must be null when webAppActor "
                    + "is an origin-based actor."
                );
            }
        } else {
            if (webAppX25519 === null) {
                throw new InvalidArgumentError(
                    "webAppX25519 must be provided when webAppActor is "
                    + "a crypto actor."
                );
            }
            if (unmaskAllowlist === null) {
                throw new InvalidArgumentError(
                    "unmaskAllowlist must be provided when webAppActor is "
                    + "a crypto actor."
                );
            }
        }
        const res = await conn<PCreateChannelRequest[]>`
            INSERT INTO ${table('berytus_channel_request')}
            (WebAppActor, WebAppX25519, UnmaskAllowlist)
            VALUES (${conn.json(webAppActor)}, ${conn.json(webAppX25519)}, ${conn.json(unmaskAllowlist)})
            RETURNING RequestID
        `;
        releaseAssert(
            res.count === 1,
            "Expected exactly one row to be inserted for ChannelRequest, "
            + " got " + res.length + " row(s) instead."
        );
        return new ChannelRequest(
            res[0].requestid,
            webAppActor,
            webAppX25519,
            unmaskAllowlist
        );
    }

    public static async getRequest(
        id: BigInt,
        existingConn?: PoolConnection
    ): Promise<ChannelRequest> {
        if (existingConn) {
            return ChannelRequest.#getRequest(existingConn, id);
        }
        return useConnection(
            conn => ChannelRequest.#getRequest(conn, id)
        );
    }

    static async #getRequest(conn: PoolConnection, id: BigInt): Promise<ChannelRequest> {
        const res = await conn<PGetChannelRequest[]>`
            SELECT RequestID, WebAppActor, WebAppX25519, UnmaskAllowlist
            FROM ${table('berytus_channel_request')}
            WHERE RequestID = ${toPostgresBigInt(id)}
        `;
        if (res.length === 0) {
            throw EntityNotFoundError.default(
                ChannelRequest.name,
                String(id),
                "RequestID",
                "No ChannelRequest found for the given RequestID"
            );
        }
        return new ChannelRequest(
            res[0].requestid,
            res[0].webappactor,
            res[0].webappx25519,
            res[0].unmaskallowlist
        );
    }
}