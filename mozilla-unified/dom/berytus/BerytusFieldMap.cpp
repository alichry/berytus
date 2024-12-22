/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusFieldMap.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/BerytusFieldMapBinding.h"
#include "mozilla/dom/BerytusIdentityField.h"
#include "mozilla/dom/BerytusForeignIdentityField.h"
#include "mozilla/dom/BerytusPasswordField.h"
#include "mozilla/dom/BerytusSecurePasswordField.h"
#include "mozilla/dom/BerytusKeyField.h"
#include "mozilla/dom/BerytusSharedKeyField.h"
#include "nsCOMPtr.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusFieldMap, mGlobal, mFields)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusFieldMap)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusFieldMap)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusFieldMap)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusFieldMap::BerytusFieldMap(nsIGlobalObject* aGlobal)
  : mGlobal(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusFieldMap);| for a non-refcounted object.
}

BerytusFieldMap::~BerytusFieldMap()
{
    // Add |MOZ_COUNT_DTOR(BerytusFieldMap);| for a non-refcounted object.
}

nsIGlobalObject* BerytusFieldMap::GetParentObject() const { return mGlobal; }


JSObject*
BerytusFieldMap::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusFieldMap_Binding::Wrap(aCx, this, aGivenProto);
}

const nsTArray<RefPtr<BerytusField>>& BerytusFieldMap::List() const {
  return mFields;
}

void BerytusFieldMap::AddField(BerytusField* aField, ErrorResult& aRv)
{
  nsString fieldId;
  aField->GetId(fieldId);

  mozilla::dom::BerytusFieldMap_Binding::MaplikeHelpers::Set(
    this,
    fieldId,
    *aField,
    aRv
  );
  if (aRv.Failed()) {
    return;
  }
  RefPtr<BerytusField> field = aField;
  mFields.AppendElement(field);
}

bool BerytusFieldMap::HasField(const nsAString& aFieldId, ErrorResult& aRv)
{
  return mozilla::dom::BerytusFieldMap_Binding::MaplikeHelpers::Has(
    this, aFieldId, aRv);
}

already_AddRefed<BerytusField> BerytusFieldMap::GetField(const nsAString& aFieldId, ErrorResult& aRv) {
  return mozilla::dom::BerytusFieldMap_Binding::MaplikeHelpers::Get(
    this,aFieldId, aRv);
}

void BerytusFieldMap::ToRecord(Record<nsString, RefPtr<BerytusField>>& aOutRecord) {
  for (const auto& field : mFields) {
    auto newEntry = aOutRecord.Entries().AppendElement();
    nsString fieldId;
    field->GetId(fieldId);
    newEntry->mKey = fieldId;
    newEntry->mValue = field;
  }
}

} // namespace mozilla::dom
