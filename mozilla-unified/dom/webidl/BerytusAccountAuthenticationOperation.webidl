[SecureContext, Exposed=(Window)]
interface BerytusAccountAuthenticationOperation : BerytusLoginOperation {
    readonly attribute BerytusChallengeMap challenges;

    [Throws]
    Promise<BerytusChallenge> createChallenge(
        DOMString challengeId,
        AuthRealmChallengeType challengeType,
        optional BerytusChallengeParameters? challengeParameters
    );

    [Throws]
    Promise<undefined> finish();
};

BerytusAccountAuthenticationOperation includes BerytusAccountMetadata;

/**
 * Work In Progress: Enums for challenge messages.
 * These will be moved to separate files once we introduce interfaces
 * for each challenge type.
 */

enum BerytusIdentificationChallengeMessageName {
    "GetIdentityFields"
};

enum BerytusPasswordChallengeMessageName {
    "GetPasswordFields"
};

enum BerytusSecureRemotePasswordChallengeMessageName {
    "SelectSecurePasswordField",
    "ExchangePublicKeys",
    "ComputeClientProof",
    "VerifyServerProof"
};

enum BerytusDigitalSignatureChallengeMessageName {
    "GetPublicKey",
    "SignNonce"
};

enum BerytusForeignIdentityOtpChallengeMessageName {
    "GetOtp"
};