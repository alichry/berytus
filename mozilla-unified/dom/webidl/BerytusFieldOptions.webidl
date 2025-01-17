dictionary BerytusFieldCategoryOptions {
    /**
     * The category id  to group the field under.
     * The category id must be created before hand
     * through createFieldCategory(id, info);
     */
    required DOMString categoryId;

    /**
     * Optional - Field position within the category.
     */
    unsigned short position;
};

dictionary BerytusBaseFieldOptions {
    /**
     * Optional - The field can be grouped alongside
     * other fields in a category. If this is desired,
     * then pass a `category` option to specify
     * the category id, created using
     * Account.createFieldCategory.
     */
    BerytusFieldCategoryOptions category;
};

/* IDENTITY */
[GenerateConversionToJS]
dictionary BerytusIdentityFieldOptions : BerytusBaseFieldOptions {
    /**
     * Is this identifier supposed to be human-readable?
     */
    required boolean            humanReadable;
    /**
     * Is this identifier _discoverable_ by other users?
     */
    required boolean            private;
    required unsigned short     maxLength;
    /**
     * What are the allowed characters for this identifier?
     */
    DOMString                  allowedCharacters;
};

/**
 * Possible values:
 * EmailAddress
 * PhoneNumber
 * /^Other:[A-Za-z][A-Za-z0-9\-_]+$/
 */
typedef DOMString BerytusForeignIdentityKind;

/* FOREIGN IDENTITY borrows `private` from IDENTITY */
[GenerateConversionToJS]
dictionary BerytusForeignIdentityFieldOptions : BerytusBaseFieldOptions {
    /**
     * Is this foreign identifier _discoverable_ by other users?
     */
    required boolean                      private;
    required BerytusForeignIdentityKind   kind;
};

/* PASSWORD */
[GenerateConversionToJS]
dictionary BerytusPasswordFieldOptions : BerytusBaseFieldOptions {
    /**
     * Optional - The password composition policy.
     * Based on Check Apple's password rules:
     * https://github.com/apple/password-manager-resources/blob/main/quirks/password-rules.json
     */
    DOMString passwordRules;
};

/* CONSUMABLE PASSWORD */
[GenerateConversionToJS]
dictionary BerytusConsumablePasswordFieldOptions : BerytusPasswordFieldOptions {
    /**
     * Optional - Consumable password may be distinguishable from
     * other consumable passwords. A web application might request
     * a specific consumable password using its identifier.
     */
    DOMString passwordIdentifier;
};

/* SECURE PASSWORD */
[GenerateConversionToJS]
dictionary BerytusSecurePasswordFieldOptions : BerytusBaseFieldOptions {
    /**
     * This must be the fieldId for the desired the SRP username value.
     * The `username` must be a registered field in the Password Manager.
     * identityFieldId should point to an Identity or ForeignIdentity field.
     */
    required DOMString identityFieldId;
};

/* KEY */
[GenerateConversionToJS]
dictionary BerytusKeyFieldOptions : BerytusBaseFieldOptions {
    /**
     * COSEAlgorithmIdentifier is borrowed from WebAuthentication.webidl
     * See https://www.iana.org/assignments/cose/cose.xhtml#algorithms
     * Note: Obviously, not all algorithms are suitable.
     * The specified algorithm must be suitable for signing (and
     * potentially asymmetric encryption, maybe? might be better
     * to separate signing keys from encryption keys) of data.
     * Otherwise, it SHOULD be rejected. It is an array of
     * COSEAlgorithmIdentifier to allow the specification of
     * a signing algorithm, and an encryption algorithm.
     */
    //required sequence<COSEAlgorithmIdentifier>     alg;
    required COSEAlgorithmIdentifier     alg;
    // ^ TODO(berytus): 2/1/2024 - At the moment, only a single COSEE algorithm can be
    // specified. In the future, this should be changed to a sequence
    // (array) of algorithms, e.g. to specify an encrpytion algorithm
    // and a signing alogirthm at the same time.
};

/* SHARED KEY; i.e. a private key that is stored in the secret manager. */
[GenerateConversionToJS]
dictionary BerytusSharedKeyFieldOptions : BerytusKeyFieldOptions {};

/**
 * Custom field creation options.
 */
[GenerateConversionToJS]
dictionary BerytusCustomFieldOptions : BerytusBaseFieldOptions {
    /**
     * The mime type of this field's data. E.g.
     * text/plain
     * application/json
     */
    required DOMString mimeType;

    /**
     * A human-readable description of this custom field. E.g.,
     * its purpose.
     */
    required DOMString info;

    /**
     * Optional - Custom fields might require additional
     * options that we cannot standardise. The parameters object
     * will be passed to the Secret Manager. it must not contain
     * any sensitive information. This can be retrieved during
     * authentication.
     */
    record<DOMString, (DOMString or long long or boolean or BufferSource)> parameters;
};