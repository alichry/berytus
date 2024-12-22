/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSharedKeyField.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/OwningNonNull.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h"
#include "mozilla/dom/RootedDictionary.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusSharedKeyField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusSharedKeyField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusSharedKeyField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusSharedKeyField::BerytusSharedKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusSharedKeyFieldOptions&& aFieldOptions
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::SharedKey,
                 Nullable<ValueUnion>()),
    mOptions(std::move(aFieldOptions)) {}

BerytusSharedKeyField::BerytusSharedKeyField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusSharedKeyFieldOptions&& aFieldOptions,
  Nullable<ValueUnion>&& aFieldValue
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::SharedKey,
                 std::move(aFieldValue)),
    mOptions(std::move(aFieldOptions)) {}


BerytusSharedKeyField::~BerytusSharedKeyField() = default;

JSObject*
BerytusSharedKeyField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSharedKeyField_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusSharedKeyField::GetParentObject() const { return mGlobal; }

void BerytusSharedKeyField::CacheOptions(JSContext* aCx, ErrorResult& aRv) {
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

bool BerytusSharedKeyField::IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const {
  return aValue.IsNull() ||
    (aValue.Value().IsBerytusFieldValueDictionary() && aValue.Value().GetAsBerytusFieldValueDictionary()->Type() == Type());
}

already_AddRefed<BerytusSharedKeyField> BerytusSharedKeyField::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusSharedKeyFieldOptions& aOptions,
  const Optional<NonNull<BerytusSharedKeyFieldValue>>& aDesiredValue,
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
  RootedDictionary<BerytusSharedKeyFieldOptions> copiedOptions(aGlobal.Context());
  if (NS_WARN_IF(!copiedOptions.Init(aGlobal.Context(), jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  Nullable<ValueUnion> value;
  if (!aDesiredValue.WasPassed()) {
    value.SetNull();
  } else {
    value.SetValue().SetAsBerytusFieldValueDictionary() =
      // TODO(berytus): check if creating a pointer
      // from a reference given by const NonNull<T> is a good idea.
      OwningNonNull(aDesiredValue.Value());
  }
  RefPtr<BerytusSharedKeyField> field = new BerytusSharedKeyField(
    global,
    aId,
    std::move(copiedOptions),
    std::move(value)
  );
  return field.forget();
}


} // namespace mozilla::dom