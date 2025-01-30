[SecureContext, Exposed=(Window)]
interface BerytusPasswordField : BerytusField {
    /* -- readonly attribute BerytusPasswordFieldOptions options; */

    [Throws]
    constructor(
        DOMString id,
        optional BerytusPasswordFieldOptions options = {},
        optional (BerytusPlaintextStringSource or BerytusCiphertextSource) desiredValue
    );
};