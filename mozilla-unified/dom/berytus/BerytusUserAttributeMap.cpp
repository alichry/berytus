/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusUserAttributeMap.h"
#include "mozilla/dom/BerytusUserAttributeMapBinding.h"
#include "nsIProperty.h"
#include "nsISimpleEnumerator.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusUserAttributeMap, mGlobal, mAttributes)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusUserAttributeMap)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusUserAttributeMap)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusUserAttributeMap)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusUserAttributeMap::BerytusUserAttributeMap(nsIGlobalObject* aGlobal) : mGlobal(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusUserAttributeMap);| for a non-refcounted object.
}

BerytusUserAttributeMap::~BerytusUserAttributeMap()
{
    // Add |MOZ_COUNT_DTOR(BerytusUserAttributeMap);| for a non-refcounted object.
}

void BerytusUserAttributeMap::AddAttribute(
  BerytusUserAttribute* aAttribute,
  ErrorResult& aRv
)
{
  nsString attributeId;
  aAttribute->GetId(attributeId);

  mozilla::dom::BerytusUserAttributeMap_Binding::MaplikeHelpers::Set( // maybe worth copying it.
    this,
    attributeId,
    *aAttribute,
    aRv
  );
  if (aRv.Failed()) {
    return;
  }
  RefPtr<BerytusUserAttribute> attrPr = aAttribute;
  mAttributes.AppendElement(attrPr);
}

bool BerytusUserAttributeMap::HasAttribute(const nsString& aId) {
  for (const auto& attr : mAttributes) {
    nsString currentId;
    attr->GetId(currentId);
    if (currentId.Equals(aId)) {
      return true;
    }
  }
  return false;
}
RefPtr<BerytusUserAttribute> BerytusUserAttributeMap::GetAttribute(const nsString& aId) {
for (const auto& attr : mAttributes) {
    nsString currentId;
    attr->GetId(currentId);
    if (currentId.Equals(aId)) {
      return attr;
    }
  }
  return nullptr;
}

void BerytusUserAttributeMap::RemoveAttribute(const nsString& aId, ErrorResult& aRv) {
  for (size_t i = 0; i < mAttributes.Length(); i++) {
    const auto& attr = mAttributes.ElementAt(i);
    nsString currentId;
    attr->GetId(currentId);
    if (!currentId.Equals(aId)) {
      continue;
    }
    mAttributes.RemoveElementAt(i);
    mozilla::dom::BerytusUserAttributeMap_Binding::MaplikeHelpers::Delete(this, aId, aRv);
    return;
  }
}

nsIGlobalObject* BerytusUserAttributeMap::GetParentObject() const { return mGlobal; }

JSObject*
BerytusUserAttributeMap::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusUserAttributeMap_Binding::Wrap(aCx, this, aGivenProto);
}


} // namespace mozilla::dom
