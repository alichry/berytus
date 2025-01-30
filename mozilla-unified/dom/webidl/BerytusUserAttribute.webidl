/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

/**
 * Must match any of the following strings based on the standardised
 * OpenID Claims.
 *  https://openid.net/specs/openid-connect-core-1_0.html#StandardClaims
 * Valid keys:
 *  name
 *  givenName
 *  familyName
 *  middleName
 *  nickname
 *  profile
 *  picture
 *  website
 *  email
 *  gender
 *  birthdate
 *  zoneinfo
 *  locale
 *  phoneNumber
 *  address
 * Or any string matching the following RegExp:
 *  /^custom:[a-zA-Z][a-zA-Z0-9_\-]*$/
 */
typedef DOMString BerytusUserAttributeKey;

dictionary BerytusUserAttributeDefinition {
    /**
     * The user attribute key
     */
    required BerytusUserAttributeKey id;
    /**
     * A human-readable description of this attribute
     */
    DOMString info;
    /**
     * The mime type for the contents of this user attribute.
     * Defaults to text/plain.
     */
    DOMString mimeType;
    /**
     * The user attribute's value.
     */
    required (DOMString or BufferSource or BerytusEncryptedPacket) value;
};

enum BerytusUserAttributeValueEncodingType {
    "None",
    "Base64URLString",
    "EncryptedPacketJSON"
};

dictionary BerytusUserAttributeJSON {
    required BerytusUserAttributeKey id;
    DOMString info;
    DOMString mimeType;
    required BerytusUserAttributeValueEncodingType encoding;
    required (DOMString /*or Base64URLString*/ or BerytusEncryptedPacketJSON) value;
};

[SecureContext, Exposed=(Window)]
interface BerytusUserAttribute {
    readonly attribute BerytusUserAttributeKey id;
    /**
     * A user attribute can represent readable strings
     * or binary formats such as a PNG. This attribute
     * is used as an indicator for such variations of
     * user attribute mime types.
     */
    readonly attribute DOMString? mimeType;
    /**
     * Optional - A human-readable description is often helpful
     * for the user to understand that this user attribute
     * is and, perhaps, what it is used for.
     */
    readonly attribute DOMString? info;

    /**
     * The user attribute's value.
     */
    [Throws]
    readonly attribute (DOMString or ArrayBuffer or BerytusEncryptedPacket) value;

    [Throws]
    BerytusUserAttributeJSON toJSON();
};