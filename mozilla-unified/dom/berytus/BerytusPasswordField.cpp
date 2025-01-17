/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusPasswordField.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "mozilla/OwningNonNull.h"
#include "mozilla/dom/BerytusField.h"
#include "js/Value.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusPasswordFieldBinding.h"
#include "mozilla/dom/RootedDictionary.h"
#include "mozilla/dom/UnionTypes.h" // StringOrBerytusEncryptedPacket
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusPasswordField, BerytusField)
NS_IMPL_RELEASE_INHERITED(BerytusPasswordField, BerytusField)
NS_INTERFACE_MAP_BEGIN(BerytusPasswordField)
NS_INTERFACE_MAP_END_INHERITING(BerytusField)

BerytusPasswordField::BerytusPasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusPasswordFieldOptions&& aFieldOptions
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::Password,
                 Nullable<ValueUnion>()),
    mOptions(std::move(aFieldOptions)) {}


BerytusPasswordField::BerytusPasswordField(
  nsIGlobalObject* aGlobal,
  const nsAString& aFieldId,
  BerytusPasswordFieldOptions&& aFieldOptions,
  Nullable<ValueUnion>&& aFieldValue
) : BerytusField(aGlobal,
                 aFieldId,
                 BerytusFieldType::Password,
                 std::move(aFieldValue)),
    mOptions(std::move(aFieldOptions)) {}


BerytusPasswordField::~BerytusPasswordField() = default;

JSObject*
BerytusPasswordField::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusPasswordField_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusPasswordField::GetParentObject() const { return mGlobal; }

BerytusPasswordFieldOptions const& BerytusPasswordField::Options() const {
  return mOptions;
}

void BerytusPasswordField::CacheOptions(JSContext* aCx, ErrorResult& aRv) {
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

bool BerytusPasswordField::IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const {
  return aValue.IsNull() ||
    aValue.Value().IsBerytusEncryptedPacket() ||
    aValue.Value().IsString();
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
  JS::Rooted<JS::Value> jsOptions(aGlobal.Context());
  if (NS_WARN_IF(!aOptions.ToObjectInternal(aGlobal.Context(), &jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RootedDictionary<BerytusPasswordFieldOptions> copiedOptions(aGlobal.Context());
  if (NS_WARN_IF(!copiedOptions.Init(aGlobal.Context(), jsOptions))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  Nullable<ValueUnion> value;
  if (!aDesiredValue.WasPassed()) {
    value.SetNull();
  } else if (aDesiredValue.Value().IsString()) {
    value.SetValue().SetAsString().Assign(aDesiredValue.Value().GetAsString());
  } else {
    MOZ_ASSERT(aDesiredValue.Value().IsBerytusEncryptedPacket());
    value.SetValue().SetAsBerytusEncryptedPacket() =
      // TODO(berytus): check if creating a pointer
      // from a reference given by const NonNull<T> is a good idea.
      OwningNonNull(aDesiredValue.Value().GetAsBerytusEncryptedPacket());
  }
  RefPtr<BerytusPasswordField> field = new BerytusPasswordField(
    global,
    aId,
    std::move(copiedOptions),
    std::move(value)
  );
  return field.forget();
}


} // namespace mozilla::dom