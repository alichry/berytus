/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSecurePasswordField.h"
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusSecurePasswordField, BerytusField, (mValue), ())
NS_IMPL_ADDREF_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSecurePasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSecurePasswordField::BerytusSecurePasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions
) : BerytusField(aGlobal, aFieldId, BerytusFieldType::SecurePassword, aOptions) {}

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
    object
  );
  return obj.forget();
}

bool BerytusSecurePasswordField::HasValue() const {
  return mValue != nullptr;
}

void BerytusSecurePasswordField::GetValue(JSContext* aCx,
                                          Nullable<ValueType>& aRetVal,
                                          ErrorResult& aRv) const {
  if (!mValue) {
    aRetVal.SetNull();
    return;
  }
  aRetVal.SetValue().SetAsBerytusFieldValueDictionary() = mValue;
}

void BerytusSecurePasswordField::AddValueToJSON(JSContext* aCx,
                                                JS::Handle<JSObject*> aObj,
                                                ErrorResult& aRv) {
  JS::Rooted<JS::Value> value(aCx);
  if (!mValue) {
    value.setNull();
  } else {
    mValue->ToJSON(aCx, &value, aRv);
    if (aRv.Failed()) {
      return;
    }
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, aObj, "value" ,value))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
}

void BerytusSecurePasswordField::SetValueImpl(JSContext* aCx,
                                              const Nullable<ValueType>& aValue,
                                              ErrorResult& aRv) {
  if (aValue.IsNull()) {
    mValue = nullptr;
    return;
  }
  if (aValue.Value().IsBerytusFieldValueDictionary()) {
    RefPtr<BerytusFieldValueDictionary> v = aValue.Value().GetAsBerytusFieldValueDictionary();
    if (v->Type() == BerytusFieldType::SecurePassword) {
      mValue = RefPtr<BerytusSecurePasswordFieldValue>(static_cast<BerytusSecurePasswordFieldValue*>(v.get()));
      return;
    }
  }
  MOZ_ASSERT(false, "Invalid Value for BerytusSecurePasswordField");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom
