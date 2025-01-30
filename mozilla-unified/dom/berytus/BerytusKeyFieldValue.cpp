/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ErrorList.h"
#include "mozilla/Base64.h"
#include "mozilla/dom/BerytusBuffer.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h"
#include "mozilla/dom/BerytusKeyFieldValue.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary, mBuffer)
NS_IMPL_ADDREF_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusKeyFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusKeyFieldValue::BerytusKeyFieldValue(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusBuffer>& aBuffer
) : BerytusFieldValueDictionary(aGlobal),
    mBuffer(aBuffer) {}

BerytusKeyFieldValue::~BerytusKeyFieldValue() {}

JSObject*
BerytusKeyFieldValue::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyFieldValue_Binding::Wrap(aCx, this, aGivenProto);
}

BerytusFieldType BerytusKeyFieldValue::Type() {
  return BerytusFieldType::Key;
}

void BerytusKeyFieldValue::GetPublicKey(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv
) const {
  mBuffer->Get(aCx,aRetVal, aRv);
}

void BerytusKeyFieldValue::ToJSON(JSContext* aCx, JS::MutableHandle<JS::Value> aRetVal, ErrorResult& aRv) {
  mBuffer->ToJSON(aCx, aRetVal, aRv);
}

} // namespace mozilla::dom
