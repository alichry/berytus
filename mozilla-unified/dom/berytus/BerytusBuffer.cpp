/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusBuffer.h"

#include "mozilla/dom/BerytusEncryptedPacketBinding.h" // BerytusEncryptedPacketJSON
#include "mozilla/Base64.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_WITH_JS_MEMBERS(BerytusBuffer, (mAsPacket),
                                         (mCachedBuffer))
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusBuffer)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusBuffer)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusBuffer)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusBuffer::BerytusBuffer(CryptoBuffer&& aBuffer)
    : mAsPacket(nullptr),
      mAsBuffer(std::move(aBuffer)),
      mCachedBuffer(nullptr) {
  mozilla::HoldJSObjects(this);
}
BerytusBuffer::BerytusBuffer(
  const RefPtr<BerytusEncryptedPacket>& aPacket
) : mAsPacket(aPacket),
    mAsBuffer(),
    mCachedBuffer(nullptr) {
  mozilla::HoldJSObjects(this);
}

BerytusBuffer::~BerytusBuffer() { mozilla::DropJSObjects(this); };

already_AddRefed<BerytusBuffer> BerytusBuffer::FromArrayBuffer(
  const ArrayBuffer& aValue,
  nsresult& aRv
) {
  CryptoBuffer buf;
  if (buf.Assign(aValue)) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  RefPtr<BerytusBuffer> res = new BerytusBuffer(std::move(buf));
  aRv = NS_OK;
  return res.forget();
}
already_AddRefed<BerytusBuffer> BerytusBuffer::FromArrayBufferView(
  const ArrayBufferView& aValue,
  nsresult& aRv
) {
  CryptoBuffer buf;
  if (buf.Assign(aValue)) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  RefPtr<BerytusBuffer> res = new BerytusBuffer(std::move(buf));
  aRv = NS_OK;
  return res.forget();
}

void BerytusBuffer::Get(JSContext* aCx,
                        OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
                        ErrorResult& aRv) {
  if (mAsPacket) {
    aRetVal.SetAsBerytusEncryptedPacket() = mAsPacket;
    return;
  }
  if (!mCachedBuffer) {
    mCachedBuffer = mAsBuffer.ToArrayBuffer(aCx, aRv);
    if (aRv.Failed()) {
      return;
    }
  }
  if (NS_WARN_IF(!aRetVal.SetAsArrayBuffer().Init(mCachedBuffer))) {
    aRv.ThrowInvalidStateError("Unable to init ArrayBuffer");
    return;
  }
}

void BerytusBuffer::ToJSON(JSContext* aCx,
                           JS::MutableHandle<JS::Value> aRetVal,
                           ErrorResult& aRv) {
  if (mAsPacket) {
    BerytusEncryptedPacketJSON packetJson;
    mAsPacket->ToJSON(packetJson, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return;
    }
    packetJson.ToObjectInternal(aCx, aRetVal);
    return;
  }
  nsAutoCString base64Url;
  nsresult res;
  res = Base64URLEncode(
      mAsBuffer.Length(), mAsBuffer.Elements(),
      Base64URLEncodePaddingPolicy::Omit, base64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aRv.Throw(res);
    return;
  }
  JSString* str = JS_NewStringCopyN(aCx, base64Url.Data(), base64Url.Length());
  if (NS_WARN_IF(!str)) {
    aRv.Throw(nsresult::NS_ERROR_OUT_OF_MEMORY);
    return;
  }
  aRetVal.setString(str);
}

} // namespace mozilla::dom