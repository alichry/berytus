/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "BerytusChannel.h"
#include "BerytusLoginOperation.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperationBinding.h"
#include "nsCOMPtr.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountAuthenticationOperation,
// # BerytusLoginOperation, your, members, here)"
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountAuthenticationOperation, BerytusLoginOperation)
NS_IMPL_ADDREF_INHERITED(BerytusAccountAuthenticationOperation, BerytusLoginOperation)
NS_IMPL_RELEASE_INHERITED(BerytusAccountAuthenticationOperation, BerytusLoginOperation)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAccountAuthenticationOperation)
NS_INTERFACE_MAP_END_INHERITING(BerytusLoginOperation)

BerytusAccountAuthenticationOperation::BerytusAccountAuthenticationOperation(nsIGlobalObject* aGlobalObject,
                                                                             const RefPtr<BerytusChannel>& aChannel,
                                                                             const nsAString& aOperationId)
  : BerytusLoginOperation(aGlobalObject, aChannel, aOperationId, BerytusOnboardingIntent::Authenticate),
    mChallenges(new BerytusChallengeMap(aGlobalObject))
{
    // Add |MOZ_COUNT_CTOR(BerytusAccountAuthenticationOperation);| for a non-refcounted object.
}

BerytusAccountAuthenticationOperation::~BerytusAccountAuthenticationOperation()
{
    // Add |MOZ_COUNT_DTOR(BerytusAccountAuthenticationOperation);| for a non-refcounted object.
}

JSObject*
BerytusAccountAuthenticationOperation::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusAccountAuthenticationOperation_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusAccountAuthenticationOperation::GetParentObject() const { return mGlobal; }

BerytusChannel* BerytusAccountAuthenticationOperation::Channel() const {
  return mChannel;
}
BerytusLoginOperation* BerytusAccountAuthenticationOperation::Operation() {
  return this;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusChallengeMap> BerytusAccountAuthenticationOperation::Challenges() const {
  return do_AddRef(mChallenges);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountAuthenticationOperation::CreateChallenge(
    const nsAString& challengeId,
    BerytusChallengeType challengeType,
    const Optional<BerytusChallengeParameters*>& challengeParameters,
    ErrorResult& aRv) {
  MOZ_ASSERT(false, "BerytusAccountAuthenticationOperation::CreateChallenge");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountAuthenticationOperation::Finish(ErrorResult& aRv) {
  MOZ_ASSERT(false, "BerytusAccountAuthenticationOperation::Finish");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}

RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> BerytusAccountAuthenticationOperation::CreateApproved(
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId) {
  RefPtr<BerytusAccountAuthenticationOperation> op = new BerytusAccountAuthenticationOperation(
    aGlobalObject, aChannel, aOperationId);
  return BerytusAccountAuthenticationOperation::CreationPromise::CreateAndResolve(op, __func__);
}


} // namespace mozilla::dom
