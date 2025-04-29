/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyExchangePartyPublicKeys {
    required DOMString scm;
    required DOMString webApp;
};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyExchangeAuthentication : BerytusKeyExchangePartyPublicKeys {
    required DOMString name; // Ed25519
    required BerytusKeyExchangePartyPublicKeys public;
};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyExchangeParams {
  required DOMString name; // -> X25519
  required BerytusKeyExchangePartyPublicKeys public;
};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyDerivationParams : HkdfParams {};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyGenParams : AesKeyGenParams {};

/**
 * The session fingerprint is produced by
 * Hashing(Salt . UTF8(ID) . Timestamp (as unsigned long) . Origin . DER(certificate))
 */
[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyExchangeSessionFingerprint {
    required DOMString hash; // -> SHA-256
    required DOMString version;
    /**
     * Alternatively, we can streamline a flow where the browser
     * sends the salt to a signed URL, therefore concealing the salt
     * from the client-side.
     */
    required BufferSource salt;
    required BufferSource value;
};

[GenerateInit, GenerateConversionToJS]
dictionary BerytusKeyExchangeSession {
    /** Session id, same as channel id */
    required DOMString id;
    required unsigned long timestamp;
    required BerytusKeyExchangeSessionFingerprint fingerprint;
};

[SecureContext, Exposed=(Window)]
interface BerytusKeyAgreementParameters {
    /**
     * @type {BerytusKeyExchangeSession}
     */
    [Throws]
    readonly attribute any session;
    /**
     * @type {BerytusKeyExchangeAuthentication}
     */
    [Throws]
    readonly attribute any authentication;
    /**
     * @type {BerytusKeyExchangeParams}
     */
    [Throws]
    readonly attribute any exchange;
    /**
     * @type {BerytusKeyDerivationParams}
     */
    [Throws]
    readonly attribute any derivation;
    /**
     * @type {BerytusKeyGenParams}
     */
    [Throws]
    readonly attribute any generation;

    /**
     * Convert this object to a canonical JSON value; see RFC 8785
     * @url {https://www.rfc-editor.org/rfc/rfc8785}
     */
    [Throws]
    DOMString toCanonicalJSON();
};