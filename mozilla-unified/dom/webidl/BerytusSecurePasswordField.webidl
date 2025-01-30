[SecureContext, Exposed=(Window)]
interface BerytusSecurePasswordFieldValue : BerytusFieldValueDictionary {
    [Throws]
    readonly attribute (ArrayBuffer or BerytusEncryptedPacket) salt;
    [Throws]
    readonly attribute (ArrayBuffer or BerytusEncryptedPacket) verifier;
};

[SecureContext, Exposed=(Window)]
interface BerytusSecurePasswordField : BerytusField {
    /* -- readonly attribute BerytusSecurePasswordFieldOptions options; */
    /* -- readonly attribute BerytusSecurePasswordValue value; */

    [Throws]
    constructor(
        DOMString id,
        BerytusSecurePasswordFieldOptions options
    );
};