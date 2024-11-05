[SecureContext, Exposed=(Window)]
interface BerytusIdentityField : BerytusField {
    /* -- readonly attribute BerytusIdentityFieldOptions options; */

    [Throws]
    constructor(
        DOMString id,
        BerytusIdentityFieldOptions options,
        optional (DOMString or BerytusEncryptedPacket) desiredValue
    );
};