type BufferSource = ArrayBuffer | ArrayBufferView;
type Base64URLString = string;
type KeyType = string;
type KeyUsage = string;
type NamedCurve = string;
type BigInteger = Uint8Array;
export interface Algorithm {
    name: string;
}
export interface AesCbcParams extends Algorithm {
    iv: BufferSource;
}
export interface AesCtrParams extends Algorithm {
    counter: BufferSource;
    length: number;
}
export interface AesGcmParams extends Algorithm {
    iv: BufferSource;
    additionalData?: BufferSource;
    tagLength?: number;
}
export interface HmacImportParams extends Algorithm {
    hash: AlgorithmIdentifier;
}
export interface Pbkdf2Params extends Algorithm {
    salt: BufferSource;
    iterations: number;
    hash: AlgorithmIdentifier;
}
export interface RsaHashedImportParams {
    hash: AlgorithmIdentifier;
}
export interface AesKeyGenParams extends Algorithm {
    length: number;
}
export interface HmacKeyGenParams extends Algorithm {
    hash: AlgorithmIdentifier;
    length?: number;
}
export interface RsaHashedKeyGenParams extends Algorithm {
    modulusLength: number;
    publicExponent: BigInteger;
    hash: AlgorithmIdentifier;
}
export interface RsaOaepParams extends Algorithm {
    label?: BufferSource;
}
export interface RsaPssParams extends Algorithm {
    saltLength: number;
}
export interface EcKeyGenParams extends Algorithm {
    namedCurve: NamedCurve;
}
export interface AesDerivedKeyParams extends Algorithm {
    length: number;
}
export interface HmacDerivedKeyParams extends HmacImportParams {
    length?: number;
}
export interface EcdhKeyDeriveParams extends Algorithm {
    public: CryptoKey;
}
export interface DhImportKeyParams extends Algorithm {
    prime: BigInteger;
    generator: BigInteger;
}
export interface EcdsaParams extends Algorithm {
    hash: AlgorithmIdentifier;
}
export interface EcKeyImportParams extends Algorithm {
    namedCurve?: NamedCurve;
}
export interface HkdfParams extends Algorithm {
    hash: AlgorithmIdentifier;
    salt: BufferSource;
    info: BufferSource;
}
export interface RsaOtherPrimesInfo {
    r: string;
    d: string;
    t: string;
}
export interface JsonWebKey {
    kty: string;
    use?: string;
    key_ops?: Array<string>;
    alg?: string;
    ext?: boolean;
    crv?: string;
    x?: string;
    y?: string;
    d?: string;
    n?: string;
    e?: string;
    p?: string;
    q?: string;
    dp?: string;
    dq?: string;
    qi?: string;
    oth?: Array<RsaOtherPrimesInfo>;
    k?: string;
}
export interface CryptoKey {
    readonly type: KeyType;
    readonly extractable: boolean;
    readonly algorithm: any;
    readonly usages: Array<KeyUsage>;
}
export interface CryptoKeyPair {
    publicKey: CryptoKey;
    privateKey: CryptoKey;
}
type KeyFormat = string;
type AlgorithmIdentifier = any | string;
export interface SubtleCrypto {
}
type BerytusPlaintextStringSource = string;
type BerytusPlaintextBufferSource = BufferSource;
type BerytusPlaintextSource = BerytusPlaintextStringSource | BerytusPlaintextBufferSource;
type BerytusCiphertextSource = BerytusEncryptedPacket;
type BerytusDataSource = BerytusPlaintextSource | BerytusCiphertextSource;
type BerytusDataType = string | ArrayBuffer | BerytusEncryptedPacket;
type BerytusEncryptionParams = AesGcmParams;
export interface AesGcmParamsJSON extends Algorithm {
    iv: Base64URLString;
    additionalData?: Base64URLString;
    tagLength?: number;
}
type BerytusEncryptionParamsJSON = AesGcmParamsJSON;
export interface BerytusEncryptedPacketJSON {
    parameters: BerytusEncryptionParamsJSON;
    ciphertext: Base64URLString;
}
export interface BerytusEncryptedPacket {
    readonly parameters: AesGcmParams;
    readonly ciphertext: ArrayBuffer;
}
type BerytusFieldType = "Identity" | "ForeignIdentity" | "Password" | "SecurePassword" | "ConsumablePassword" | "Key" | "SharedKey" | "Custom";
type BerytusFieldRejectionReasonCode = string;
type BerytusFieldId = string;
type BerytusFieldValue = string | BerytusEncryptedPacket | BerytusFieldValueDictionary;
export interface BerytusField {
    readonly id: BerytusFieldId;
    readonly type: BerytusFieldType;
    readonly options: any;
    readonly value: BerytusFieldValue | null;
}
export interface BerytusFieldCategoryOptions {
    categoryId: string;
    position?: number;
}
export interface BerytusBaseFieldOptions {
    category?: BerytusFieldCategoryOptions;
}
export interface BerytusIdentityFieldOptions extends BerytusBaseFieldOptions {
    humanReadable: boolean;
    private: boolean;
    maxLength: number;
    allowedCharacters?: string;
}
type BerytusForeignIdentityKind = string;
export interface BerytusForeignIdentityFieldOptions extends BerytusBaseFieldOptions {
    private: boolean;
    kind: BerytusForeignIdentityKind;
}
export interface BerytusPasswordFieldOptions extends BerytusBaseFieldOptions {
    passwordRules?: string;
}
export interface BerytusConsumablePasswordFieldOptions extends BerytusPasswordFieldOptions {
    passwordIdentifier?: string;
}
export interface BerytusSecurePasswordFieldOptions extends BerytusBaseFieldOptions {
    identityFieldId: string;
}
export interface BerytusKeyFieldOptions extends BerytusBaseFieldOptions {
    alg: COSEAlgorithmIdentifier;
}
export interface BerytusSharedKeyFieldOptions extends BerytusKeyFieldOptions {
}
export interface BerytusCustomFieldOptions extends BerytusBaseFieldOptions {
    mimeType: string;
    info: string;
    parameters?: Record<string, string | number | boolean | BufferSource>;
}
export interface BerytusFieldValueDictionary {
}
export interface BerytusForeignIdentityField extends BerytusField {
    type: EBerytusFieldType.ForeignIdentity;
    options: BerytusForeignIdentityFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
export interface BerytusIdentityField extends BerytusField {
    type: EBerytusFieldType.Identity;
    options: BerytusIdentityFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
export interface BerytusKeyAgreementParameters {
    readonly sessionId: string;
    readonly webAppX25519Key: string;
    readonly scmX25519Key: string;
    readonly hkdfHash: string;
    readonly hkdfSalt: ArrayBuffer;
    readonly hkdfInfo: ArrayBuffer;
    readonly aesKeyLength: number;
}
export interface BerytusKeyFieldValue extends BerytusFieldValueDictionary {
    readonly publicKey: ArrayBufferOrBerytusEncryptedPacket;
}
export interface BerytusKeyField extends BerytusField {
    type: EBerytusFieldType.Key;
    options: BerytusKeyFieldOptions;
    value: nullOrBerytusKeyFieldValue;
}
export interface BerytusPasswordField extends BerytusField {
    type: EBerytusFieldType.Password;
    options: BerytusPasswordFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
export interface BerytusSecurePasswordFieldValue extends BerytusFieldValueDictionary {
    readonly salt: ArrayBufferOrBerytusEncryptedPacket;
    readonly verifier: ArrayBufferOrBerytusEncryptedPacket;
}
export interface BerytusSecurePasswordField extends BerytusField {
    type: EBerytusFieldType.SecurePassword;
    options: BerytusSecurePasswordFieldOptions;
    value: nullOrBerytusSecurePasswordFieldValue;
}
export interface BerytusSharedKeyFieldValue extends BerytusFieldValueDictionary {
    readonly privateKey: ArrayBufferOrBerytusEncryptedPacket;
}
export interface BerytusSharedKeyField extends BerytusField {
    type: EBerytusFieldType.SharedKey;
    options: BerytusSharedKeyFieldOptions;
    value: nullOrBerytusSharedKeyFieldValue;
}
type BerytusUserAttributeKey = string;
export interface BerytusUserAttributeDefinition {
    id: BerytusUserAttributeKey;
    info?: string;
    mimeType?: string;
    value: stringOrBufferSourceOrBerytusEncryptedPacket;
}
type BerytusUserAttributeValueEncodingType = "None" | "Base64URLString" | "EncryptedPacketJSON";
export interface BerytusUserAttributeJSON {
    id: BerytusUserAttributeKey;
    info?: string;
    mimeType?: string;
    encoding: BerytusUserAttributeValueEncodingType;
    value: string | BerytusEncryptedPacketJSON;
}
export interface BerytusUserAttribute {
    readonly id: BerytusUserAttributeKey;
    readonly mimeType: string | null;
    readonly info: string | null;
    readonly value: string | ArrayBuffer | BerytusEncryptedPacket;
}
export interface BerytusChallengeMessageRequestDefinition {
    name: string;
    request: any;
}
export interface BerytusChallengeMessageResponseDefinition {
    response: any;
}
export interface BerytusChallengeMessage {
    name: string;
    request: any;
    response: any;
}
type BerytusChallengeType = "Identification" | "DigitalSignature" | "Password" | "SecureRemotePassword" | "OffChannelOtp";
type BerytusChallengeAbortionCode = "GenericWebAppFailure" | "UserInterrupt" | "IdentityDoesNotExists" | "IncorrectPassword" | "InvalidProof" | "InvalidSignature" | "IncorrectOtp";
type BerytusChallengeId = string;
export interface BerytusChallenge {
    readonly id: BerytusChallengeId;
    readonly type: BerytusChallengeType;
    readonly parameters: any | null;
    readonly active: boolean;
}
export interface BerytusChallengeGetIdentityFieldsMessageResponse {
    response: Record<string, stringOrBerytusEncryptedPacket>;
}
export interface BerytusIdentificationChallenge extends BerytusChallenge {
}
export interface BerytusChallengeGetPasswordFieldsMessageResponse {
    response: Record<string, stringOrBerytusEncryptedPacket>;
}
export interface BerytusPasswordChallenge extends BerytusChallenge {
}
export interface BerytusChallengeSelectKeyMessageResponse {
    response: BerytusKeyFieldValue;
}
export interface BerytusChallengeSignNonceMessageResponse {
    response: ArrayBuffer;
}
export interface BerytusDigitalSignatureChallenge extends BerytusChallenge {
}
export interface BerytusChallengeSelectSecurePasswordMessageResponse {
    response: stringOrBerytusEncryptedPacket;
}
export interface BerytusChallengeExchangePublicKeysMessageResponse {
    response: stringOrArrayBufferOrBerytusEncryptedPacket;
}
type BerytusSecureRemotePasswordChallengeEncodingType = "None" | "Hex";
export interface BerytusSecureRemotePasswordChallengeParameters {
    encoding?: BerytusSecureRemotePasswordChallengeEncodingType;
}
export interface BerytusChallengeComputeClientProofMessageResponse {
    response: stringOrArrayBufferOrBerytusEncryptedPacket;
}
export interface BerytusChallengeVerifyServerProofMessageResponse {
}
export interface BerytusSecureRemotePasswordChallenge extends BerytusChallenge {
}
export interface BerytusChallengeGetOtpMessageResponse {
    response: stringOrBerytusEncryptedPacket;
}
export interface BerytusOffChannelOtpChallenge extends BerytusChallenge {
}
export enum EBerytusFieldType {
	Identity = "Identity",
	ForeignIdentity = "ForeignIdentity",
	Password = "Password",
	SecurePassword = "SecurePassword",
	ConsumablePassword = "ConsumablePassword",
	Key = "Key",
	SharedKey = "SharedKey",
	Custom = "Custom"
}
export type nullOrstringOrBerytusEncryptedPacket = null |
	string |
	BerytusEncryptedPacket;

export type ArrayBufferOrBerytusEncryptedPacket = ArrayBuffer |
	BerytusEncryptedPacket;

export type nullOrBerytusKeyFieldValue = null |
	BerytusKeyFieldValue;

export type nullOrBerytusSecurePasswordFieldValue = null |
	BerytusSecurePasswordFieldValue;

export type nullOrBerytusSharedKeyFieldValue = null |
	BerytusSharedKeyFieldValue;

export type BerytusFieldOptionsUnion = BerytusForeignIdentityFieldOptions
	| BerytusIdentityFieldOptions
	| BerytusKeyFieldOptions
	| BerytusPasswordFieldOptions
	| BerytusSecurePasswordFieldOptions
	| BerytusSharedKeyFieldOptions;
export type stringOrBufferSourceOrBerytusEncryptedPacket = string |
	BufferSource |
	BerytusEncryptedPacket;
export enum EBerytusChallengeType {
	Identification = "Identification",
	DigitalSignature = "DigitalSignature",
	Password = "Password",
	SecureRemotePassword = "SecureRemotePassword",
	OffChannelOtp = "OffChannelOtp"
}

export interface BerytusIdentificationChallengeInfo {
    id: string;
    type: EBerytusChallengeType.Identification;
    parameters: null;
}
export interface BerytusChallengeGetIdentityFieldsMessageRequest {
    payload: string[]
}
export interface BerytusSendGetIdentityFieldsMessage extends BerytusChallengeGetIdentityFieldsMessageRequest {
    challenge: BerytusIdentificationChallengeInfo;
    name: EBerytusIdentificationChallengeMessageName.GetIdentityFields;
}

export enum EBerytusIdentificationChallengeMessageName {
    GetIdentityFields = "GetIdentityFields"
}
export interface BerytusPasswordChallengeInfo {
    id: string;
    type: EBerytusChallengeType.Password;
    parameters: null;
}
export interface BerytusChallengeGetPasswordFieldsMessageRequest {
    payload: string[]
}
export interface BerytusSendGetPasswordFieldsMessage extends BerytusChallengeGetPasswordFieldsMessageRequest {
    challenge: BerytusPasswordChallengeInfo;
    name: EBerytusPasswordChallengeMessageName.GetPasswordFields;
}

export enum EBerytusPasswordChallengeMessageName {
    GetPasswordFields = "GetPasswordFields"
}
export interface BerytusDigitalSignatureChallengeInfo {
    id: string;
    type: EBerytusChallengeType.DigitalSignature;
    parameters: null;
}
export interface BerytusChallengeSelectKeyMessageRequest {
    payload: string
}
export interface BerytusSendSelectKeyMessage extends BerytusChallengeSelectKeyMessageRequest {
    challenge: BerytusDigitalSignatureChallengeInfo;
    name: EBerytusDigitalSignatureChallengeMessageName.SelectKey;
}

export interface BerytusChallengeSignNonceMessageRequest {
    payload: ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket
}
export interface BerytusSendSignNonceMessage extends BerytusChallengeSignNonceMessageRequest {
    challenge: BerytusDigitalSignatureChallengeInfo;
    name: EBerytusDigitalSignatureChallengeMessageName.SignNonce;
}

export enum EBerytusDigitalSignatureChallengeMessageName {
    SelectKey = "SelectKey",
	SignNonce = "SignNonce"
}
export interface BerytusSecureRemotePasswordChallengeInfo {
    id: string;
    type: EBerytusChallengeType.SecureRemotePassword;
    parameters: BerytusSecureRemotePasswordChallengeParameters;
}
export interface BerytusChallengeSelectSecurePasswordMessageRequest {
    payload: string
}
export interface BerytusSendSelectSecurePasswordMessage extends BerytusChallengeSelectSecurePasswordMessageRequest {
    challenge: BerytusSecureRemotePasswordChallengeInfo;
    name: EBerytusSecureRemotePasswordChallengeMessageName.SelectSecurePassword;
}

export interface BerytusChallengeExchangePublicKeysMessageRequest {
    payload: stringOrArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket
}
export interface BerytusSendExchangePublicKeysMessage extends BerytusChallengeExchangePublicKeysMessageRequest {
    challenge: BerytusSecureRemotePasswordChallengeInfo;
    name: EBerytusSecureRemotePasswordChallengeMessageName.ExchangePublicKeys;
}

export interface BerytusChallengeComputeClientProofMessageRequest {
    payload: stringOrArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket
}
export interface BerytusSendComputeClientProofMessage extends BerytusChallengeComputeClientProofMessageRequest {
    challenge: BerytusSecureRemotePasswordChallengeInfo;
    name: EBerytusSecureRemotePasswordChallengeMessageName.ComputeClientProof;
}

export interface BerytusChallengeVerifyServerProofMessageRequest {
    payload: stringOrArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket
}
export interface BerytusSendVerifyServerProofMessage extends BerytusChallengeVerifyServerProofMessageRequest {
    challenge: BerytusSecureRemotePasswordChallengeInfo;
    name: EBerytusSecureRemotePasswordChallengeMessageName.VerifyServerProof;
}

export enum EBerytusSecureRemotePasswordChallengeMessageName {
    SelectSecurePassword = "SelectSecurePassword",
	ExchangePublicKeys = "ExchangePublicKeys",
	ComputeClientProof = "ComputeClientProof",
	VerifyServerProof = "VerifyServerProof"
}
export interface BerytusOffChannelOtpChallengeInfo {
    id: string;
    type: EBerytusChallengeType.OffChannelOtp;
    parameters: null;
}
export interface BerytusChallengeGetOtpMessageRequest {
    payload: string
}
export interface BerytusSendGetOtpMessage extends BerytusChallengeGetOtpMessageRequest {
    challenge: BerytusOffChannelOtpChallengeInfo;
    name: EBerytusOffChannelOtpChallengeMessageName.GetOtp;
}

export enum EBerytusOffChannelOtpChallengeMessageName {
    GetOtp = "GetOtp"
}
export type BerytusChallengeInfoUnion = BerytusIdentificationChallengeInfo
	| BerytusPasswordChallengeInfo
	| BerytusDigitalSignatureChallengeInfo
	| BerytusDigitalSignatureChallengeInfo
	| BerytusSecureRemotePasswordChallengeInfo
	| BerytusSecureRemotePasswordChallengeInfo
	| BerytusSecureRemotePasswordChallengeInfo
	| BerytusSecureRemotePasswordChallengeInfo
	| BerytusOffChannelOtpChallengeInfo;
export type BerytusSendMessageUnion = BerytusSendGetIdentityFieldsMessage
	| BerytusSendGetPasswordFieldsMessage
	| BerytusSendSelectKeyMessage
	| BerytusSendSignNonceMessage
	| BerytusSendSelectSecurePasswordMessage
	| BerytusSendExchangePublicKeysMessage
	| BerytusSendComputeClientProofMessage
	| BerytusSendVerifyServerProofMessage
	| BerytusSendGetOtpMessage;
export type BerytusReceiveMessageUnion = BerytusChallengeGetIdentityFieldsMessageResponse
	| BerytusChallengeGetPasswordFieldsMessageResponse
	| BerytusChallengeSelectKeyMessageResponse
	| BerytusChallengeSignNonceMessageResponse
	| BerytusChallengeSelectSecurePasswordMessageResponse
	| BerytusChallengeExchangePublicKeysMessageResponse
	| BerytusChallengeComputeClientProofMessageResponse
	| BerytusChallengeVerifyServerProofMessageResponse
	| BerytusChallengeGetOtpMessageResponse;
export type BerytusFieldUnion = BerytusForeignIdentityField
	| BerytusIdentityField
	| BerytusKeyField
	| BerytusPasswordField
	| BerytusSecurePasswordField
	| BerytusSharedKeyField;
export type BerytusFieldValueUnion = string | BerytusEncryptedPacket
	| BerytusKeyFieldValue
	| BerytusSecurePasswordFieldValue
	| BerytusSharedKeyFieldValue;
export type stringOrBerytusEncryptedPacket = string |
	BerytusEncryptedPacket;

export type stringOrArrayBufferOrBerytusEncryptedPacket = string |
	ArrayBuffer |
	BerytusEncryptedPacket;

export type ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket = ArrayBuffer |
	ArrayBufferView |
	BerytusEncryptedPacket;

export type stringOrArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket = string |
	ArrayBuffer |
	ArrayBufferView |
	BerytusEncryptedPacket;
