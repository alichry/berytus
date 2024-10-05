/* -*- Mode: IDL; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

typedef DOMString BerytusPlaintextStringSource;

typedef BufferSource BerytusPlaintextBufferSource;

typedef (BerytusPlaintextStringSource or BerytusPlaintextBufferSource) BerytusPlaintextSource;

typedef BerytusEncryptedPacket BerytusCiphertextSource;

typedef (BerytusPlaintextSource or BerytusCiphertextSource) BerytusDataSource;

typedef (DOMString or ArrayBuffer or BerytusEncryptedPacket) BerytusDataType;

/**
 * See childs of Algorithm in SubtleCrypto.webidl; see
 * https://developer.mozilla.org/en-US/docs/Web/API/SubtleCrypto/encrypt#supported_algorithms
 */
typedef AesGcmParams BerytusEncryptionParams;

[GenerateConversionToJS]
dictionary AesGcmParamsJSON : Algorithm {
  required Base64URLString iv;
  Base64URLString additionalData;
  [EnforceRange] octet tagLength;
};

typedef AesGcmParamsJSON BerytusEncryptionParamsJSON;

[GenerateConversionToJS]
dictionary BerytusEncryptedPacketJSON {
    required BerytusEncryptionParamsJSON parameters;
    required Base64URLString ciphertext;
};

[SecureContext, Exposed=(Window)]
interface BerytusEncryptedPacket {
    /**
     * @type {BerytusEncryptionParams}
     */
    [Throws]
    readonly attribute object parameters;

    [SameObject, Throws]
    readonly attribute ArrayBuffer ciphertext;

    [Throws]
    constructor(BerytusEncryptionParams algorithm, BufferSource ciphertext);

    /**
     * Return a serialisable JavaScript object that can be
     * sent over HTTP. Mirrors `this` interface; however,
     * ArrayBuffers are transformed into Base64 strings.
     */
    [Throws]
    BerytusEncryptedPacketJSON toJSON();
};