/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusKeyField.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/OwningNonNull.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h"
#include "mozilla/dom/RootedDictionary.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusKeyField::BerytusKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusKeyFieldOptions&& aFieldOptions
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::Key,
                 Nullable<ValueUnion>()),
    mOptions(std::move(aFieldOptions)) {}


BerytusKeyField::BerytusKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusKeyFieldOptions&& aFieldOptions,
  Nullable<ValueUnion>&& aFieldValue
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::Key,
                 std::move(aFieldValue)),
    mOptions(std::move(aFieldOptions)) {}


BerytusKeyField::~BerytusKeyField() = default;

JSObject*
BerytusKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusKeyField::GetParentObject() const { return mGlobal; }

BerytusKeyFieldOptions const& BerytusKeyField::Options() const {
  return mOptions;
}

void BerytusKeyField::CacheOptions(JSContext* aCx, ErrorResult& aRv) {
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

bool BerytusKeyField::IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const {
  return aValue.IsNull() ||
    (aValue.Value().IsBerytusFieldValueDictionary() && aValue.Value().GetAsBerytusFieldValueDictionary()->Type() == Type());
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
  if (NS_WARN_IF(!aOptions.ToObjectInternal(aGlobal.Context(), &jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RootedDictionary<BerytusKeyFieldOptions> copiedOptions(aGlobal.Context());
  if (NS_WARN_IF(!copiedOptions.Init(aGlobal.Context(), jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  Nullable<ValueUnion> value;
  value.SetNull();
  RefPtr<BerytusKeyField> field = new BerytusKeyField(
    global,
    aId,
    std::move(copiedOptions),
    std::move(value)
  );
  return field.forget();
}


} // namespace mozilla::dom