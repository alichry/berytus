/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSecurePasswordField.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/OwningNonNull.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h"
#include "mozilla/dom/RootedDictionary.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSecurePasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusSecurePasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSecurePasswordField::BerytusSecurePasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusSecurePasswordFieldOptions&& aFieldOptions
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::SecurePassword,
                  Nullable<ValueUnion>()),
    mOptions(std::move(aFieldOptions)) {}

BerytusSecurePasswordField::BerytusSecurePasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusSecurePasswordFieldOptions&& aFieldOptions,
  Nullable<ValueUnion>&& aFieldValue
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::SecurePassword,
                 std::move(aFieldValue)),
    mOptions(std::move(aFieldOptions)) {}


BerytusSecurePasswordField::~BerytusSecurePasswordField() = default;

JSObject*
BerytusSecurePasswordField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSecurePasswordField_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusSecurePasswordField::GetParentObject() const { return mGlobal; }

BerytusSecurePasswordFieldOptions const& BerytusSecurePasswordField::Options() const {
  return mOptions;
}

void BerytusSecurePasswordField::CacheOptions(JSContext* aCx, ErrorResult& aRv) {
  if (mCachedOptions) {
    return;
  }
  JS::Rooted<JS::Value> options(aCx);
  if (NS_WARN_IF(!mOptions.ToObjectInternal(aCx, &options))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedOptions = options.toObjectOrNull();
}

bool BerytusSecurePasswordField::IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const {
  return aValue.IsNull() ||
    (aValue.Value().IsBerytusFieldValueDictionary() && aValue.Value().GetAsBerytusFieldValueDictionary()->Type() == Type());
}

already_AddRefed<BerytusSecurePasswordField> BerytusSecurePasswordField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusSecurePasswordFieldOptions& aOptions,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  JS::Rooted<JS::Value> jsOptions(aGlobal.Context());
  if (NS_WARN_IF(!aOptions.ToObjectInternal(aGlobal.Context(), &jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RootedDictionary<BerytusSecurePasswordFieldOptions> copiedOptions(aGlobal.Context());
  if (NS_WARN_IF(!copiedOptions.Init(aGlobal.Context(), jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  Nullable<ValueUnion> value;
  value.SetNull();
  RefPtr<BerytusSecurePasswordField> field = new BerytusSecurePasswordField(
    global,
    aId,
    std::move(copiedOptions),
    std::move(value)
  );
  return field.forget();
}


} // namespace mozilla::dom