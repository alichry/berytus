[SecureContext, Exposed=(Window)]
interface BerytusAccountCreationOperation : BerytusLoginOperation {

    /**
     * This attribute indicates whether the account
     * record was just created or was previously created.
     * This is important when it comes to resuming
     * the registration process.
     */
    readonly attribute boolean newborn;

    /**
     * Attempt to save this account in the Secret Manager.
     * TODO(berytus): Add operation commitment signature arg.
     */
    [Throws]
    Promise<undefined> save();

    [Throws]
    Promise<BerytusAccountAuthenticationOperation> transitionToAuthOperation();
};

BerytusAccountCreationOperation includes BerytusAccount;
BerytusAccountCreationOperation includes BerytusAccountMetadata;
BerytusAccountCreationOperation includes BerytusAccountWritableMetadata;