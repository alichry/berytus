/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ErrorList.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldValue.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSharedKeyFieldValue,BerytusFieldValueDictionary, mPrivateKey)
NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSharedKeyFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusSharedKeyFieldValue::BerytusSharedKeyFieldValue(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusDataVariant>& aPrivateKey
) : BerytusFieldValueDictionary(aGlobal), mPrivateKey(aPrivateKey)
{
    // Add |MOZ_COUNT_CTOR(BerytusSharedKeyFieldValue);| for a non-refcounted object.
}

BerytusSharedKeyFieldValue::BerytusSharedKeyFieldValue(
  const BerytusSharedKeyFieldValue& aOther
) : BerytusSharedKeyFieldValue(aOther.mGlobal, aOther.mPrivateKey)
{

}

BerytusSharedKeyFieldValue::~BerytusSharedKeyFieldValue()
{
    // Add |MOZ_COUNT_DTOR(BerytusSharedKeyFieldValue);| for a non-refcounted object.
}

JSObject*
BerytusSharedKeyFieldValue::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSharedKeyFieldValue_Binding::Wrap(aCx, this, aGivenProto);
}

mozIAuthRealmFieldValue* BerytusSharedKeyFieldValue::ToAuthRealmFieldValue(nsresult* aOutRes)
{
  printf("######### FUNCBRTTODO: BerytusSharedKeyFieldValue ToAuthRealmFieldValue ########\n");
  mozIAuthRealmFieldValue* res = mPrivateKey->ToAuthRealmFieldValue(aOutRes);
  printf("######### FUNCBRTTODO: BerytusSharedKeyFieldValue Return ToAuthRealmFieldValue ########\n");
  return res;
}

BerytusFieldType BerytusSharedKeyFieldValue::Type() {
  return BerytusFieldType::SharedKey;
}

already_AddRefed<BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue::Constructor(
  const GlobalObject& aGlobal,
  const ArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket& aPrivateKey,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<BerytusDataVariant> privateKey;
  if (aPrivateKey.IsArrayBuffer()) {
    privateKey = new BerytusDataVariant(
      aGlobal.Context(),
      aPrivateKey.GetAsArrayBuffer(),
      aRv
    );
    if (aRv.Failed()) {
      return nullptr;
    }
  } else if (aPrivateKey.IsArrayBufferView()) {
    privateKey = new BerytusDataVariant(
      aGlobal.Context(),
      aPrivateKey.GetAsArrayBufferView(),
      aRv
    );
    if (aRv.Failed()) {
      return nullptr;
    }
  } else {
    // another mem copy...
    RefPtr<BerytusEncryptedPacket> copiedPacket = new BerytusEncryptedPacket(aPrivateKey.GetAsBerytusEncryptedPacket());
    privateKey = new BerytusDataVariant(copiedPacket);
  }

  RefPtr<BerytusSharedKeyFieldValue> obj = new BerytusSharedKeyFieldValue(
    global,
    privateKey
  );
  return obj.forget();
}

void BerytusSharedKeyFieldValue::GetPrivateKey(
  JSContext* aCx,
  OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
  ErrorResult& aRv
) const {
  nsresult res = mPrivateKey->GetValue(aCx, aRetVal);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aRv.ThrowOperationError("Unable to get public key.");
  }
}

already_AddRefed<BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue::Create(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusDataVariant>& aPrivateKey,
  nsresult* aOutRes
) {
  if (! aPrivateKey->IsArrayBuffer() && ! aPrivateKey->IsBerytusEncryptedPacket()) {
    *aOutRes = NS_ERROR_UNEXPECTED;
    return nullptr;
  }

  RefPtr<BerytusSharedKeyFieldValue> obj = new BerytusSharedKeyFieldValue(aGlobal, aPrivateKey);
  return obj.forget();
}

} // namespace mozilla::dom
