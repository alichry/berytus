/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

enum BerytusAccountStatus {
    "Pending",
    "Created",
    "Retired"
};

typedef DOMString BerytusAccountCategory;

typedef unsigned long long BerytusAccountVersion;

// dictionary BerytusFieldRejectionParameters {
//     required (BerytusField or DOMString) field;
//     required BerytusFieldRejectionReasonCode reason;
//     /**
//      * Optional - the desired value to override the field
//      *  value with.
//      *
//      *  Care must be taken here... When plaintext is passed,
//      *  some field types might only accept strings or buffers.
//      *  Consult the individual field types' constructors
//      *  to see what is allowed for the value type.
//      */
//     (BerytusDataSource or BerytusFieldValueDictionary) newValue;
// };

[SecureContext, Exposed=(Window)]
interface mixin BerytusAccountMetadata {
    /**
     * The account category. This is used to establish
     * different categories that can exist in the same
     * Web Application, e.g. an admin account and a regular
     * user account.
     *
     * Defaults to <empty string>
     */
    readonly attribute BerytusAccountCategory category;

    /**
     * The account record version. The account shema, as
     * defined by the Web Application, might change overtime.
     * Versoning the account record helps in migrating an
     * old account record to conform to the latest schema.
     *
     * Defaults to 0
     */
    readonly attribute BerytusAccountVersion version;

    /**
     * The account status reflected by the Web Application.
     * Despite offering several API calls to modify the
     * account record,
     *
     * Defaults to "Pending"
     */
    readonly attribute BerytusAccountStatus status;

    /**
     * The password change URL associated with this
     * account record. By default, no password
     * change url is set. This is useful for Password
     * Managers as we have seen it as a "quirk" in
     * https://github.com/apple/password-manager-resources
     */
    readonly attribute DOMString? changePasswordUrl;
};

[SecureContext, Exposed=(Window)]
interface mixin BerytusAccountWritableMetadata {
    [Throws]
    Promise<undefined> setCategory(BerytusAccountCategory category);

    [Throws]
    Promise<undefined> setStatus(BerytusAccountStatus status);

    [Throws]
    Promise<undefined> setVersion(BerytusAccountVersion version);

    [Throws]
    Promise<undefined> setChangePasswordUrl(DOMString url);
};

[SecureContext, Exposed=(Window)]
interface mixin BerytusAccount {
    /**
     * The registered user attriibutes with this account record.
     */
    //readonly attribute BerytusUserAttributeMap userAttributes;

    /**
     * The existing fields in the account record.
     * Additional calls to `addFields` will add
     * further entries to the fields map.
     */
    //readonly attribute BerytusFieldMap fields;

    // [Throws]
    // Promise<record<DOMString, BerytusField>> addFields(
    //     BerytusField... field
    // );
    // [Throws]
    // Promise<record<DOMString, BerytusField>> rejectAndReviseFields(
    //     BerytusFieldRejectionParameters... rejectionParameters
    // );

    /**
     * Web applications might want to set (create) an additional
     * (custom) user attribute or override a default attribute.
     * In the latter case, the web application  might require
     * the attribute value to match a specific format.
     * The primary purpose of allowing the web application
     * to store the transformed/new value of a user attribute is
     * the ability to retrieve it again when resuming the registration process.
     * The account could be in a unreserved pending verification state
     * where it is not even created in the web app but it is only
     * saved in the Secret Manager. In this case, the web app frontend
     * could just retrieve the transformed/new/default user attributes
     * and proceed with the registration.
     *
     * @param attributes A key-value dictionary where keys are
     *  user attribute ids (BerytusUserAttributeKey) and the values
     *  are the desired user attribute value.
     */
    // [Throws]
    // Promise<undefined> setUserAttributes(
    //     record<DOMString, BerytusUserAttributeDefinition> attributes
    // );

    /* --------------------------------------------------- */
    /* Extra methods (might not be implemented)            */
    /* --------------------------------------------------- */
    /**
     * Create a field category.
     * @param id The ID of the category. Must match the following RegExp:
     *  ^[a-zA-Z][a-zA-Z0-9_\-]$
     * @param info A human-readable description of the category, i.e.
     *  its purpose.
     */
    // [Throws]
    // Promise<undefined> addFieldCategory(
    //     DOMString id,
    //     DOMString info
    // );
};