enum BerytusChallengeType {
  "Identification",
  "DigitalSignature",
  "Password",
  "SecureRemotePassword",
  "OffChannelOtp",
};

enum BerytusChallengeAbortionCode {
  "GenericWebAppFailure",
  "UserInterrupt",
  "IdentityDoesNotExists",
  "IncorrectPassword",
  "InvalidProof",
  "InvalidSignature",
  "IncorrectOtp"
};

typedef DOMString BerytusChallengeId;

[SecureContext, Exposed=(Window)]
interface BerytusChallenge {
  readonly attribute BerytusChallengeId id;
  readonly attribute BerytusChallengeType type;
  [Throws]
  readonly attribute object? parameters;
  readonly attribute boolean active;

  [Throws]
  Promise<BerytusChallengeMessageResponseDefinition> send(BerytusChallengeMessageRequestDefinition messageDef);

  [Throws]
  Promise<undefined> seal();

  [Throws]
  Promise<undefined> abort(BerytusChallengeAbortionCode abortionReasonCode);

  [Throws]
  Promise<undefined> abortWithUserInterruptError();

  [Throws]
  Promise<undefined> abortWithGenericWebAppFailureError();
};

[GenerateInit]
dictionary BerytusChallengeGetIdentityFieldsMessageResponse {
  /**
   * Implementation should ensure it is of type:
   *   (record<DOMString, DOMString> or record<DOMString, BerytusEncryptedPacket>)
   */
  required record<DOMString, (DOMString or BerytusEncryptedPacket)> response;
};

[SecureContext, Exposed=(Window)]
interface BerytusIdentificationChallenge : BerytusChallenge {
  [Throws]
  constructor(DOMString id);

  [Throws]
  Promise<BerytusChallengeGetIdentityFieldsMessageResponse> getIdentityFields(
    sequence<DOMString> identityFieldIds
  );
  [Throws]
  Promise<undefined> abortWithIdentityDoesNotExistsError();
};

dictionary BerytusChallengeGetPasswordFieldsMessageResponse {
  /**
   * Implementation should ensure it is of type:
   *   (record<DOMString, DOMString> or record<DOMString, BerytusEncryptedPacket>)
   */
  required record<DOMString, (DOMString or BerytusEncryptedPacket)> response;
};

[SecureContext, Exposed=(Window)]
interface BerytusPasswordChallenge : BerytusChallenge {
  [Throws]
  constructor(DOMString id);

  [Throws]
  Promise<BerytusChallengeGetPasswordFieldsMessageResponse> getPasswordFields(
    sequence<DOMString> passwordFieldIds
  );
  [Throws]
  Promise<undefined> abortWithIncorrectPasswordError();
};

[GenerateInit]
dictionary BerytusChallengeSelectKeyMessageResponse {
  required BerytusKeyFieldValue response;
};

[GenerateInit]
dictionary BerytusChallengeSignNonceMessageResponse {
  required ArrayBuffer response;
};

[SecureContext, Exposed=(Window)]
interface BerytusDigitalSignatureChallenge : BerytusChallenge {
  [Throws]
  constructor(DOMString id);

  [Throws]
  Promise<BerytusChallengeSelectKeyMessageResponse> selectKey(
    DOMString keyFieldId
  );
  [Throws]
  Promise<BerytusChallengeSignNonceMessageResponse> signNonce(
    (ArrayBuffer or ArrayBufferView or BerytusEncryptedPacket) nonce
  );
  [Throws]
  Promise<undefined> abortWithInvalidSignatureError();
};

[GenerateInit]
dictionary BerytusChallengeSelectSecurePasswordMessageResponse {
  /**
    * The identity field value that is bound to the selected secure password field.
    * See BerytusSecurePasswordFieldOptions -> identityFieldId. This
    * value could be wrapped in a BerytusEncryptedPacket, depending
    * whether app-level E2EE is enabled.
    */
  required (DOMString or BerytusEncryptedPacket) response;
};

