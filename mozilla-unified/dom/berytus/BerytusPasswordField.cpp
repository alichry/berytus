/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusPasswordField.h"
#include "BerytusEncryptedPacket.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusPasswordFieldBinding.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusPasswordField,
// # BerytusField, your, members, here)"
/*
//NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusPasswordField, BerytusField, mAsEncrypted)
NS_IMPL_ADDREF_INHERITED(BerytusPasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusPasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusPasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)
*/

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusPasswordField, BerytusField, (mAsEncrypted), ())
// --
//NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusPasswordField, BerytusField)
// --: the above is equivalent to:
NS_IMPL_ADDREF_INHERITED(BerytusPasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusPasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusPasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusPasswordField::BerytusPasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions,
  BerytusEncryptedPacket* aValue
) : BerytusPasswordField(aGlobal,
                         aFieldId,
                         BerytusFieldType::Password,
                         aOptions,
                         aValue,
                         nsString()) {}

BerytusPasswordField::BerytusPasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions,
  const nsAString& aValue
) : BerytusPasswordField(aGlobal,
                         aFieldId,
                         BerytusFieldType::Password,
                         aOptions,
                         nullptr,
                         aValue) {}

BerytusPasswordField::BerytusPasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  const BerytusFieldType& aFieldType,
  JS::Handle<JSObject*> aOptions,
  BerytusEncryptedPacket* aAsEncrypted,
  const nsAString& aAsString
) : BerytusField(aGlobal,
                 aFieldId,
                 aFieldType,
                 aOptions),
    mAsEncrypted(aAsEncrypted),
    mAsString(aAsString) { };


BerytusPasswordField::~BerytusPasswordField() = default;

nsIGlobalObject* BerytusPasswordField::GetParentObject() const {
  return mGlobal;
}

JSObject*
BerytusPasswordField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusPasswordField_Binding::Wrap(aCx, this, aGivenProto);
}


already_AddRefed<BerytusPasswordField> BerytusPasswordField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusPasswordFieldOptions& aOptions,
  const Optional<StringOrBerytusEncryptedPacket>& aDesiredValue,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> optionsV(aGlobal.Context());
  aOptions.ToObjectInternal(aGlobal.Context(), &optionsV);
  JS::Rooted<JSObject*> options(
    aGlobal.Context(),
    &optionsV.toObject()
  );

  RefPtr<BerytusPasswordField> attr;
  if (!aDesiredValue.WasPassed()) {
    attr = new BerytusPasswordField(
      global,
      aId,
      options,
      nullptr
    );
    return attr.forget();
  }
  if (aDesiredValue.Value().IsBerytusEncryptedPacket()) {
    attr = new BerytusPasswordField(
      global,
      aId,
      options,
      // TODO(berytus): check if creating a pointer
      // from a reference given by NonNull<T> is a good idea.
      OwningNonNull(aDesiredValue.Value().GetAsBerytusEncryptedPacket())
    );
    return attr.forget();
  }

  attr = new BerytusPasswordField(
    global,
    aId,
    options,
    aDesiredValue.Value().GetAsString()
  );
  return attr.forget();
}

bool BerytusPasswordField::HasValue() const {
  return mAsEncrypted != nullptr || mAsString.Length() > 0;
}

void BerytusPasswordField::GetValue(JSContext* aCx,
                                    Nullable<ValueType>& aRetVal,
                                    ErrorResult& aRv) const {
  ValueType val;
  if (mAsEncrypted) {
    val.SetAsBerytusEncryptedPacket() = mAsEncrypted;
  } else if (mAsString.Length() > 0) {
    val.SetAsString().Assign(mAsString);
  } else {
    aRetVal.SetNull();
    return;
  }
  aRetVal.SetValue(std::move(val));
}

void BerytusPasswordField::AddValueToJSON(JSContext* aCx,
                                  JS::Handle<JSObject*> aObj,
                                  ErrorResult& aRv) {
  JS::Rooted<JS::Value> value(aCx);
  if (mAsEncrypted) {
    BerytusEncryptedPacketJSON packetJson;
    mAsEncrypted->ToJSON(packetJson, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return;
    }
    packetJson.ToObjectInternal(aCx, &value);
  } else if (mAsString.Length() > 0) {
    JSString* str = JS_NewUCStringCopyN(aCx, mAsString.get(), mAsString.Length());
    if (NS_WARN_IF(!str)) {
      aRv.Throw(NS_ERROR_FAILURE);
      return;
    }
    value.setString(str);
  } else {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, aObj, "value" ,value))) {
    aRv.Throw(NS_ERROR_FAILURE);
  }
}

void BerytusPasswordField::SetValueImpl(JSContext* aCx,
                                        const Nullable<ValueType>& aValue,
                                        ErrorResult& aRv) {
  if (aValue.IsNull()) {
    mAsEncrypted = nullptr;
    mAsString.Truncate();
    return;
  }
  if (aValue.Value().IsString()) {
    mAsString.Assign(aValue.Value().GetAsString());
    return;
  }
  if (aValue.Value().IsBerytusEncryptedPacket()) {
    mAsEncrypted = aValue.Value().GetAsBerytusEncryptedPacket();
    return;
  }
  MOZ_ASSERT(false, "Invalid Value for BerytusPasswordField");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom