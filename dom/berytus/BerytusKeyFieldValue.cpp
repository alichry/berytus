/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ErrorList.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h"
#include "mozilla/dom/BerytusKeyFieldValue.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusKeyFieldValue,BerytusFieldValueDictionary, mPublicKey)
NS_IMPL_ADDREF_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusKeyFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusKeyFieldValue::BerytusKeyFieldValue(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusDataVariant>& aPublicKey
) : BerytusFieldValueDictionary(aGlobal), mPublicKey(aPublicKey)
{
    // Add |MOZ_COUNT_CTOR(BerytusKeyFieldValue);| for a non-refcounted object.
}

BerytusKeyFieldValue::~BerytusKeyFieldValue()
{
    // Add |MOZ_COUNT_DTOR(BerytusKeyFieldValue);| for a non-refcounted object.
}

JSObject*
BerytusKeyFieldValue::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyFieldValue_Binding::Wrap(aCx, this, aGivenProto);
}

BerytusFieldType BerytusKeyFieldValue::Type() {
  return BerytusFieldType::Key;
}

mozIAuthRealmFieldValue* BerytusKeyFieldValue::ToAuthRealmFieldValue(nsresult* aOutRes)
{
  printf("######### FUNCBRTTODO: BerytusKeyFieldValue ToAuthRealmFieldValue ########\n");
  return nullptr;
  // This is not needed...
}

void BerytusKeyFieldValue::GetPublicKey(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv
) const {
  nsresult res = mPublicKey->GetValue(aCx, aRetVal);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aRv.ThrowOperationError("Unable to get public key.");
  }
}

already_AddRefed<BerytusKeyFieldValue> BerytusKeyFieldValue::Create(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusDataVariant>& aPublicKey,
  nsresult* aOutRes
) {
  if (! aPublicKey->IsArrayBuffer() && ! aPublicKey->IsBerytusEncryptedPacket()) {
    *aOutRes = NS_ERROR_UNEXPECTED;
    return nullptr;
  }

  RefPtr<BerytusKeyFieldValue> obj = new BerytusKeyFieldValue(aGlobal, aPublicKey);
  return obj.forget();
}

} // namespace mozilla::dom