[GenerateInit]
dictionary BerytusChallengeExchangePublicKeysMessageResponse {
  /**
   * The client public key (SRP:A) - As hex string or as an ArrayBuffer(View).
   * By default, no hex encoding is applied. To change this, specify the encoding
   * type in the challenge parameters. In all cases, this value could
   * be wrapped in a BerytusEncryptedPacket, depending whether app-level
   * E2EE is enabled.
   */
  required (DOMString or ArrayBuffer or BerytusEncryptedPacket) response;
};

enum BerytusSecureRemotePasswordChallengeEncodingType {
  "None",
  "Hex"
};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusSecureRemotePasswordChallengeParameters {
  /**
   * Defaults to "None"
   */
  BerytusSecureRemotePasswordChallengeEncodingType encoding;
};

[GenerateInit]
dictionary BerytusChallengeComputeClientProofMessageResponse {
  /**
   * The client proof (SRP:M1) - As hex string or as an ArrayBuffer(View).
   * By default, no hex encoding is applied. To change this, specify the encoding
   * type in the challenge parameters. In all cases, this value could
   * be wrapped in a BerytusEncryptedPacket, depending whether app-level
   * E2EE is enabled.
   */
  required (DOMString or ArrayBuffer or BerytusEncryptedPacket) response;
};

[GenerateInit]
dictionary BerytusChallengeVerifyServerProofMessageResponse {
  // nothing
};

[SecureContext, Exposed=(Window)]
interface BerytusSecureRemotePasswordChallenge : BerytusChallenge {
  [Throws]
  constructor(DOMString id, optional BerytusSecureRemotePasswordChallengeParameters parameters = {});

  [Throws]
  Promise<BerytusChallengeSelectSecurePasswordMessageResponse> selectSecurePassword(
    DOMString securePasswordFieldId
  );

  /**
   * SRP:B - As a hex string or as an ArrayBuffer(View). By default,
   * no hex encoding should be applied. To change this, specify the encoding
   * type in the challenge parameters. In all cases, this value should
   * be wrapped in a BerytusEncryptedPacket if app-level E2EE is enabled.
   */
  [Throws]
  Promise<BerytusChallengeExchangePublicKeysMessageResponse> exchangePublicKeys(
    (ArrayBuffer or ArrayBufferView or DOMString or BerytusEncryptedPacket) webAppServerPublicKeyB
  );

  /**
   * SRP:salt - As a hex string or as an ArrayBuffer(View). By default,
   * no hex encoding should be applied. To change this, specify the encoding
   * type in the challenge parameters. In all cases, this value should
   * be wrapped in a BerytusEncryptedPacket if app-level E2EE is enabled.
   */
  [Throws]
  Promise<BerytusChallengeComputeClientProofMessageResponse> computeClientProof(
    (ArrayBuffer or ArrayBufferView or DOMString or BerytusEncryptedPacket) salt
  );

  /**
   * SRP:M2 - As a hex string or as an ArrayBuffer(View). By default,
   * no hex encoding should be applied. To change this, specify the encoding
   * type in the challenge parameters. In all cases, this value should
   * be wrapped in a BerytusEncryptedPacket if app-level E2EE is enabled.
   */
  [Throws]
  Promise<BerytusChallengeVerifyServerProofMessageResponse> verifyServerProof(
    (ArrayBuffer or ArrayBufferView or DOMString or BerytusEncryptedPacket) serverProofM2
  );
  [Throws]
  Promise<undefined> abortWithInvalidProofError();
};

[GenerateInit]
dictionary BerytusChallengeGetOtpMessageResponse {
  required (DOMString or BerytusEncryptedPacket) response;
};

[SecureContext, Exposed=(Window)]
interface BerytusOffChannelOtpChallenge : BerytusChallenge {
  [Throws]
  constructor(DOMString id);

  [Throws]
  Promise<BerytusChallengeGetOtpMessageResponse> getOtp(
    DOMString foreignIdentityFieldId
  );
  [Throws]
  Promise<undefined> abortWithIncorrectOtpError();
};

// TODO(berytus): field Ids specifiied in
// getOtp, getPasswordFields, getIdentityFields
// should be defined in the parameters of the challenge.