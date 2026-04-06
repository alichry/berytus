import type {
    BerytusEncryptedPacket,
    BerytusFieldValueUnion,
    BerytusIdentificationChallengeMessageName,
    BerytusPasswordChallengeMessageName,
    BerytusSecureRemotePasswordChallengeMessageName,
    BerytusDigitalSignatureChallengeMessageName,
    BerytusOffChannelOtpChallengeMessageName
} from "@berytus/types-extd";
import { Channel } from "@root/db/Channel";
import { db, Session } from "@root/db";
import JWEPacketCipherBox from "@root/crypto/JWEPacketBox";

const createJWEPacketCipherBox = async (channelOrId: Channel | string) => {
    let channel;
    if (typeof channelOrId === "string") {
        channel = await db.channel.get(channelOrId);
        if (! channel) {
            throw new Error('Unable to find channel by id ' + channelOrId);
        }
    } else {
        channel = channelOrId;
    }
    if (!channel.e2eeKey) {
        throw new Error('Cannot create JWECipherBox, channel\'s e2eeKey is unset.');
    }
    const box = new JWEPacketCipherBox({ key: channel.e2eeKey });
    return box;
}

const decryptPacket = async (
    channelOrId: Channel | string,
    packet: BerytusEncryptedPacket
): Promise<ArrayBufferLike> => {
    if (packet.type !== "JWE") {
        throw new Error('Unrecognised packet type.');
    }
    const box = await createJWEPacketCipherBox(channelOrId);
    const decr = await box.decrypt(packet);
    if (decr === null) {
        throw new Error('CipherBox refused to decrypt.');
    }
    return decr;
}

export const toClearFieldValue = async <FVU extends BerytusFieldValueUnion>(
    channelOrId: Channel | string,
    fieldValue: FVU
) => {
    if (typeof fieldValue === "string") {
        return fieldValue;
    }
    if (
        typeof fieldValue === "object" &&
        fieldValue !== null &&
        "type" in fieldValue &&
        "value" in fieldValue &&
        fieldValue.type === "JWE" &&
        typeof fieldValue.value === "string"
    ) {
        const decrypted = await decryptPacket(
            channelOrId,
            fieldValue
        );
        // TODO(berytus): This implementation needs documentation
        // We stringified the returned, deciphered, buffer as
        // all field who has a non-strucuted field values have string
        // field value types. A Better approach is to return the proper
        // decoding according to the field type.
        return new TextDecoder().decode(new Uint8Array(decrypted));
    }
    const box = await createJWEPacketCipherBox(channelOrId);
    // Note(berytus): All other fields that have structured values
    // have inner values as ArrayBuffers, not strings.
    const res = await box.decryptDictionary(fieldValue as Exclude<FVU, string | BerytusEncryptedPacket>);
    return res;
}

/**
 * Currently, all message request payloads are either:
 *  - a string (plaintext or base64), or
 *  - a list of strings (plaintext), or
 *  - an ArrayBuffer, or
 *  - a JWE packets, or
 *  - a list of JWE packets.
 * In other words, no plain dictionaries. This method turns
 * JWE packets to their cleartext equivalents; e.g. a stirng
 * or ArrayBuffer. DO NOT pass a string or ArrayBuffer as input.
 */
export const toClearMessageRequestPayload = async (
    sessionOrId: Session | string,
    channelOrId: Channel | string,
    challengeId: string,
    messageName: BerytusIdentificationChallengeMessageName
        | BerytusPasswordChallengeMessageName
        | BerytusSecureRemotePasswordChallengeMessageName
        | BerytusDigitalSignatureChallengeMessageName
        | BerytusOffChannelOtpChallengeMessageName,
    requestPayload: BerytusEncryptedPacket | BerytusEncryptedPacket[]
): Promise<string | ArrayBuffer | string[]> => {
    const box = await createJWEPacketCipherBox(channelOrId);
    let session;
    if (typeof sessionOrId === "string") {
        session = await db.sessions.get(sessionOrId);
        if (! session) {
            throw new Error('Unable to find session by id ' + sessionOrId);
        }
    } else {
        session = sessionOrId;
    }
    let challenge = session.challenges
        ? session.challenges[challengeId]
        : undefined;
    if (! challenge ) {
        throw new Error('toClearMessageRequestPayload(): Unable to find challenge by id ' + challengeId);
    }
    switch (messageName) {
        case "GetIdentityFields":
        case "GetPasswordFields": {
            if (! Array.isArray(requestPayload)) {
                throw new Error('toClearMessageRequestPayload(): Expected array payload for message ' + messageName);
            }
            const clearPayload: string[] = [];
            for (const packet of requestPayload) {
                const decrypted = await box.decrypt(packet);
                if (decrypted === null) {
                    throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
                }
                clearPayload.push(new TextDecoder().decode(new Uint8Array(decrypted)));
            }
            return clearPayload;
        }
        case "SelectKey": {
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            return new TextDecoder().decode(new Uint8Array(decrypted));
        }
        case "SignNonce":
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            return decrypted;
        case "SelectSecurePassword": {
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            return new TextDecoder().decode(new Uint8Array(decrypted));
        }
        case "ExchangePublicKeys": {
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            if (challenge.parameters == null) {
                throw new Error(
                    'toClearMessageRequestPayload(): SRP Challenge '
                    + 'parameters are missing.'
                );
            }
            if (challenge.parameters.encoding === "Hex") {
                // @ts-ignore: NOTE(berytus): toBase64() is
                // now widely available in modern browsers!
                return new Uint8Array(decrypted).toBase64();
            }
            return decrypted;
        }
        case "ComputeClientProof": {
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            if (challenge.parameters == null) {
                throw new Error(
                    'toClearMessageRequestPayload(): SRP Challenge '
                    + 'parameters are missing.'
                );
            }
            if (challenge.parameters.encoding === "Hex") {
                // @ts-ignore: NOTE(berytus): toBase64() is
                // now widely available in modern browsers!
                return new Uint8Array(decrypted).toBase64();
            }
            return decrypted;
        }
        case "VerifyServerProof": {
            if (! box.isCiphertextType(requestPayload)) {
                throw new Error("toClearMessageRequestPayload(): Expected JWE packet payload for message " + messageName);
            }
            const decrypted = await box.decrypt(requestPayload);
            if (decrypted === null) {
                throw new Error('toClearMessageRequestPayload(): CipherBox refused to decrypt.');
            }
            if (challenge.parameters == null) {
                throw new Error(
                    'toClearMessageRequestPayload(): SRP Challenge '
                    + 'parameters are missing.'
                );
            }
            if (challenge.parameters.encoding === "Hex") {
                // @ts-ignore: NOTE(berytus): toBase64() is
                // now widely available in modern browsers!
                return new Uint8Array(decrypted).toBase64();
            }
            return decrypted;
        }
        default:
            throw new Error('toClearMessageRequestPayload(): Unhandled message name ' + messageName);
    }
}

/**
 * Returns true if the value is a non-empty array of JWE packets.
 */
export function isNonEmptyArrayOfPackets(value: unknown): value is Array<BerytusEncryptedPacket> {
    if (! Array.isArray(value)) {
        return false;
    }
    if (value.length === 0) {
        return false;
    }
    if (! JWEPacketCipherBox.isCiphertextType(value[0])) {
        return false;
    }
    value.forEach((item, i) => {
        if (JWEPacketCipherBox.isCiphertextType(item)) {
            return;
        }
        throw new Error(
            "isArrayOfPackets(): Not all items are JWE packets. Got "
            + `item at index ${i} of type ${typeof item} (${item})`
        );
    });
    return true;
}