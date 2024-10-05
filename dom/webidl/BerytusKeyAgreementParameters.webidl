/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */
 
[SecureContext, Exposed=(Window)]
interface BerytusKeyAgreementParameters {
    /**
     * Channel/session ID provided by Berytus
     */
    readonly attribute DOMString sessionId;
    readonly attribute DOMString webAppX25519Key;
    readonly attribute DOMString scmX25519Key;
    readonly attribute DOMString hkdfHash;
    readonly attribute ArrayBuffer hkdfSalt;
    readonly attribute ArrayBuffer hkdfInfo;
    /**
     * (in bits)
     */
    readonly attribute unsigned short aesKeyLength;

    /**
     * Convert this object to an ordered-JSON string.
     * This is designed for key agreement parameter signing.
     * toJSON is deterministic. hkdfSalt and hkdfInfo are encoded
     * as base64.
     * The order is as follows:
     * - sessionId
     * - webAppX25519Key
     * - scmX25519Key
     * - hkdfHash;
     * - hkdfSalt
     * - hkdfInfo
     * - aesKeyLength
     *
     * Two space characters are used as indentation.
     */
    [Throws]
    DOMString toJSON();
};