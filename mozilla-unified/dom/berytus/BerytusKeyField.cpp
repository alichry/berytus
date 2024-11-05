/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusKeyField.h"
#include "BerytusFieldValueDictionary.h"
#include "js/ArrayBuffer.h"
#include "js/String.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusKeyField, BerytusField, (mValue), ())
NS_IMPL_ADDREF_INHERITED(BerytusKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusKeyField::BerytusKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  const BerytusFieldType& aFieldType,
  JS::Handle<JSObject*> aOptions
) : BerytusField(aGlobal, aFieldId, aFieldType, aOptions),
    mValue(nullptr) {}

BerytusKeyField::~BerytusKeyField() = default;

nsIGlobalObject* BerytusKeyField::GetParentObject() const {
  return mGlobal;
}

JSObject*
BerytusKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusKeyField> BerytusKeyField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusKeyFieldOptions& aOptions,
  ErrorResult& aRv
) {
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
    object
  );

  return obj.forget();
}

bool BerytusKeyField::HasValue() const {
  return mValue != nullptr;
}

void BerytusKeyField::GetValue(JSContext* aCx,
                               Nullable<ValueType>& aRetVal,
                               ErrorResult& aRv) const {
  if (!mValue) {
    aRetVal.SetNull();
    return;
  }
  aRetVal.Value().SetAsBerytusFieldValueDictionary() = mValue;
}

void BerytusKeyField::AddValueToJSON(JSContext* aCx,
                                     JS::Handle<JSObject*> aObj,
                                     ErrorResult& aRv) {
  JS::Rooted<JS::Value> value(aCx);
  if (!mValue) {
    value.setNull();
  } else {
    mValue->ToJSON(aCx, &value, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return;
    }
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, aObj, "value" ,value))) {
    aRv.Throw(NS_ERROR_FAILURE);
  }
}

void BerytusKeyField::SetValueImpl(JSContext* aCx,
                                   const Nullable<ValueType>& aValue,
                                   ErrorResult& aRv) {
  if (aValue.IsNull()) {
    mValue = nullptr;
    return;
  }
  if (aValue.Value().IsBerytusFieldValueDictionary()) {
    RefPtr<BerytusFieldValueDictionary> v = aValue.Value().GetAsBerytusFieldValueDictionary();
    if (v->Type() == BerytusFieldType::Key) {
      mValue = RefPtr<BerytusKeyFieldValue>(static_cast<BerytusKeyFieldValue*>(v.get()));
      return;
    }
  }
  MOZ_ASSERT(false, "Invalid Value for BerytusKeyField");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom
