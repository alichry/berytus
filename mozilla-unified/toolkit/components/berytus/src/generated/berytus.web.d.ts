type BufferSource = ArrayBuffer | ArrayBufferView;
type Base64URLString = string;
type KeyType = string;
type KeyUsage = string;
type NamedCurve = string;
type BigInteger = Uint8Array;
interface Algorithm {
    name: string;
}
interface AesCbcParams extends Algorithm {
    iv: BufferSource;
}
interface AesCtrParams extends Algorithm {
    counter: BufferSource;
    length: number;
}
interface AesGcmParams extends Algorithm {
    iv: BufferSource;
    additionalData?: BufferSource;
    tagLength?: number;
}
interface HmacImportParams extends Algorithm {
    hash: AlgorithmIdentifier;
}
interface Pbkdf2Params extends Algorithm {
    salt: BufferSource;
    iterations: number;
    hash: AlgorithmIdentifier;
}
interface RsaHashedImportParams {
    hash: AlgorithmIdentifier;
}
interface AesKeyGenParams extends Algorithm {
    length: number;
}
interface HmacKeyGenParams extends Algorithm {
    hash: AlgorithmIdentifier;
    length?: number;
}
interface RsaHashedKeyGenParams extends Algorithm {
    modulusLength: number;
    publicExponent: BigInteger;
    hash: AlgorithmIdentifier;
}
interface RsaOaepParams extends Algorithm {
    label?: BufferSource;
}
interface RsaPssParams extends Algorithm {
    saltLength: number;
}
interface EcKeyGenParams extends Algorithm {
    namedCurve: NamedCurve;
}
interface AesDerivedKeyParams extends Algorithm {
    length: number;
}
interface HmacDerivedKeyParams extends HmacImportParams {
    length?: number;
}
interface EcdhKeyDeriveParams extends Algorithm {
    public: CryptoKey;
}
interface DhImportKeyParams extends Algorithm {
    prime: BigInteger;
    generator: BigInteger;
}
interface EcdsaParams extends Algorithm {
    hash: AlgorithmIdentifier;
}
interface EcKeyImportParams extends Algorithm {
    namedCurve?: NamedCurve;
}
interface HkdfParams extends Algorithm {
    hash: AlgorithmIdentifier;
    salt: BufferSource;
    info: BufferSource;
}
interface RsaOtherPrimesInfo {
    r: string;
    d: string;
    t: string;
}
interface JsonWebKey {
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
interface CryptoKey {
    readonly type: KeyType;
    readonly extractable: boolean;
    readonly algorithm: any;
    readonly usages: Array<KeyUsage>;
}
interface CryptoKeyPair {
    publicKey: CryptoKey;
    privateKey: CryptoKey;
}
type KeyFormat = string;
type AlgorithmIdentifier = any | string;
interface SubtleCrypto {
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
    readonly parameters: AesGcmParams;
    readonly ciphertext: ArrayBuffer;
}
type BerytusFieldType = "Identity" | "ForeignIdentity" | "Password" | "SecurePassword" | "ConsumablePassword" | "Key" | "SharedKey" | "Custom";
type BerytusFieldRejectionReasonCode = string;
type BerytusFieldId = string;
type BerytusFieldValue = BerytusDataType | BerytusFieldValueDictionary;
interface BerytusField {
    readonly id: BerytusFieldId;
    readonly type: BerytusFieldType;
    readonly options: any;
    readonly value: BerytusFieldValue | null;
}
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
    type: EBerytusFieldType.ForeignIdentity;
    options: BerytusForeignIdentityFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
interface BerytusIdentityField extends BerytusField {
    type: EBerytusFieldType.Identity;
    options: BerytusIdentityFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
interface BerytusKeyAgreementParameters {
    readonly sessionId: string;
    readonly webAppX25519Key: string;
    readonly scmX25519Key: string;
    readonly hkdfHash: string;
    readonly hkdfSalt: ArrayBuffer;
    readonly hkdfInfo: ArrayBuffer;
    readonly aesKeyLength: number;
}
interface BerytusKeyFieldValue extends BerytusFieldValueDictionary {
    readonly publicKey: ArrayBufferOrBerytusEncryptedPacket;
}
interface BerytusKeyField extends BerytusField {
    type: EBerytusFieldType.Key;
    options: BerytusKeyFieldOptions;
    value: BerytusKeyFieldValue;
}
interface BerytusPasswordField extends BerytusField {
    type: EBerytusFieldType.Password;
    options: BerytusPasswordFieldOptions;
    value: nullOrstringOrBerytusEncryptedPacket;
}
interface BerytusSecurePasswordFieldValue extends BerytusFieldValueDictionary {
    readonly salt: ArrayBufferOrBerytusEncryptedPacket;
    readonly verifier: ArrayBufferOrBerytusEncryptedPacket;
}
interface BerytusSecurePasswordField extends BerytusField {
    type: EBerytusFieldType.SecurePassword;
    options: BerytusSecurePasswordFieldOptions;
    value: BerytusSecurePasswordFieldValue;
}
interface BerytusSharedKeyFieldValue extends BerytusFieldValueDictionary {
    readonly privateKey: ArrayBufferOrBerytusEncryptedPacket;
}
interface BerytusSharedKeyField extends BerytusField {
    type: EBerytusFieldType.SharedKey;
    options: BerytusSharedKeyFieldOptions;
    value: BerytusSharedKeyFieldValue;
}
type BerytusUserAttributeKey = string;
interface BerytusUserAttributeDefinition {
    id: BerytusUserAttributeKey;
    info?: string;
    mimeType?: string;
    value: string | BufferSource | BerytusEncryptedPacket;
}
interface BerytusUserAttributeJSON {
    id: BerytusUserAttributeKey;
    info?: string;
    mimeType?: string;
    encodingType?: string;
    value: string | BerytusEncryptedPacketJSON;
}
interface BerytusUserAttribute {
    readonly id: BerytusUserAttributeKey;
    readonly mimeType: string | null;
    readonly info: string | null;
    readonly value: string | ArrayBuffer | BerytusEncryptedPacket;
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
export type BerytusFieldUnion = BerytusForeignIdentityField
	| BerytusIdentityField
	| BerytusKeyField
	| BerytusPasswordField
	| BerytusSecurePasswordField
	| BerytusSharedKeyField;