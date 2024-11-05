/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/BerytusWebAppActorBinding.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusWebAppActor, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusWebAppActor)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusWebAppActor)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusWebAppActor)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusWebAppActor::BerytusWebAppActor(nsIGlobalObject* aGlobalObject) : mGlobal(aGlobalObject)
{
    // Add |MOZ_COUNT_CTOR(BerytusWebAppActor);| for a non-refcounted object.
}

BerytusWebAppActor::~BerytusWebAppActor()
{
    // Add |MOZ_COUNT_DTOR(BerytusWebAppActor);| for a non-refcounted object.
}

nsIGlobalObject* BerytusWebAppActor::GetParentObject() const { return mGlobal; }


} // namespace mozilla::dom
