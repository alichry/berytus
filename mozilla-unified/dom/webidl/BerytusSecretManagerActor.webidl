[SecureContext, Exposed=(Window)]
interface BerytusSecretManagerActor {
    /**
     * Ed25519 public key of the Secret Manager.
     * SPKI Hex-encoded. Anonymous Secret Managers
     * should not reuse ed25519 keys across sessions.
     */
    readonly attribute DOMString ed25519Key;
};