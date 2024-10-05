/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h"
#include "mozilla/dom/BerytusSecurePasswordFieldValue.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSecurePasswordFieldValue,
// # BerytusFieldValueDictionary, your, members, here)"
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary, mSalt, mVerifier)
NS_IMPL_ADDREF_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)
NS_IMPL_RELEASE_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSecurePasswordFieldValue)
NS_INTERFACE_MAP_END_INHERITING(BerytusFieldValueDictionary)

BerytusSecurePasswordFieldValue::BerytusSecurePasswordFieldValue(
  nsIGlobalObject* aGlobal,
  const nsAString& aSaltBase64,
  const nsAString& aVerifierBase64
) : BerytusFieldValueDictionary(aGlobal),
  mSalt(new BerytusDataVariant(aSaltBase64)),
  mVerifier(new BerytusDataVariant(aVerifierBase64))
{
    // Add |MOZ_COUNT_CTOR(BerytusSecurePasswordFieldValue);| for a non-refcounted object.
}

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

mozIAuthRealmFieldValue* BerytusSecurePasswordFieldValue::ToAuthRealmFieldValue(nsresult* aOutRes)
{
  return nullptr;
}

void BerytusSecurePasswordFieldValue::GetSalt(OwningStringOrBerytusEncryptedPacket& aRetVal) const
{
  nsresult res = mSalt->GetValue(aRetVal);
  // BRTTODO: should be infallible...
  MOZ_ASSERT(!NS_FAILED(res), "BerytusDataVariant GetValue failed");
}

void BerytusSecurePasswordFieldValue::GetVerifier(OwningStringOrBerytusEncryptedPacket& aRetVal) const
{
  nsresult res = mVerifier->GetValue(aRetVal);
  // BRTTODO: should be infallible...
  MOZ_ASSERT(!NS_FAILED(res), "BerytusDataVariant GetValue failed");
}

} // namespace mozilla::dom
