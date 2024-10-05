/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusField.h"
#include "ErrorList.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "js/Value.h"
#include "mozilla/Assertions.h"
#include "mozilla/HoldDropJSObjects.h"
#include "mozilla/dom/BerytusFieldBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusField, (mGlobal), (mOptions, mCachedJson))
//NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusField, (mGlobal, mValue)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusField)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusField)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusField::BerytusField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  const BerytusFieldType& aFieldType,
  JS::Handle<JSObject*> aOptions
  // const BerytusAccountFieldDefinition& aOptions
) : mGlobal(aGlobal),
  mFieldId(aFieldId),
  mFieldType(aFieldType),
  mOptions(aOptions.get()),
  mCachedJson(nullptr) {
  MOZ_ASSERT(mOptions);
  mozilla::HoldJSObjects(this);
  // Add |MOZ_COUNT_CTOR(BerytusField);| for a non-refcounted object.
}

BerytusField::~BerytusField() {
  mozilla::DropJSObjects(this);
    // Add |MOZ_COUNT_DTOR(BerytusField);| for a non-refcounted object.
}

nsIGlobalObject* BerytusField::GetParentObject() const { return mGlobal; }

// JSObject*
// BerytusField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
// {
//   return BerytusField_Binding::Wrap(aCx, this, aGivenProto);
// }

void BerytusField::GetId(nsString& aRetVal) const
{
  aRetVal.Assign(mFieldId);
}

BerytusFieldType BerytusField::Type() const
{
  return mFieldType;
}

void BerytusField::GetOptions(JSContext* cx, JS::MutableHandle<JSObject*> aRetVal) const
{
  aRetVal.set(mOptions.get());
}

void BerytusField::ToJSON(JSContext* aCx,
                          JS::MutableHandle<JSObject*> aRetVal,
                          ErrorResult& aRv) {
  if (! mCachedJson) {
    mCachedJson = JS_NewPlainObject(aCx);
    JS::Rooted<JSObject*> obj(aCx, mCachedJson);

    AddValueToJSON(aCx, obj, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      mCachedJson = nullptr;
      return;
    }
    AddFieldMetadataToCachedJSON(aCx, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      mCachedJson = nullptr;
      return;
    }
  }
  aRetVal.set(mCachedJson);
}

void BerytusField::AddFieldMetadataToCachedJSON(JSContext* aCx, ErrorResult& aRv) {
  MOZ_ASSERT(mCachedJson);
  JS::Rooted<JSObject*> obj(aCx, mCachedJson);
  JSString* idStr = JS_NewUCStringCopyN(aCx, mFieldId.get(), mFieldId.Length());
  if (NS_WARN_IF(!idStr)) {
    aRv.Throw(NS_ERROR_FAILURE);
  }
  JS::Rooted<JS::Value> id(aCx, JS::StringValue(idStr));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", id))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  JSString* typeStr;
  if (mFieldType == BerytusFieldType::Identity) {
    typeStr = JS_NewStringCopyZ(aCx, "Identity");
  } else if (mFieldType == BerytusFieldType::ForeignIdentity) {
    typeStr = JS_NewStringCopyZ(aCx, "ForeignIdentity");
  } else if (mFieldType == BerytusFieldType::ConsumablePassword) {
    typeStr = JS_NewStringCopyZ(aCx, "ConsumablePassword");
  } else if (mFieldType == BerytusFieldType::Password) {
    typeStr = JS_NewStringCopyZ(aCx, "Password");
  } else if (mFieldType == BerytusFieldType::SecurePassword) {
    typeStr = JS_NewStringCopyZ(aCx, "SecurePassword");
  } else if (mFieldType == BerytusFieldType::Key) {
    typeStr = JS_NewStringCopyZ(aCx, "Key");
  } else if (mFieldType == BerytusFieldType::SharedKey) {
    typeStr = JS_NewStringCopyZ(aCx, "SharedKey");
  } else {
    MOZ_ASSERT(false, "Unrecognised Berytus Field Type");
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  JS::Rooted<JS::Value> type(aCx, JS::StringValue(typeStr));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", type))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  JS::Rooted<JS::Value> options(aCx, JS::ObjectValue(*mOptions.get()));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", options))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
}

void BerytusField::SetValue(JSContext* aCx,
                            const Nullable<ValueType>& aValue,
                            ErrorResult& aRv) {
  mCachedJson = nullptr;
  SetValueImpl(aCx, aValue, aRv);
}

} // namespace mozilla::dom
