import { KeyAgreementParameters } from "@berytus/types";

export interface Channel {
    id: string;
    scmEd25519?: {
        private: ArrayBuffer; // raw pkcs8
        public: string; // base64 spki
    };
    scmX25519?: {
        private: ArrayBuffer; // raw pkcs8
        public: string; // base64 spki
    };
    webAppEd25519Pub?: string;
    keyAgreement?: {
        parameters: KeyAgreementParameters;
        signatures: {
            canonicalJson: string;
            webAppSignature: ArrayBuffer;
            scmSignature?: ArrayBuffer;
        };
    };
    e2eeActvie: boolean;
    e2eeKey?: ArrayBuffer;
}

interface E2EReadyChannel extends Channel {
    scmEd25519: NonNullable<Channel['scmEd25519']>;
    scmX25519: NonNullable<Channel['scmX25519']>;
    webAppEd25519Pub: NonNullable<Channel['webAppEd25519Pub']>;
    keyAgreement: NonNullable<Channel['keyAgreement']>;
}

export interface E2EChannel extends E2EReadyChannel {
    e2eeActvie: true;
    e2eeKey: ArrayBuffer;
}

export function isChannelE2EReady(channel: Channel): channel is E2EReadyChannel {
    return !!(channel.scmEd25519 &&
        channel.scmX25519 &&
        channel.webAppEd25519Pub &&
        channel.keyAgreement &&
        channel.keyAgreement.signatures.webAppSignature &&
        channel.keyAgreement.signatures.scmSignature);
}