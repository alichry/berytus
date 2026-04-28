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

[SecureContext, Exposed=(Window)]
interface BerytusEncryptedPacket : Blob {};

[SecureContext, Exposed=(Window)]
interface BerytusJWEPacket : BerytusEncryptedPacket {
  [Throws]
  constructor(DOMString jweCompact);
};