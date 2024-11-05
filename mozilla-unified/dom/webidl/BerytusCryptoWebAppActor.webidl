[SecureContext, Exposed=(Window)]
interface BerytusCryptoWebAppActor : BerytusWebAppActor {
    /**
     * The SPKI base64 string representation of the
     * Ed25519 Public Key of the Subsystem.
     */
    readonly attribute DOMString ed25519Key;

    /**
     * @param ed25519 The Subsystem's Ed25519 public key.
     * @param handler Proof handler
     */
    [Throws]
    constructor(DOMString ed25519Key);
};