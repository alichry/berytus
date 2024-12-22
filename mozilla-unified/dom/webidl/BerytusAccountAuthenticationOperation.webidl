/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

[SecureContext, Exposed=(Window)]
interface BerytusAccountAuthenticationOperation : BerytusLoginOperation {
    readonly attribute BerytusChallengeMap challenges;

    [Throws]
    Promise<BerytusChallenge> createChallenge(
        DOMString challengeId,
        BerytusChallengeType challengeType,
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