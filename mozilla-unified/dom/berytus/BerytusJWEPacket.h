/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSJWEPACKET_H_
#define DOM_BERYTUSJWEPACKET_H_

#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"

namespace mozilla::dom {

class BerytusJWEPacket final : public BerytusEncryptedPacket {
protected:
  BerytusJWEPacket(nsIGlobalObject* aGlobal,
                   Content&& aMaskContent,
                   Content&& aUnmaskedJweCompactContent);
  BerytusJWEPacket(nsIGlobalObject* aGlobal,
                   Content&& aExposedJweCompactContent);
  ~BerytusJWEPacket();
  Content mJweCompact;

  already_AddRefed<Blob> UnmaskImpl(ErrorResult& aRv) override;
public:
  PacketType Type() const override;
  void SerializeExposedToString(nsACString& aValue, ErrorResult& aRv) const override;

  static already_AddRefed<BerytusJWEPacket> Create(
    nsIGlobalObject* aGlobal,
    const nsAString& aJweCompact,
    const bool& aConceal,
    ErrorResult& aRv
  );
  static already_AddRefed<BerytusJWEPacket> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aJweCompact,
    ErrorResult& aRv
  );
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;
  nsIGlobalObject* GetParentObject() const;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSJWEPACKET_H_