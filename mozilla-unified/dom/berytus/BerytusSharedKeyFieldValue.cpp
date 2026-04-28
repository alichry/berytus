/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ErrorList.h"
#include "mozilla/dom/BerytusBuffer.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldValue.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary, mBuffer)
NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSharedKeyFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusSharedKeyFieldValue::BerytusSharedKeyFieldValue(
  nsIGlobalObject* aGlobal,
  RefPtr<BerytusBuffer>& aBuffer
) : BerytusFieldValueDictionary(aGlobal),
    mBuffer(aBuffer) {}

BerytusSharedKeyFieldValue::~BerytusSharedKeyFieldValue() {}

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
  mBuffer->Get(aCx,aRetVal, aRv);
}

void BerytusSharedKeyFieldValue::ToJSON(JSContext* aCx, JS::MutableHandle<JS::Value> aRetVal, ErrorResult& aRv) {
  mBuffer->ToJSON(aCx, aRetVal, aRv);
}

already_AddRefed<BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue::Constructor(
  GlobalObject& aGlobal,
  const mozilla::dom::ArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket& aPrivateKeyValue,
  ErrorResult& aRv
) {
  nsresult res;
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<BerytusBuffer> buffer;
  if (aPrivateKeyValue.IsBerytusEncryptedPacket()) {
    buffer = new BerytusBuffer(
      // TODO(berytus): check if creating a pointer
      // from a reference given by NonNull<T> is a good idea.
      OwningNonNull(aPrivateKeyValue.GetAsBerytusEncryptedPacket())
    );
  } else if (aPrivateKeyValue.IsArrayBuffer()) {
    buffer = BerytusBuffer::FromArrayBuffer(aPrivateKeyValue.GetAsArrayBuffer(), res);
    if (NS_WARN_IF(NS_FAILED(res))) {
      aRv.Throw(res);
      return nullptr;
    }
  } else {
    buffer = BerytusBuffer::FromArrayBufferView(aPrivateKeyValue.GetAsArrayBufferView(), res);
    if (NS_WARN_IF(NS_FAILED(res))) {
      aRv.Throw(res);
      return nullptr;
    }
  }
  RefPtr<BerytusSharedKeyFieldValue> value = new BerytusSharedKeyFieldValue(
    global,
    buffer
  );
  return value.forget();
}

} // namespace mozilla::dom
