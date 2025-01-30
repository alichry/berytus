[SecureContext, Exposed=(Window)]
interface BerytusSharedKeyFieldValue : BerytusFieldValueDictionary {
    [Throws]
    readonly attribute (ArrayBuffer or BerytusEncryptedPacket) privateKey;

    [Throws]
    constructor((BufferSource or BerytusEncryptedPacket) privateKey);
};

[SecureContext, Exposed=(Window)]
interface BerytusSharedKeyField : BerytusField {
    /* -- readonly attribute BerytusSharedKeyFieldOptions options; */

    [Throws]
    constructor(
        DOMString id,
        BerytusSharedKeyFieldOptions options,
        optional BerytusSharedKeyFieldValue desiredPrivateKeyValue
    );
};