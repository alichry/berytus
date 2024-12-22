/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusBuffer.h"
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h"
#include "mozilla/dom/BerytusSecurePasswordFieldValue.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary, mSalt, mVerifier)
NS_IMPL_ADDREF_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSecurePasswordFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusSecurePasswordFieldValue::BerytusSecurePasswordFieldValue(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusBuffer>& aSalt,
  const RefPtr<BerytusBuffer>& aVerifier
) : BerytusFieldValueDictionary(aGlobal),
    mSalt(aSalt),
    mVerifier(aVerifier) {}

BerytusSecurePasswordFieldValue::~BerytusSecurePasswordFieldValue()
{
    // Add |MOZ_COUNT_DTOR(BerytusSecurePasswordFieldValue);| for a non-refcounted object.
}

JSObject*
BerytusSecurePasswordFieldValue::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSecurePasswordFieldValue_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusSecurePasswordFieldValue::GetParentObject() const { return mGlobal; }

BerytusFieldType BerytusSecurePasswordFieldValue::Type() {
  return BerytusFieldType::SecurePassword;
}

void BerytusSecurePasswordFieldValue::GetSalt(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv) const {
  mSalt->Get(aCx, aRetVal, aRv);
}

void BerytusSecurePasswordFieldValue::GetVerifier(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv) const {
  mVerifier->Get(aCx, aRetVal, aRv);
}

void BerytusSecurePasswordFieldValue::ToJSON(
  JSContext* aCx,
  JS::MutableHandle<JS::Value> aRetVal,
  ErrorResult& aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::Value> salt(aCx);
  JS::Rooted<JS::Value> verifier(aCx);

  mSalt->ToJSON(aCx, &salt, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  mVerifier->ToJSON(aCx, &verifier, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "salt", salt))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "verifier", verifier))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
}

} // namespace mozilla::dom
