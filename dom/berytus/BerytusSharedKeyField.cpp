/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSharedKeyField.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusSharedKeyField,
// # BerytusField, your, members, here)"
NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusSharedKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSharedKeyField::~BerytusSharedKeyField() = default;

JSObject*
BerytusSharedKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSharedKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusField> BerytusSharedKeyField::Clone()
{
  RefPtr<BerytusField> obj = new BerytusSharedKeyField(*this);
  return obj.forget();
}

bool BerytusSharedKeyField::ValidateValue(BerytusDataVariant* aValue)
{
  if (! aValue->IsBerytusFieldValueDictionary()) {
    return false;
  }
  return aValue->AsBerytusFieldValueDictionary()->IsForField(Type());
}

mozIAuthRealmRegistrationBasicFieldCreationOptions::FieldType BerytusSharedKeyField::AuthRealmFieldType() const
{
  return mozIAuthRealmRegistrationBasicFieldCreationOptions::EFieldTypeKey;
}

already_AddRefed<BerytusSharedKeyField> BerytusSharedKeyField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusSharedKeyFieldOptions& aOptions,
  const Optional<NonNull<BerytusSharedKeyFieldValue>>& aDesiredValue,
  ErrorResult& aRv
)
{
  nsresult res;
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

  RefPtr<BerytusDataVariant> fieldValueVariant = nullptr;
  if (aDesiredValue.WasPassed()) {
    RefPtr<BerytusSharedKeyFieldValue> sharedKeyFv = new BerytusSharedKeyFieldValue(aDesiredValue.Value());
    fieldValueVariant = new BerytusDataVariant(sharedKeyFv);
  }

  RefPtr<BerytusSharedKeyField> obj = new BerytusSharedKeyField(
    global,
    aId,
    BerytusFieldType::SharedKey,
    aGlobal.Context(),
    object,
    fieldValueVariant
  );
  return obj.forget();
}


} // namespace mozilla::dom