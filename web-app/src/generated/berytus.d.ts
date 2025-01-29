export {};
declare global {
	type BerytusAccountStatus = "Pending" | "Created" | "Retired";
	type BerytusAccountCategory = string;
	type BerytusAccountVersion = number;
	interface BerytusFieldRejectionParameters {
	    field: BerytusField | string;
	    reason: BerytusFieldRejectionReasonCode;
	    newValue?: BerytusFieldValue;
	}
	interface BerytusAccountMetadata {
	    readonly category: BerytusAccountCategory;
	    readonly version: BerytusAccountVersion;
	    readonly status: BerytusAccountStatus;
	    readonly changePasswordUrl: string | null;
	}
	interface BerytusAccountWritableMetadata {
	    setCategory(category: BerytusAccountCategory): Promise<undefined>;
	    setStatus(status: BerytusAccountStatus): Promise<undefined>;
	    setVersion(version: BerytusAccountVersion): Promise<undefined>;
	    setChangePasswordUrl(url: string): Promise<undefined>;
	}
	interface BerytusAccount {
	    readonly userAttributes: BerytusUserAttributeMap;
	    readonly fields: BerytusFieldMap;
	    addFields(...field: Array<BerytusField>): Promise<Record<string, BerytusField>>;
	    rejectAndReviseFields(...rejectionParameters: Array<BerytusFieldRejectionParameters>): Promise<Record<string, BerytusField>>;
	    setUserAttributes(attributes: Array<BerytusUserAttributeDefinition>): Promise<undefined>;
	}
	interface BerytusAccountAuthenticationOperation extends BerytusLoginOperation {
		readonly intent: "Authenticate";
	    readonly challenges: BerytusChallengeMap;
	    challenge(challenge: BerytusChallenge): Promise<undefined>;
	    finish(): Promise<undefined>;
	}
	interface BerytusAccountAuthenticationOperation extends BerytusAccountMetadata {
	}
	interface BerytusAccountCreationOperation extends BerytusLoginOperation {
		readonly intent: "Register";
	    readonly newborn: boolean;
	    save(): Promise<undefined>;
	    transitionToAuthOperation(): Promise<BerytusAccountAuthenticationOperation>;
	}
	interface BerytusAccountCreationOperation extends BerytusAccount {
	}
	interface BerytusAccountCreationOperation extends BerytusAccountMetadata {
	}
	interface BerytusAccountCreationOperation extends BerytusAccountWritableMetadata {
	}
	interface BerytusAnonymousWebAppActor extends BerytusWebAppActor {
	    new ();
	}
	type BerytusChallengeMap = ReadonlyMap<string, BerytusChallenge>;
	interface BerytusChallengeParameters {
	}
	interface BerytusAccountConstraints {
	    category?: BerytusAccountCategory;
	    schemaVersion?: BerytusAccountVersion;
	    identity?: Record<string, string>;
	}
	interface BerytusChannelConstraints {
	    secretManagerPublicKey?: Array<string>;
	    account?: BerytusAccountConstraints;
	    enableEndToEndEncryption?: boolean;
	}
	interface BerytusChannelOptions {
	    webApp: BerytusWebAppActor;
	    constraints?: BerytusChannelConstraints;
	}
	type BerytusOnboardingIntent = "Unknown" | "Authenticate" | "Register";
	interface BerytusOnboardingOptions {
	    intent?: BerytusOnboardingIntent;
	    requiredUserAttributes?: Record<string, boolean>;
	}
	interface BerytusChannel {
	    readonly active: boolean;
	    readonly webApp: BerytusWebAppActor;
	    readonly secretManager: BerytusSecretManagerActor | null;
	    readonly keyAgreementParams: BerytusKeyAgreementParameters | null;
	    readonly constraints: any | null;
	    static create(options?: BerytusChannelOptions): Promise<BerytusChannel>;
	    close(): Promise<undefined>;
	    login(options?: BerytusOnboardingOptions): Promise<BerytusAccountAuthenticationOperation | BerytusAccountCreationOperation>;
	    prepareKeyAgreementParameters(webAppX25519PublicKey: string): Promise<BerytusKeyAgreementParameters>;
	}
	interface BerytusCryptoWebAppActor extends BerytusWebAppActor {
	    readonly ed25519Key: string;
	    new (ed25519Key: string);
	}
	type BerytusPlaintextStringSource = string;
	type BerytusPlaintextBufferSource = BufferSource;
	type BerytusPlaintextSource = BerytusPlaintextStringSource | BerytusPlaintextBufferSource;
	type BerytusCiphertextSource = BerytusEncryptedPacket;
	type BerytusDataSource = BerytusPlaintextSource | BerytusCiphertextSource;
	type BerytusDataType = string | ArrayBuffer | BerytusEncryptedPacket;
	type BerytusEncryptionParams = AesGcmParams;
	interface AesGcmParamsJSON extends Algorithm {
	    iv: Base64URLString;
	    additionalData?: Base64URLString;
	    tagLength?: number;
	}
	type BerytusEncryptionParamsJSON = AesGcmParamsJSON;
	interface BerytusEncryptedPacketJSON {
	    parameters: BerytusEncryptionParamsJSON;
	    ciphertext: Base64URLString;
	}
	interface BerytusEncryptedPacket {
	    readonly parameters: any;
	    readonly ciphertext: ArrayBuffer;
	    new (algorithm: BerytusEncryptionParams, ciphertext: BufferSource);
	    toJSON(): BerytusEncryptedPacketJSON;
	}
	type BerytusFieldType = "Identity" | "ForeignIdentity" | "Password" | "SecurePassword" | "ConsumablePassword" | "Key" | "SharedKey" | "Custom";
	type BerytusFieldRejectionReasonCode = string;
	type BerytusFieldId = string;
	type BerytusFieldValue = string | BerytusEncryptedPacket | BerytusFieldValueDictionary;
	interface BerytusField {
	    readonly id: BerytusFieldId;
	    readonly type: BerytusFieldType;
	    readonly options: any;
	    readonly value: BerytusFieldValue | null;
	    toJSON(): any;
	}
	type BerytusFieldMap = ReadonlyMap<string, BerytusField>;
	interface BerytusFieldCategoryOptions {
	    categoryId: string;
	    position?: number;
	}
	interface BerytusBaseFieldOptions {
	    category?: BerytusFieldCategoryOptions;
	}
	interface BerytusIdentityFieldOptions extends BerytusBaseFieldOptions {
	    humanReadable: boolean;
	    private: boolean;
	    maxLength: number;
	    allowedCharacters?: string;
	}
	type BerytusForeignIdentityKind = string;
	interface BerytusForeignIdentityFieldOptions extends BerytusBaseFieldOptions {
	    private: boolean;
	    kind: BerytusForeignIdentityKind;
	}
	interface BerytusPasswordFieldOptions extends BerytusBaseFieldOptions {
	    passwordRules?: string;
	}
	interface BerytusConsumablePasswordFieldOptions extends BerytusPasswordFieldOptions {
	    passwordIdentifier?: string;
	}
	interface BerytusSecurePasswordFieldOptions extends BerytusBaseFieldOptions {
	    identityFieldId: string;
	}
	interface BerytusKeyFieldOptions extends BerytusBaseFieldOptions {
	    alg: COSEAlgorithmIdentifier;
	}
	interface BerytusSharedKeyFieldOptions extends BerytusKeyFieldOptions {
	}
	interface BerytusCustomFieldOptions extends BerytusBaseFieldOptions {
	    mimeType: string;
	    info: string;
	    parameters?: Record<string, string | number | boolean | BufferSource>;
	}
	interface BerytusFieldValueDictionary {
	}
	interface BerytusForeignIdentityField extends BerytusField {
	    new (id: string, options: BerytusForeignIdentityFieldOptions, desiredValue?: BerytusPlaintextStringSource | BerytusCiphertextSource);
	}
	interface BerytusIdentityField extends BerytusField {
	    new (id: string, options: BerytusIdentityFieldOptions, desiredValue?: string | BerytusEncryptedPacket);
	}
	interface BerytusKeyAgreementParameters {
	    readonly sessionId: string;
	    readonly webAppX25519Key: string;
	    readonly scmX25519Key: string;
	    readonly hkdfHash: string;
	    readonly hkdfSalt: ArrayBuffer;
	    readonly hkdfInfo: ArrayBuffer;
	    readonly aesKeyLength: number;
	    toJSON(): string;
	}
	interface BerytusKeyFieldValue extends BerytusFieldValueDictionary {
	    readonly publicKey: ArrayBuffer | BerytusEncryptedPacket;
	}
	interface BerytusKeyField extends BerytusField {
	    new (id: string, options: BerytusKeyFieldOptions);
	}
	interface BerytusLoginOperation {
	    readonly intent: BerytusOnboardingIntent;
	}
	interface BerytusPasswordField extends BerytusField {
	    new (id: string, options?: BerytusPasswordFieldOptions, desiredValue?: BerytusPlaintextStringSource | BerytusCiphertextSource);
	}
	interface BerytusSecretManagerActor {
	    readonly ed25519Key: string;
	}
	interface BerytusSecurePasswordFieldValue extends BerytusFieldValueDictionary {
	    readonly salt: ArrayBuffer | BerytusEncryptedPacket;
	    readonly verifier: ArrayBuffer | BerytusEncryptedPacket;
	}
	interface BerytusSecurePasswordField extends BerytusField {
	    new (id: string, options: BerytusSecurePasswordFieldOptions);
	}
	interface BerytusSharedKeyFieldValue extends BerytusFieldValueDictionary {
	    readonly privateKey: ArrayBuffer | BerytusEncryptedPacket;
	    new (privateKey: BufferSource | BerytusEncryptedPacket);
	}
	interface BerytusSharedKeyField extends BerytusField {
	    new (id: string, options: BerytusSharedKeyFieldOptions, desiredPrivateKeyValue?: BerytusSharedKeyFieldValue);
	}
	type BerytusUserAttributeKey = string;
	interface BerytusUserAttributeDefinition {
	    id: BerytusUserAttributeKey;
	    info?: string;
	    mimeType?: string;
	    value: string | BufferSource | BerytusEncryptedPacket;
	}
	type BerytusUserAttributeValueEncodingType = "None" | "Base64URLString" | "EncryptedPacketJSON";
	interface BerytusUserAttributeJSON {
	    id: BerytusUserAttributeKey;
	    info?: string;
	    mimeType?: string;
	    encoding: BerytusUserAttributeValueEncodingType;
	    value: string | BerytusEncryptedPacketJSON;
	}
	interface BerytusUserAttribute {
	    readonly id: BerytusUserAttributeKey;
	    readonly mimeType: string | null;
	    readonly info: string | null;
	    readonly value: string | ArrayBuffer | BerytusEncryptedPacket;
	    toJSON(): BerytusUserAttributeJSON;
	}
	type BerytusUserAttributeMap = ReadonlyMap<BerytusUserAttributeKey, BerytusUserAttribute>;
	interface BerytusWebAppActor {
	}
	type BerytusChallengeType = "Identification" | "DigitalSignature" | "Password" | "SecureRemotePassword" | "OffChannelOtp";
	type BerytusChallengeAbortionCode = "GenericWebAppFailure" | "UserInterrupt" | "IdentityDoesNotExists" | "IncorrectPassword" | "InvalidProof" | "PublicKeyMismatch" | "InvalidSignature" | "IncorrectOtp";
	type BerytusChallengeId = string;
	interface BerytusChallenge {
	    readonly id: BerytusChallengeId;
	    readonly type: BerytusChallengeType;
	    readonly parameters: any | null;
	    readonly active: boolean;
	    send(messageDef: BerytusChallengeMessageRequestDefinition): Promise<BerytusChallengeMessageResponseDefinition>;
	    seal(): Promise<undefined>;
	    abort(abortionReasonCode: BerytusChallengeAbortionCode): Promise<undefined>;
	    abortWithUserInterruptError(): Promise<undefined>;
	    abortWithGenericWebAppFailureError(): Promise<undefined>;
	}
	interface BerytusChallengeGetIdentityFieldsMessageResponse {
	    response: Record<string, string | BerytusEncryptedPacket>;
	}
	interface BerytusIdentificationChallenge extends BerytusChallenge {
	    new (id: string);
	    getIdentityFields(identityFieldIds: Array<string>): Promise<BerytusChallengeGetIdentityFieldsMessageResponse>;
	    abortWithIdentityDoesNotExistsError(): Promise<undefined>;
	}
	interface BerytusChallengeGetPasswordFieldsMessageResponse {
	    response: Record<string, string | BerytusEncryptedPacket>;
	}
	interface BerytusPasswordChallenge extends BerytusChallenge {
	    new (id: string);
	    getPasswordFields(passwordFieldIds: Array<string>): Promise<BerytusChallengeGetPasswordFieldsMessageResponse>;
	    abortWithIncorrectPasswordError(): Promise<undefined>;
	}
	interface BerytusChallengeSelectKeyMessageResponse {
	    response: BerytusKeyFieldValue;
	}
	interface BerytusChallengeSignNonceMessageResponse {
	    response: ArrayBuffer;
	}
	interface BerytusDigitalSignatureChallenge extends BerytusChallenge {
	    new (id: string);
	    selectKey(keyFieldId: string): Promise<BerytusChallengeSelectKeyMessageResponse>;
	    signNonce(nonce: ArrayBuffer | ArrayBufferView | BerytusEncryptedPacket): Promise<BerytusChallengeSignNonceMessageResponse>;
	    abortWithPublicKeyMismatchError(): Promise<undefined>;
	    abortWithInvalidSignatureError(): Promise<undefined>;
	}
	interface BerytusChallengeSelectSecurePasswordMessageResponse {
	    response: string | BerytusEncryptedPacket;
	}
	interface BerytusChallengeExchangePublicKeysMessageResponse {
	    response: string | ArrayBuffer | BerytusEncryptedPacket;
	}
	type BerytusSecureRemotePasswordChallengeEncodingType = "None" | "Hex";
	interface BerytusSecureRemotePasswordChallengeParameters {
	    encoding?: BerytusSecureRemotePasswordChallengeEncodingType;
	}
	interface BerytusChallengeComputeClientProofMessageResponse {
	    response: string | ArrayBuffer | BerytusEncryptedPacket;
	}
	interface BerytusChallengeVerifyServerProofMessageResponse {
	}
	interface BerytusSecureRemotePasswordChallenge extends BerytusChallenge {
	    new (id: string, parameters?: BerytusSecureRemotePasswordChallengeParameters);
	    selectSecurePassword(securePasswordFieldId: string): Promise<BerytusChallengeSelectSecurePasswordMessageResponse>;
	    exchangePublicKeys(webAppServerPublicKeyB: ArrayBuffer | ArrayBufferView | string | BerytusEncryptedPacket): Promise<BerytusChallengeExchangePublicKeysMessageResponse>;
	    computeClientProof(salt: ArrayBuffer | ArrayBufferView | string | BerytusEncryptedPacket): Promise<BerytusChallengeComputeClientProofMessageResponse>;
	    verifyServerProof(serverProofM2: ArrayBuffer | ArrayBufferView | string | BerytusEncryptedPacket): Promise<BerytusChallengeVerifyServerProofMessageResponse>;
	    abortWithInvalidProofError(): Promise<undefined>;
	}
	interface BerytusChallengeGetOtpMessageResponse {
	    response: string | BerytusEncryptedPacket;
	}
	interface BerytusOffChannelOtpChallenge extends BerytusChallenge {
	    new (id: string);
	    getOtp(foreignIdentityFieldId: string): Promise<BerytusChallengeGetOtpMessageResponse>;
	    abortWithIncorrectOtpError(): Promise<undefined>;
	}
	type BerytusIdentificationChallengeMessageName = "GetIdentityFields";
	type BerytusPasswordChallengeMessageName = "GetPasswordFields";
	type BerytusSecureRemotePasswordChallengeMessageName = "SelectSecurePassword" | "ExchangePublicKeys" | "ComputeClientProof" | "VerifyServerProof";
	type BerytusDigitalSignatureChallengeMessageName = "SelectKey" | "SignNonce";
	type BerytusOffChannelOtpChallengeMessageName = "GetOtp";
	declare var BerytusAccountAuthenticationOperation: BerytusAccountAuthenticationOperation;
	declare var BerytusAccountCreationOperation: BerytusAccountCreationOperation;
	declare var BerytusAnonymousWebAppActor: BerytusAnonymousWebAppActor;
	declare var BerytusChallengeMap: BerytusChallengeMap;
	declare var BerytusChallengeParameters: BerytusChallengeParameters;
	declare var BerytusChannel: BerytusChannel;
	declare var BerytusCryptoWebAppActor: BerytusCryptoWebAppActor;
	declare var BerytusEncryptedPacket: BerytusEncryptedPacket;
	declare var BerytusField: BerytusField;
	declare var BerytusFieldMap: BerytusFieldMap;
	declare var BerytusFieldValueDictionary: BerytusFieldValueDictionary;
	declare var BerytusForeignIdentityField: BerytusForeignIdentityField;
	declare var BerytusIdentityField: BerytusIdentityField;
	declare var BerytusKeyAgreementParameters: BerytusKeyAgreementParameters;
	declare var BerytusKeyFieldValue: BerytusKeyFieldValue;
	declare var BerytusKeyField: BerytusKeyField;
	declare var BerytusLoginOperation: BerytusLoginOperation;
	declare var BerytusPasswordField: BerytusPasswordField;
	declare var BerytusSecretManagerActor: BerytusSecretManagerActor;
	declare var BerytusSecurePasswordFieldValue: BerytusSecurePasswordFieldValue;
	declare var BerytusSecurePasswordField: BerytusSecurePasswordField;
	declare var BerytusSharedKeyFieldValue: BerytusSharedKeyFieldValue;
	declare var BerytusSharedKeyField: BerytusSharedKeyField;
	declare var BerytusUserAttribute: BerytusUserAttribute;
	declare var BerytusUserAttributeMap: BerytusUserAttributeMap;
	declare var BerytusWebAppActor: BerytusWebAppActor;
	declare var BerytusChallenge: BerytusChallenge;
	declare var BerytusIdentificationChallenge: BerytusIdentificationChallenge;
	declare var BerytusPasswordChallenge: BerytusPasswordChallenge;
	declare var BerytusDigitalSignatureChallenge: BerytusDigitalSignatureChallenge;
	declare var BerytusSecureRemotePasswordChallenge: BerytusSecureRemotePasswordChallenge;
	declare var BerytusOffChannelOtpChallenge: BerytusOffChannelOtpChallenge;
}
