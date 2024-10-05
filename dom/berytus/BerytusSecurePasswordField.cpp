/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSecurePasswordField.h"
#include "BerytusData.h"
#include "BerytusDataVariant.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_ADDREF_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusSecurePasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSecurePasswordField::~BerytusSecurePasswordField()
{
    // Add |MOZ_COUNT_DTOR(BerytusSecurePasswordField);| for a non-refcounted object.
}

JSObject*
BerytusSecurePasswordField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSecurePasswordField_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusSecurePasswordField::GetParentObject() const {
  return mGlobal;
}

already_AddRefed<BerytusField> BerytusSecurePasswordField::Clone()
{
  RefPtr<BerytusField> obj = new BerytusSecurePasswordField(*this);
  return obj.forget();
}

bool BerytusSecurePasswordField::ValidateValue(BerytusDataVariant* aValue)
{
  if (! aValue->IsBerytusFieldValueDictionary()) {
    return false;
  }
  return aValue->AsBerytusFieldValueDictionary()->IsForField(Type());
}

mozIAuthRealmRegistrationBasicFieldCreationOptions::FieldType BerytusSecurePasswordField::AuthRealmFieldType() const
{
  return mozIAuthRealmRegistrationBasicFieldCreationOptions::EFieldTypeSecurePassword;
}

already_AddRefed<BerytusSecurePasswordField> BerytusSecurePasswordField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusSecurePasswordFieldOptions& aOptions,
  ErrorResult& aRv
)
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> jsOptions(aGlobal.Context());
  aOptions.ToObjectInternal(aGlobal.Context(), &jsOptions);
  JS::Rooted<JSObject*> object(
    aGlobal.Context(),
    &jsOptions.toObject()
  );

  RefPtr<BerytusSecurePasswordField> obj = new BerytusSecurePasswordField(
    global,
    aId,
    BerytusFieldType::SecurePassword,
    aGlobal.Context(),
    object,
    nullptr
  );
  return obj.forget();
}


} // namespace mozilla::dom
