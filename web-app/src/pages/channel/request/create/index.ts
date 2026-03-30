import type { APIRoute } from 'astro';
import { Body, Result } from './schema.js';
import { ChannelRequest } from '@root/backend/db/models/ChannelRequest.js';
import {
    StaticSigningKeyStore
} from '@root/backend/logic/e2ee-orchestration/StaticSigningKeyStore.js';
import {
    X25519KeyGenerator
} from '@root/backend/logic/e2ee-orchestration/X25519KeyGenerator.js';

const allowedOrigins = [
    "localhost",
    "127.0.0.1",
    "poc.berytus.io",
    "poc-e2ee.berytus.io"
];

export const POST: APIRoute = async ({ request }) => {
    const { type } =  Body.parse(await request.json());
    const originHeader = request.headers.get("origin");
    const origin = originHeader ? new URL('/', originHeader).hostname : null;
    if (null === origin || ! allowedOrigins.includes(origin)) {
        return new Response(
            JSON.stringify({
                error: `Invalid origin (${origin})`
            }),
            {
                status: 400,
                headers: {
                    "Content-Type": "application/json"
                }
            }
        );
    }
    const ed25519KeyStore = new StaticSigningKeyStore();
    const x25519Generator = new X25519KeyGenerator();
    let webAppActor, webAppX25519;
    if (type === "E2EE") {
        webAppActor = {
            ed25519Key: await ed25519KeyStore.getPublicKeyMaterial()
        };
        const x25519KeyPair = await x25519Generator.generateKeyPair();
        webAppX25519 = {
            public: x25519KeyPair.publicKey,
            private: x25519KeyPair.privateKey
        };
    } else {
        webAppActor = { origin };
        webAppX25519 = null;
    }
    const unmaskAllowlist: string[] | null =
        webAppX25519
            ? [] // TODO(berytus): <--
            : null;
    const channelRequest = await ChannelRequest.create(
        webAppActor,
        webAppX25519,
        unmaskAllowlist
    );
    const result: Result = {
        webAppActor,
        webAppX25519: webAppX25519 ? webAppX25519.public : null,
        unmaskAllowlist,
        channelRequestId: String(channelRequest.id)
    };
    return new Response(JSON.stringify(result), {
        headers: {
            "Content-Type": "application/json"
        }
    });
};