/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusFieldValueDictionaryBinding.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusFieldValueDictionary, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusFieldValueDictionary)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusFieldValueDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusFieldValueDictionary)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusFieldValueDictionary::BerytusFieldValueDictionary(nsIGlobalObject* aGlobal) : mGlobal(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusFieldValueDictionary);| for a non-refcounted object.
}

BerytusFieldValueDictionary::~BerytusFieldValueDictionary()
{
    // Add |MOZ_COUNT_DTOR(BerytusFieldValueDictionary);| for a non-refcounted object.
}

nsIGlobalObject* BerytusFieldValueDictionary::GetParentObject() const { return mGlobal; }

bool BerytusFieldValueDictionary::IsForField(const BerytusFieldType& aType)
{
  return Type() == aType;
}

} // namespace mozilla::dom
