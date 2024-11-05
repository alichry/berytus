[SecureContext, Exposed=(Window)]
interface BerytusKeyFieldValue : BerytusFieldValueDictionary {
    [Throws]
    readonly attribute (ArrayBuffer or BerytusEncryptedPacket) publicKey;
};

[SecureContext, Exposed=(Window)]
interface BerytusKeyField : BerytusField {
    /* -- readonly attribute BerytusKeyFieldOptions options; */
    /* -- readonly attribute BerytusKeyFieldValue value; */

    [Throws]
    constructor(
        DOMString id,
        BerytusKeyFieldOptions options
    );
};