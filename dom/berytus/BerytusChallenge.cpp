/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChallenge.h"
#include "mozilla/dom/BerytusChallengeBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusChallenge, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChallenge)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChallenge)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChallenge)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChallenge::BerytusChallenge(nsIGlobalObject* aGlobal,
                                   const nsAString& aID) :
                                   mGlobal(aGlobal),
                                   mID(aID),
                                   mActive(true)
{
    // Add |MOZ_COUNT_CTOR(BerytusChallenge);| for a non-refcounted object.
}

BerytusChallenge::~BerytusChallenge()
{
    // Add |MOZ_COUNT_DTOR(BerytusChallenge);| for a non-refcounted object.
}

nsIGlobalObject* BerytusChallenge::GetParentObject() const { return mGlobal; }

JSObject*
BerytusChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

bool BerytusChallenge::Active() { return mActive; }

void BerytusChallenge::GetID(nsAString& aRetVal) {
  aRetVal.Assign(mID);
}

} // namespace mozilla::dom
