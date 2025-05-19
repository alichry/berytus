/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

[GenerateConversionToJS, GenerateInit]
dictionary BerytusAccountConstraints {
    /**
     * The authenticator-defined category id.
     * The value could represent a specific
     * user type such as admin, teacher, etc.
     * This would be used to narrow down the list
     * of existing accounts in the Secret Manager.
     */
    BerytusAccountCategory category;
    /**
     * Authenticators might evolve their account-credentials
     * schema. Specifying a desired schema version
     * could be worthwile...
     */
    BerytusAccountVersion schemaVersion;
    /**
     * The key-value dictionary of (foreign) identity field id
     * mapping to its value. This is also used to narrow
     * down the list of existing accounts further to match
     * the expected account by  the authenticator.
     *
     * This is ideal when the authenticator knows
     * which account it was to retrieve without
     * confusing the user to select one.
     *
     * If this is specified, and `onboard` is called,
     * only authentication could be undertaken since
     * the channel is now bound to a specific account.
     * I.e. the intent is fixed to 'Authenticate' by
     * default.
     */
    record<DOMString, DOMString> identity;
};

[GenerateConversionToJS, GenerateInit]
dictionary BerytusChannelConstraints {
    /**
     * Optional - The desired Secret Manager to interact
     * with. The public key serves as a unique
     * identifier. If omitted, any Secret Manager
     * could be elected. If specified along with
     * 'account', then the account constraints
     * will be applied to the desired Secret Manager
     * instead of all installed Secret Managers.
     * More than one public key could be specified
     * to allow a list of one-or-more accepted
     * Secret Managers by the Web Application.
     */
    sequence<DOMString> secretManagerPublicKey;

    /**
     * Optional - The authenticator (web app) has the chance
     * to narrow down the list of accounts to
     * match account-related constraints such
     * as identity and foreign identity field values,
     * the authenticator-defined category (admin, user)
     * and the authenticator account record schema version.
     */
    BerytusAccountConstraints account;

    /**
     * Whether the web application would like to enable
     * end-to-end encryption. If this is true, the passed
     * web app actor must be a CryptoActor. Otherwise,
     * an error will be thrown.
     */
    boolean enableEndToEndEncryption;
};

[GenerateConversionToJS]
dictionary BerytusChannelOptions {
    /**
     * The Web app-side Actor for the Channel.
     * A Crypto Actor must be passed if end-to-end
     * encryption or Secure Credential Mapping is desired.
     */
    required BerytusWebAppActor webApp;

    /**
     * Optional - channcel constraints, e.g. which
     *  Secret Manager is accepted. An account
     *  classifier (email, username, category, etc.)
     *  can be specified to make sure that the Secret Manager
     *  has such account(s). Filtering against an account
     *  is desirable when the web application wants to
     *  update the credentials of that account, for examople.
     *  When it comes to onboarding, specifying an account
     *  filter is not necessary. Alternatively, the web
     *  app can specify an account category to filter against,
     *  e.g. "admin"
     */
    BerytusChannelConstraints constraints;
};

enum BerytusOnboardingIntent {
    "Unknown",
    "Authenticate",
    "Register"
};

[GenerateConversionToJS]
dictionary BerytusOnboardingOptions {
    /**
     * Optional - The user's intent for this onboarding operation;
     *  to login with an existing account or create a new one.
     *  The web app can use its own UI to query the
     *  user for his intent. Accordingly, this propery could
     *  reflect the retrieved user intent. If unspecified,
     *  it defaults to "Unknown" and the secret manager
     *  will query the user for his intent using its UI.
     *  However, if the channel creation options included
     *  account constraints specifying identity fields,
     *  the intent is fixed to "Authenticate", and if
     *  otherwise is specified, an error will be thrown.
     *
     *  Note - The web application might enforce
     *  an intent without consulting the user if necessary.
     *  For example, if the web application is not
     *  accepting any new users, then "Authenticate"
     *  can be specified to only allow authentication.
     *
     *  Note - When the intent is fixed to 'Authenticate',
     *  the selected Secret Manager must have at least
     *  one existing account in its database. Therefore,
     *  it is impossible to select a Secret Manager
     *  without an existing acocunt when intent is 'Authenticate'.
     */
    BerytusOnboardingIntent intent;

    /**
     * Optional - Desired user attributes to retrieve from the
     *  Secret Manager when undertaking an account creation operation.
     *  If the login operation results in an account authentication
     *  operation, this attribute will have no impact. When the
     *  operation is returned, and is an instance of
     *  BerytusAccountCreationOperation, the `userAttributes` property
     *  will be populated according to the required user attributes
     *  specified here.
     *
     *  A dictionay where the keys should match a subset of
     *  the OpenID standard claims or a string with a prefix of of `custom:`
     *  See the typedef BerytusUserAttributeKey for exact RegExp for the
     *  custom user attribute keys. If the key value is `true`, then
     *  the web application expects the Secret Manager to return a non-empty
     *  value. If the Secret Manager was not unable to provide a non-empty
     *  value, an error will be thrown. To retrieve potentially empty
     *  user attributes, specify a value of `false`. We recommend
     *  that `true` be set only when the Web Application cannot resort
     *  to leveraging its own UI for querying the user's input for
     *  the needed user attributes (identity information).
     */
    record<DOMString, boolean> requiredUserAttributes;
};

dictionary BerytusKeyAgreementInput {
    /**
     * Base64 string of the DER-structured X25519 SPKI
     * This is the web app X25519 public key
     */
    required DOMString public;
    /**
     * A list of URLs for which ciphertext would be transmitted to.
     * If empty, all masked ciphertext would be unmasked in any
     * fetch request.
     */
    sequence<DOMString> unmaskAllowlist;
};

[SecureContext, Exposed=(Window)]
interface BerytusChannel {
    readonly attribute boolean active;

    readonly attribute BerytusWebAppActor webApp;
    /**
     * This is the selected Secret Manager.
     *  Null if enableEndToEndEncryption was set to false.
     * BRTTODO: 21/1/2024 -- It is better to have an AnonymousSecretManagerActor
     * instead when enableEndToEncryption was set to false.
     */
    readonly attribute BerytusSecretManagerActor? secretManager;
    /**
     * Key agreement parameters.
     *  Null if enableEndToEncryption was set to false.
     */
    readonly attribute BerytusKeyAgreementParameters? keyAgreementParams;

    /**
     * @type {BerytusChannelConstraints}
     */
    [Throws]
    readonly attribute object? constraints;

    /**
     * Establish a new channel. Depending on the options,
     * a Secret Manager will be elected for this channel.
     * If no suitable Secret Manager was found, or if the
     * user aborted the operation, an error will be thrown.
     */
    [Throws]
    static Promise<BerytusChannel> create(
        optional BerytusChannelOptions options = {}
    );

    [Throws]
    Promise<undefined> close();

    [Throws]
    Promise<(BerytusAccountAuthenticationOperation or BerytusAccountCreationOperation)> login(
        optional BerytusOnboardingOptions options = {}
    );

    [Throws]
    Promise<BerytusKeyAgreementParameters> prepareKeyAgreementParameters(
        BerytusKeyAgreementInput input
    );
    [Throws]
    Promise<ArrayBuffer> exchangeKeyAgreementSignatures(
        ArrayBuffer webAppSignature
    );
    [Throws]
    Promise<undefined> enableEndToEndEncryption();
};