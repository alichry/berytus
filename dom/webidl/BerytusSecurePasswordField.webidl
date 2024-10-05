[SecureContext, Exposed=(Window)]
interface BerytusSecurePasswordFieldValue : BerytusFieldValueDictionary {
    readonly attribute (DOMString or BerytusEncryptedPacket) salt;
    readonly attribute (DOMString or BerytusEncryptedPacket) verifier;
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