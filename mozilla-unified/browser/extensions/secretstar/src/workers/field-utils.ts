import type { BerytusEncryptedPacket, BerytusFieldValueUnion } from "@berytus/types-extd";
import { Channel } from "@root/db/Channel";
import { db } from "@root/db";
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