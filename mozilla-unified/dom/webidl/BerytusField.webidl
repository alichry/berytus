enum BerytusFieldType {
  "Identity",
  "ForeignIdentity",
  "Password",
  "SecurePassword",
  "ConsumablePassword",
  /**
   * The "Key" field is a public/private key pair.
   * The Secret Manager only returns the public key
   * material of the created key. To ensure the private
   * key is not or has not been, by any chance, accessed
   * by the Web Application. The "Key" field value (the private key)
   * cannot be specified by the Web Application during
   * field creation. Therefore, the Secret Manager has to
   * produce a private key during field creation.
   *
   * In addition to the isolation of the private key,
   * the Secret Manager could act as a decryption oracle
   * for the Web Application to leverage, if needed.
   */
  "Key",
  /**
   * The "SharedKey" field is a public/private key pair.
   * It is shared in the sense that both the Web Application
   * and the Secret Manager, at some point, read the
   * private key material of the produced key. This is
   * important in instances where the Web Application
   * must produce the key instead of the the Secret Manager
   * or when the Web Application must hold a copy of the
   * produced private key. This is different than the "Key"
   * field where the Secret Manager must produce the
   * key and isolate the private key.
   *
   * The private key material can be retrieved by the
   * Web Application later on.
   */
  "SharedKey",
  "Custom"
};

/**
 * Possible values:
 *  Identity:IdentityAlreadyExists
 *  ForeignIdentity:IdentityAlreadyExists
 *  Password:IncompatiblePassword
 *  Key:PublicKeyAlreadyExists
 * Or, any string that matches the below RegExp:
 *  /^Other:[A-Za-z][A-Za-z0-9\-_]*$/
 * // TODO(berytus): Define enum in types.ts
 */
typedef DOMString BerytusFieldRejectionReasonCode;

/**
 * Must match the following RegExp: ^[a-zA-Z][a-zA-Z0-9_\-]$
 */
typedef DOMString BerytusFieldId;

typedef (DOMString or BerytusEncryptedPacket or BerytusFieldValueDictionary) BerytusFieldValue;

/**
 * Base interface for Berytus Fields. Each child of this
 * interface should define a constructor.
 */
[SecureContext, Exposed=(Window)]
interface BerytusField {

    /**
     * A unique, account structure-specific, identifier for this field.
     */
    readonly attribute BerytusFieldId id;
    readonly attribute BerytusFieldType type;
    [Throws]
    readonly attribute object options;
    [Throws]
    readonly attribute BerytusFieldValue? value;

    [Throws]
    object toJSON();
};