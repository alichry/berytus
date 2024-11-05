/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusLoginOperationBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusLoginOperation, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusLoginOperation)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusLoginOperation)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusLoginOperation)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusLoginOperation::BerytusLoginOperation(
  nsIGlobalObject* aGlobal,
  const BerytusOnboardingIntent& aIntent
)
  : mGlobal(aGlobal), mIntent(aIntent)
{
    // Add |MOZ_COUNT_CTOR(BerytusLoginOperation);| for a non-refcounted object.
}

BerytusLoginOperation::~BerytusLoginOperation()
{
    // Add |MOZ_COUNT_DTOR(BerytusLoginOperation);| for a non-refcounted object.
}

nsIGlobalObject* BerytusLoginOperation::GetParentObject() const { return mGlobal; }

BerytusOnboardingIntent BerytusLoginOperation::Intent() const
{
  return mIntent;
}
} // namespace mozilla::dom
