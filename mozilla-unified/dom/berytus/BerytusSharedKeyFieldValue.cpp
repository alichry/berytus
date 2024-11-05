/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BerytusEncryptedPacket.h"
#include "ErrorList.h"
#include "mozilla/Base64.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldValue.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary, (mAsEncrypted), (mCachedBuffer))
NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSharedKeyFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusSharedKeyFieldValue::BerytusSharedKeyFieldValue(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusEncryptedPacket>& aAsEncrypted
) : BerytusFieldValueDictionary(aGlobal),
    mAsEncrypted(aAsEncrypted),
    mAsBuffer(CryptoBuffer()) {
  MOZ_ASSERT(mAsEncrypted);
  mozilla::HoldJSObjects(this);
}

BerytusSharedKeyFieldValue::BerytusSharedKeyFieldValue(
  nsIGlobalObject* aGlobal,
  CryptoBuffer&& aAsBuffer
) : BerytusFieldValueDictionary(aGlobal),
    mAsEncrypted(nullptr),
    mAsBuffer(std::move(aAsBuffer)) {
  mozilla::HoldJSObjects(this);
}


BerytusSharedKeyFieldValue::~BerytusSharedKeyFieldValue()
{
    mozilla::DropJSObjects(this);
}

nsIGlobalObject* BerytusSharedKeyFieldValue::GetParentObject() const { return mGlobal; }

JSObject*
BerytusSharedKeyFieldValue::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSharedKeyFieldValue_Binding::Wrap(aCx, this, aGivenProto);
}

BerytusFieldType BerytusSharedKeyFieldValue::Type() {
  return BerytusFieldType::SharedKey;
}

void BerytusSharedKeyFieldValue::GetPrivateKey(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv
) {
  if (mAsEncrypted) {
    aRetVal.SetAsBerytusEncryptedPacket() = mAsEncrypted;
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

void BerytusSharedKeyFieldValue::ToJSON(JSContext* aCx, JS::MutableHandle<JS::Value> aRetVal, ErrorResult& aRv) {
  if (mAsEncrypted) {
    BerytusEncryptedPacketJSON packetJson;
    mAsEncrypted->ToJSON(packetJson, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return;
    }
    packetJson.ToObjectInternal(aCx, aRetVal);
    return;
  }
  nsAutoCString publicKeyBase64Url;
  nsresult res;
  res = Base64URLEncode(
      mAsBuffer.Length(), mAsBuffer.Elements(),
      Base64URLEncodePaddingPolicy::Omit, publicKeyBase64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aRv.Throw(res);
    return;
  }
  JSString* str = JS_NewStringCopyN(aCx, publicKeyBase64Url.Data(), publicKeyBase64Url.Length());
  if (NS_WARN_IF(!str)) {
    aRv.Throw(nsresult::NS_ERROR_OUT_OF_MEMORY);
    return;
  }
  aRetVal.setString(str);
}

already_AddRefed<BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue::Constructor(
  GlobalObject& aGlobal,
  const mozilla::dom::ArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket& aPrivateKeyValue,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (aPrivateKeyValue.IsBerytusEncryptedPacket()) {
    // nsresult rv;
    // RefPtr<BerytusEncryptedPacket> copiedPacket = aPrivateKeyValue.GetAsBerytusEncryptedPacket().Clone(&rv);
    RefPtr<BerytusSharedKeyFieldValue> value = new BerytusSharedKeyFieldValue(
      global,
      //copiedPacket
      // TODO(berytus): check if creating a pointer
      // from a reference given by NonNull<T> is a good idea.
      OwningNonNull(aPrivateKeyValue.GetAsBerytusEncryptedPacket())
    );
    return value.forget();
  }
  CryptoBuffer buffer;
  bool assigned;
  if (aPrivateKeyValue.IsArrayBuffer()) {
    assigned = buffer.Assign(aPrivateKeyValue.GetAsArrayBuffer());
  } else {
    assigned = buffer.Assign(aPrivateKeyValue.GetAsArrayBufferView());
  }
  if (NS_WARN_IF(!assigned)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    return nullptr;
  }

  RefPtr<BerytusSharedKeyFieldValue> value = new BerytusSharedKeyFieldValue(
    global,
    std::move(buffer)
  );
  return value.forget();
}

already_AddRefed<BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue::Clone(nsresult* aRv) {
  if (mAsEncrypted) {
    RefPtr<BerytusSharedKeyFieldValue> v = new BerytusSharedKeyFieldValue(
      mGlobal,
      mAsEncrypted
    );
    return v.forget();
  }
  CryptoBuffer buffer;
  if (NS_WARN_IF(!buffer.Assign(mAsBuffer))) {
    *aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  RefPtr<BerytusSharedKeyFieldValue> v = new BerytusSharedKeyFieldValue(
    mGlobal,
    std::move(buffer)
  );
  *aRv = NS_OK;
  return v.forget();
}


} // namespace mozilla::dom
