/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusJWEPacket.h"
#include <cstdint>
#include "BerytusEncryptedPacket.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "nsStringFwd.h"
#include "mozilla/dom/MemoryBlobImpl.h"

namespace mozilla::dom {

BerytusJWEPacket::BerytusJWEPacket(
  nsIGlobalObject* aGlobal,
  Content&& aMaskContent,
  Content&& aUnmaskedJweCompactContent) :
      BerytusEncryptedPacket(aGlobal,
        std::move(aMaskContent),
        true),
      mJweCompact(std::move(aUnmaskedJweCompactContent)) {
}

BerytusJWEPacket::BerytusJWEPacket(
    nsIGlobalObject* aGlobal,
    Content&& aExposedJweCompactContent) :
        BerytusEncryptedPacket(aGlobal,
                               std::move(aExposedJweCompactContent),
                               false),
        mJweCompact() {}

BerytusJWEPacket::~BerytusJWEPacket() {}

/**
 * TODO(berytus): We need to validate aJweCompact
 */
already_AddRefed<BerytusJWEPacket> BerytusJWEPacket::Create(
  nsIGlobalObject* aGlobal,
  const nsAString& aJweCompact,
  const bool& aConceal,
  ErrorResult& aRv
) {
  Content unmaskedContent;
  auto utf8Str = NS_ConvertUTF16toUTF8(aJweCompact);
  unmaskedContent.mBuf.reset((uint8_t *) malloc(utf8Str.Length()));
  if (NS_WARN_IF(!unmaskedContent.mBuf)) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  memcpy(unmaskedContent.mBuf.get(), utf8Str.BeginReading(), utf8Str.Length());
  unmaskedContent.mLen = utf8Str.Length();
  if (!aConceal) {
    return do_AddRef(new BerytusJWEPacket(
      aGlobal,
      std::move(unmaskedContent)));
  }
  Content maskContent;
  if (NS_WARN_IF(!CreateMaskContent(maskContent))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  return do_AddRef(new BerytusJWEPacket(
    aGlobal,
    std::move(maskContent),
    std::move(unmaskedContent)));
}

already_AddRefed<BerytusJWEPacket> BerytusJWEPacket::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aJweCompact,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return Create(global,
                aJweCompact,
                false,
                aRv);
}

void BerytusJWEPacket::SerializeExposedToString(nsACString& aValue, ErrorResult& aRv) const {
  const auto exposed = Exposed();
  if (NS_WARN_IF(!aValue.Assign((char*) exposed.data(), exposed.Length(), fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return;
  }
}

already_AddRefed<Blob> BerytusJWEPacket::UnmaskImpl(ErrorResult& aRv) {
  UniquePtr<uint8_t> clone((uint8_t*) malloc(mJweCompact.mLen));
  if (NS_WARN_IF(!clone)) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  memcpy(clone.get(), mJweCompact.mBuf.get(), mJweCompact.mLen);
  return do_AddRef(Blob::Create(mGlobal, MemoryBlobImpl::CreateWithLastModifiedNow(
        clone.release(),
        mJweCompact.mLen,
        u"BerytusJWEPacket"_ns,
        u"application/jose"_ns,
        RTPCallerType::Normal).take()));
}

JSObject*
BerytusJWEPacket::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusJWEPacket_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusJWEPacket::GetParentObject() const { return mGlobal; }

BerytusEncryptedPacket::PacketType BerytusJWEPacket::Type() const { return JWE; }

} // namespace mozilla::dom