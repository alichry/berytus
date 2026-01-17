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
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"
#include "nsString.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusField, (mGlobal, mFieldValue), (mCachedOptions, mCachedJson))
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
  Nullable<ValueUnion>&& aFieldValue
) : mGlobal(aGlobal),
  mFieldId(aFieldId),
  mFieldType(aFieldType),
  mFieldValue(std::move(aFieldValue)),
  mCachedOptions(nullptr),
  mCachedJson(nullptr) {
  mozilla::HoldJSObjects(this);
  // Add |MOZ_COUNT_CTOR(BerytusField);| for a non-refcounted object.
}

BerytusField::~BerytusField() {
  mozilla::DropJSObjects(this);
    // Add |MOZ_COUNT_DTOR(BerytusField);| for a non-refcounted object.
}

nsIGlobalObject* BerytusField::GetParentObject() const { return mGlobal; }

void BerytusField::GetId(nsString& aRetVal) const
{
  aRetVal.Assign(mFieldId);
}

BerytusFieldType BerytusField::Type() const
{
  return mFieldType;
}

Nullable<BerytusField::ValueUnion> const& BerytusField::GetValue() const {
  return mFieldValue;
}

void BerytusField::GetValue(JSContext* aCx,
                            Nullable<ValueUnion>& aRetVal,
                            ErrorResult& aRv) const {
  if (mFieldValue.IsNull()) {
    aRetVal.SetNull();
    return;
  }
  const ValueUnion& val = mFieldValue.Value();
  ValueUnion outVal;
  if (val.IsBerytusEncryptedPacket()) {
    outVal.SetAsBerytusEncryptedPacket() =
      OwningNonNull<BerytusEncryptedPacket>(
        val.GetAsBerytusEncryptedPacket());
  } else if (val.IsString()) {
    outVal.SetAsString().Assign(val.GetAsString());
  } else if (val.IsBerytusFieldValueDictionary()) {
    outVal.SetAsBerytusFieldValueDictionary() =
      OwningNonNull<BerytusFieldValueDictionary>(
        val.GetAsBerytusFieldValueDictionary());
  } else {
    MOZ_ASSERT(false, "Unrecognised field value union member");
    aRv.Throw(NS_ERROR_DOM_INVALID_STATE_ERR);
    return;
  }
  aRetVal.SetValue(std::move(outVal));
}

void BerytusField::GetOptions(JSContext* aCx,
                              JS::MutableHandle<JSObject*> aRetVal,
                              ErrorResult& aRv) {
  CacheOptions(aCx, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  MOZ_ASSERT(mCachedOptions);
  aRetVal.set(mCachedOptions);
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
  JS::Rooted<JSObject*> options(aCx);
  GetOptions(aCx, &options, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  JS::Rooted<JS::Value> optionsVal(aCx, JS::ObjectValue(*options));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", optionsVal))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
}

void BerytusField::AddValueToJSON(JSContext* aCx,
                                  JS::Handle<JSObject*> aObj,
                                  ErrorResult& aRv) {
  JS::Rooted<JS::Value> value(aCx, JS::NullValue());
  do {
    if (mFieldValue.IsNull()) {
      break;
    }
    const auto& intVal = mFieldValue.Value();
    if (intVal.IsBerytusEncryptedPacket()) {
      const auto& packet = intVal.GetAsBerytusEncryptedPacket();
      const auto exposed = packet->Exposed();
      const nsTDependentSubstring<char> exposedView((char*) exposed.data(),
                                                    exposed.Length());
      if (NS_WARN_IF(!ToJSValue(aCx, exposedView, &value))) {
        aRv.Throw(NS_ERROR_FAILURE);
        return;
      }
      break;
    }
    if (intVal.IsString()) {
      JSString* str = JS_NewUCStringCopyN(aCx, intVal.GetAsString().get(), intVal.GetAsString().Length());
      if (NS_WARN_IF(!str)) {
        aRv.Throw(NS_ERROR_FAILURE);
        return;
      }
      value.setString(str);
      break;
    }
    if (intVal.IsBerytusFieldValueDictionary()) {
      intVal.GetAsBerytusFieldValueDictionary()->ToJSON(aCx, &value, aRv);
      if (NS_WARN_IF(aRv.Failed())) {
        return;
      }
      break;
    }
    MOZ_ASSERT(false, "Unrecognised field value union member");
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  } while (false);
  if (NS_WARN_IF(!JS_SetProperty(aCx, aObj, "value" ,value))) {
    aRv.Throw(NS_ERROR_FAILURE);
  }
}

void BerytusField::SetValue(JSContext* aCx,
                            const Nullable<ValueUnion>& aValue,
                            ErrorResult& aRv) {
  mCachedJson = nullptr;
  SetValueImpl(aCx, aValue, aRv);
}

void BerytusField::SetValueImpl(JSContext* aCx,
                            const Nullable<ValueUnion>& aValue,
                            ErrorResult& aRv) {
  if (NS_WARN_IF(!IsValueValid(aValue))) {
    aRv.Throw(NS_ERROR_INVALID_ARG);
    return;
  }
  if (aValue.IsNull()) {
    mFieldValue.SetNull();
    return;
  }
  if (aValue.Value().IsString()) {
    mFieldValue.SetValue().SetAsString().Assign(aValue.Value().GetAsString());
    return;
  }
  if (aValue.Value().IsBerytusEncryptedPacket()) {
    const auto& packet = aValue.Value().GetAsBerytusEncryptedPacket();
    if (mChannel) {
      RefPtr<BerytusChannel> ch = mChannel.get();
      packet->Attach(ch, aRv);
      NS_ENSURE_TRUE_VOID(!aRv.Failed());
    }
    mFieldValue.SetValue().SetAsBerytusEncryptedPacket() = packet;
    return;
  }
  if (aValue.Value().IsBerytusFieldValueDictionary()) {
    mFieldValue.SetValue().SetAsBerytusFieldValueDictionary() = aValue.Value().GetAsBerytusFieldValueDictionary();
    return;
  }
  MOZ_ASSERT(false, "Unrecognised field value union member");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom
