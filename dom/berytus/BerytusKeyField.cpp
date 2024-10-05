/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusKeyField.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_ADDREF_INHERITED(BerytusKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusKeyField::~BerytusKeyField()
{
    // Add |MOZ_COUNT_DTOR(BerytusKeyField);| for a non-refcounted object.
}

JSObject*
BerytusKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusField> BerytusKeyField::Clone()
{
  RefPtr<BerytusField> obj = new BerytusKeyField(*this);
  return obj.forget();
}

mozIAuthRealmRegistrationBasicFieldCreationOptions::FieldType BerytusKeyField::AuthRealmFieldType() const
{
  return mozIAuthRealmRegistrationBasicFieldCreationOptions::EFieldTypeKey;
}

bool BerytusKeyField::ValidateValue(BerytusDataVariant* aValue)
{
  if (! aValue->IsBerytusFieldValueDictionary()) {
    return false;
  }
  return aValue->AsBerytusFieldValueDictionary()->IsForField(Type());
}

already_AddRefed<BerytusKeyField> BerytusKeyField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusKeyFieldOptions& aOptions,
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

  RefPtr<BerytusKeyField> obj = new BerytusKeyField(
    global,
    aId,
    BerytusFieldType::Key,
    aGlobal.Context(),
    object,
    nullptr
  );

  return obj.forget();
}

} // namespace mozilla::dom
