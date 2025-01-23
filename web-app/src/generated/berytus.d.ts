export {};
declare global {
	type BerytusAccountStatus = "Pending" | "Created" | "Retired";
	interface BerytusUserAttributeDefinition {
	    id: BerytusUserAttributeKey;
	    info?: string;
	    mimeType?: string;
	    value: BerytusDataSource;
	}
	type BerytusAccountCategory = string;
	type BerytusAccountVersion = number;
	interface BerytusFieldRejectionParameters {
	    field: BerytusField | string;
	    reason: BerytusFieldRejectionReasonCode;
	    newValue?: BerytusDataSource | BerytusFieldValueDictionary;
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
	    setUserAttributes(attributes: Record<string, BerytusUserAttributeDefinition>): Promise<undefined>;
	    addFieldCategory(id: string, info: string): Promise<undefined>;
	}
	interface BerytusAccountAuthenticationOperation extends BerytusLoginOperation {
		readonly intent: "Authenticate";
	    readonly challenges: BerytusChallengeMap;
	    createChallenge(challengeId: string, challengeType: AuthRealmChallengeType, challengeParameters?: BerytusChallengeParameters | null): Promise<AuthRealmChallenge>;
	    finish(): Promise<undefined>;
	}
	interface BerytusAccountAuthenticationOperation extends BerytusAccountMetadata {
	}
	type BerytusIdentificationChallengeMessageName = "GetIdentityFields";
	type BerytusPasswordChallengeMessageName = "GetPasswordFields";
	type BerytusSecureRemotePasswordChallengeMessageName = "SelectSecurePasswordField" | "ExchangePublicKeys" | "ComputeClientProof" | "VerifyServerProof";
	type BerytusDigitalSignatureChallengeMessageName = "GetPublicKey" | "SignNonce";
	type BerytusForeignIdentityOtpChallengeMessageName = "GetOtp";
	interface BerytusAccountCreationOperation extends BerytusLoginOperation {
		readonly intent: "Register";
	    readonly newborn: boolean;
	    readonly creationProofRequired: boolean;
	    save(signature?: BufferSource): Promise<undefined>;
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
	type BerytusChallengeMap = ReadonlyMap<string, AuthRealmChallenge>;
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
	    enableTokenization?: boolean;
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
	    static create(options?: BerytusChannelOptions): Promise<BerytusChannel>;
	    close(): Promise<undefined>;
	    login(options?: BerytusOnboardingOptions): Promise<BerytusAccountAuthenticationOperation | BerytusAccountCreationOperation>;
	    prepareKeyAgreementParameters(webAppX25519PublicKey: string): Promise<BerytusKeyAgreementParameters>;
	    enableEndToEndEncryption(keyAgreementSignature: ArrayBuffer): Promise<string>;
	}
	interface BerytusCryptoWebAppActor extends BerytusWebAppActor {
	    readonly ed25519Key: string;
	    new (ed25519Key: string);
	}
	type BerytusPlaintextStringSource = UTF8String;
	type BerytusPlaintextBufferSource = BufferSource;
	type BerytusPlaintextSource = BerytusPlaintextStringSource | BerytusPlaintextBufferSource;
	type BerytusCiphertextSource = BerytusEncryptedPacket;
	type BerytusDataSource = BerytusPlaintextSource | BerytusCiphertextSource;
	type BerytusDataType = string | ArrayBuffer | BerytusEncryptedPacket;
	interface BerytusEncryptedPacket {
	    new (ciphertext: BufferSource, iv: BufferSource);
	    readonly iv: ArrayBuffer;
	    readonly ciphertext: ArrayBuffer;
	}
	type BerytusFieldType = "Identity" | "ForeignIdentity" | "Password" | "SecurePassword" | "ConsumablePassword" | "Key" | "SharedKey" | "Custom";
	type BerytusFieldRejectionReasonCode = string;
	interface BerytusField {
	    readonly id: string;
	    readonly type: BerytusFieldType;
	    readonly options: any;
	    readonly value: BerytusDataType | BerytusFieldValueDictionary;
	    readonly canBeRejected: boolean;
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
	    parameters?: Record<string, string | number | boolean | ArrayBuffer>;
	}
	interface BerytusFieldValueDictionary {
	}
	interface BerytusForeignIdentityField extends BerytusField {
	    new (id: string, options: BerytusForeignIdentityFieldOptions, desiredValue?: BerytusPlaintextStringSource | BerytusCiphertextSource);
	}
	interface BerytusIdentityField extends BerytusField {
	    new (id: string, options: BerytusIdentityFieldOptions, desiredValue?: BerytusPlaintextStringSource | BerytusCiphertextSource);
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
	    readonly salt: string | BerytusEncryptedPacket;
	    readonly verifier: string | BerytusEncryptedPacket;
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
	interface BerytusUserAttribute {
	    readonly id: BerytusUserAttributeKey;
	    readonly mimeType: string | null;
	    readonly info: string | null;
	    readonly value: BerytusDataType;
	}
	type BerytusUserAttributeMap = ReadonlyMap<BerytusUserAttributeKey, BerytusUserAttribute>;
	interface BerytusWebAppActor {
	}
	interface AuthRealmChallengeMessage {
	    name: string;
	    payload: any;
	}
	interface AuthRealmChallengeMessageResponse {
	    payload: any;
	}
	type AuthRealmChallengeType = "Identification" | "DigitalSignature" | "Password" | "SecureRemotePassword" | "ForeignIdentityOtp";
	interface AuthRealmChallenge {
	    readonly id: string;
	    readonly type: AuthRealmChallengeType;
	    readonly active: boolean;
	    sendMessage(message: AuthRealmChallengeMessage): Promise<AuthRealmChallengeMessageResponse>;
	    seal(): Promise<undefined>;
	    abort(abortionReasonCode: string): Promise<undefined>;
	}
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
	declare var AuthRealmChallenge: AuthRealmChallenge;
}
