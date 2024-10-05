/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusFieldMap.h"
#include "mozilla/dom/BerytusFieldMapBinding.h"

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
  : mGlobal(aGlobal),
  mFields()
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
  // let's add the aField to an internal array to initial a RefPtr and increasing the
  // refcount... Just in case it's not ref'd anywhere else...
  RefPtr<BerytusField> fieldPtr = aField;
  mFields.AppendElement(fieldPtr);
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
