export interface Channel {
    id: string;
    webAppEd25519Key?: string;
    scmEd25519Key?: {
        publicKey: string; // base64 spki
        privateKey: string; // base64 pkcs8
    };
    scmX25519Key?: {
        publicKey: string; // base64 spki
        privateKey: string; // base64 pkcs8
    };
    webAppX25519Key?: string;
    params?: {
        hkdfHash: string; //hash name
        hkdfSalt: string; // base64
        hkdfInfo: string; // base64
        aesKeyLength: number; // bits
    };
    webAppSignature?: string; // base64
    scmSignature?: string; // base64
    sharedKey?: ArrayBuffer
}

interface E2EChannel extends Channel {
    webAppEd25519Key: NonNullable<Channel['webAppEd25519Key']>;
    scmEd25519Key: NonNullable<Channel['scmEd25519Key']>;
    scmX25519Key: NonNullable<Channel['scmX25519Key']>;
    webAppX25519Key: NonNullable<Channel['webAppX25519Key']>;
    params: NonNullable<Channel['params']>;
    webAppSignature?: NonNullable<Channel['webAppSignature']>;
    scmSignature?: NonNullable<Channel['scmSignature']>;
    sharedKey?: NonNullable<Channel['sharedKey']>;
}

export function isChannelE2EReady(channel: Channel): channel is E2EChannel {
    return !!(channel.params &&
        channel.scmEd25519Key &&
        channel.scmX25519Key &&
        channel.webAppEd25519Key &&
        channel.webAppX25519Key &&
        channel.webAppSignature &&
        channel.scmSignature &&
        channel.sharedKey);
}