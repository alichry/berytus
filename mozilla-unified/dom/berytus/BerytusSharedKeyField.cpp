/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSharedKeyField.h"
#include "BerytusFieldValueDictionary.h"
#include "js/ArrayBuffer.h"
#include "js/String.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusSharedKeyField, BerytusField, (mValue), ())
NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSharedKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSharedKeyField::BerytusSharedKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions,
  const RefPtr<BerytusSharedKeyFieldValue>& aValue
) : BerytusField(aGlobal, aFieldId, BerytusFieldType::SharedKey, aOptions),
    mValue(aValue) {}

BerytusSharedKeyField::~BerytusSharedKeyField() = default;

nsIGlobalObject* BerytusSharedKeyField::GetParentObject() const {
  return mGlobal;
}

JSObject*
BerytusSharedKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSharedKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusSharedKeyField> BerytusSharedKeyField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusKeyFieldOptions& aOptions,
  const Optional<NonNull<BerytusSharedKeyFieldValue>>& aPrivateKeyValue,
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

  if (!aPrivateKeyValue.WasPassed()) {
    RefPtr<BerytusSharedKeyField> obj = new BerytusSharedKeyField(
      global,
      aId,
      object,
      nullptr
    );

    return obj.forget();
  }

  nsresult rv;

  RefPtr<BerytusSharedKeyFieldValue> copiedValue = aPrivateKeyValue.Value().Clone(&rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  RefPtr<BerytusSharedKeyField> obj = new BerytusSharedKeyField(
    global,
    aId,
    object,
    //copiedValue
    RefPtr(aPrivateKeyValue.InternalValue().get())
  );

  return obj.forget();
}

bool BerytusSharedKeyField::HasValue() const {
  return mValue != nullptr;
}

void BerytusSharedKeyField::GetValue(JSContext* aCx,
                                     Nullable<ValueType>& aRetVal,
                                     ErrorResult& aRv) const {
  if (!mValue) {
    aRetVal.SetNull();
    return;
  }
  aRetVal.SetValue().SetAsBerytusFieldValueDictionary() = mValue;
}

void BerytusSharedKeyField::AddValueToJSON(JSContext* aCx,
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

void BerytusSharedKeyField::SetValueImpl(JSContext* aCx,
                                   const Nullable<ValueType>& aValue,
                                   ErrorResult& aRv) {
  if (aValue.IsNull()) {
    mValue = nullptr;
    return;
  }
  if (aValue.Value().IsBerytusFieldValueDictionary()) {
    RefPtr<BerytusFieldValueDictionary> v = aValue.Value().GetAsBerytusFieldValueDictionary();
    if (v->Type() == BerytusFieldType::SharedKey) {
      mValue = RefPtr<BerytusSharedKeyFieldValue>(static_cast<BerytusSharedKeyFieldValue*>(v.get()));
      return;
    }
  }
  MOZ_ASSERT(false, "Invalid Value for BerytusSharedKeyField");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom
