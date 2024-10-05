[SecureContext, Exposed=(Window)]
interface BerytusForeignIdentityField : BerytusField {
    /* -- readonly attribute BerytusForeignIdentityFieldOptions options; */

    [Throws]
    constructor(
        DOMString id,
        BerytusForeignIdentityFieldOptions options,
        optional (BerytusPlaintextStringSource or BerytusCiphertextSource) desiredValue
    );
};