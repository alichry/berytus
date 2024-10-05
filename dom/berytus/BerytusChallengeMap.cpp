/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChallengeMap.h"
#include "mozilla/dom/BerytusChallengeMapBinding.h"
#include "nsString.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusChallengeMap, mGlobal, mChallenges)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChallengeMap)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChallengeMap)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChallengeMap)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChallengeMap::BerytusChallengeMap(nsIGlobalObject* aGlobal) : mGlobal(aGlobal), mChallenges()
{
    // Add |MOZ_COUNT_CTOR(BerytusChallengeMap);| for a non-refcounted object.
}

BerytusChallengeMap::~BerytusChallengeMap()
{
    // Add |MOZ_COUNT_DTOR(BerytusChallengeMap);| for a non-refcounted object.
}

nsIGlobalObject* BerytusChallengeMap::GetParentObject() const { return mGlobal; }


JSObject*
BerytusChallengeMap::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusChallengeMap_Binding::Wrap(aCx, this, aGivenProto);
}

void BerytusChallengeMap::AddChallenge(BerytusChallenge* aChallenge, ErrorResult& aRv)
{
  nsString chId;
  aChallenge->GetID(chId);

  mozilla::dom::BerytusChallengeMap_Binding::MaplikeHelpers::Set(
    this,
    chId,
    *aChallenge,
    aRv
  );
  if (aRv.Failed()) {
    return;
  }
  // let's add the aField to an internal array to initial a RefPtr and increasing the
  // refcount... Just in case it's not ref'd anywhere else...
  // BRTTODO: Check if this is needed.
  RefPtr<BerytusChallenge> chPtr = aChallenge;
  mChallenges.AppendElement(chPtr);
}


bool BerytusChallengeMap::HasChallenge(const nsAString& aChallengeId, ErrorResult& aRv)
{
  return mozilla::dom::BerytusChallengeMap_Binding::MaplikeHelpers::Has(
    this, aChallengeId, aRv);
}

bool BerytusChallengeMap::HasActiveChallenge(const nsAString& aChallengeId, ErrorResult& aRv)
{
  bool exists = mozilla::dom::BerytusChallengeMap_Binding::MaplikeHelpers::Has(
    this, aChallengeId, aRv);
  if (! exists) {
    return false;
  }
  RefPtr<BerytusChallenge> ch = GetChallenge(aChallengeId, aRv);
  if (aRv.Failed()) {
    return false;
  }
  return ch->Active();
}

already_AddRefed<BerytusChallenge> BerytusChallengeMap::GetChallenge(const nsAString& aChallengeId, ErrorResult& aRv) {
  return mozilla::dom::BerytusChallengeMap_Binding::MaplikeHelpers::Get(
    this,aChallengeId, aRv);
}


} // namespace mozilla::dom
