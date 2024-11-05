/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusForeignIdentityField.h"
#include "BerytusEncryptedPacket.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusForeignIdentityFieldBinding.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusForeignIdentityField,
// # BerytusField, your, members, here)"
/*
//NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusForeignIdentityField, BerytusField, mAsEncrypted)
NS_IMPL_ADDREF_INHERITED(BerytusForeignIdentityField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusForeignIdentityField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusForeignIdentityField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)
*/

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusForeignIdentityField, BerytusField, (mAsEncrypted), ())
// --
//NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusForeignIdentityField, BerytusField)
// --: the above is equivalent to:
NS_IMPL_ADDREF_INHERITED(BerytusForeignIdentityField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusForeignIdentityField, BerytusField)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusForeignIdentityField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusForeignIdentityField::BerytusForeignIdentityField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions,
  BerytusEncryptedPacket* aValue
) : BerytusForeignIdentityField(aGlobal,
                         aFieldId,
                         BerytusFieldType::ForeignIdentity,
                         aOptions,
                         aValue,
                         nsString()) {}

BerytusForeignIdentityField::BerytusForeignIdentityField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  JS::Handle<JSObject*> aOptions,
  const nsAString& aValue
) : BerytusForeignIdentityField(aGlobal,
                         aFieldId,
                         BerytusFieldType::ForeignIdentity,
                         aOptions,
                         nullptr,
                         aValue) {}

BerytusForeignIdentityField::BerytusForeignIdentityField(
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


BerytusForeignIdentityField::~BerytusForeignIdentityField() = default;

nsIGlobalObject* BerytusForeignIdentityField::GetParentObject() const {
  return mGlobal;
}

JSObject*
BerytusForeignIdentityField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusForeignIdentityField_Binding::Wrap(aCx, this, aGivenProto);
}


already_AddRefed<BerytusForeignIdentityField> BerytusForeignIdentityField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusForeignIdentityFieldOptions& aOptions,
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

  RefPtr<BerytusForeignIdentityField> attr;
  if (!aDesiredValue.WasPassed()) {
    attr = new BerytusForeignIdentityField(
      global,
      aId,
      options,
      nullptr
    );
    return attr.forget();
  }
  if (aDesiredValue.Value().IsBerytusEncryptedPacket()) {
    attr = new BerytusForeignIdentityField(
      global,
      aId,
      options,
      // TODO(berytus): check if creating a pointer
      // from a reference given by NonNull<T> is a good idea.
      OwningNonNull(aDesiredValue.Value().GetAsBerytusEncryptedPacket())
    );
    return attr.forget();
  }

  attr = new BerytusForeignIdentityField(
    global,
    aId,
    options,
    aDesiredValue.Value().GetAsString()
  );
  return attr.forget();
}

bool BerytusForeignIdentityField::HasValue() const {
  return mAsEncrypted != nullptr || mAsString.Length() > 0;
}

void BerytusForeignIdentityField::GetValue(JSContext* aCx,
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

void BerytusForeignIdentityField::AddValueToJSON(JSContext* aCx,
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

void BerytusForeignIdentityField::SetValueImpl(JSContext* aCx,
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
  MOZ_ASSERT(false, "Invalid Value for BerytusForeignIdentityField");
  aRv.Throw(NS_ERROR_FAILURE);
}

} // namespace mozilla::dom