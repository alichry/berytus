/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "BerytusChallenge.h"
#include "mozilla/berytus/AgentProxy.h"
#include "nsIGlobalObject.h"
#include "BerytusChannel.h"
#include "BerytusLoginOperation.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperationBinding.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h"
#include "nsCOMPtr.h"
#include "mozilla/dom/BerytusChallengeMap.h"

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

bool BerytusAccountAuthenticationOperation::Active() const {
  return mActive;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusChallengeMap> BerytusAccountAuthenticationOperation::Challenges() const {
  return do_AddRef(mChallenges);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountAuthenticationOperation::Challenge(
    JSContext* aCx,
    const OwningNonNull<BerytusChallenge>& aChallenge,
    ErrorResult& aRv) {
  if (!Active()) {
    aRv.ThrowInvalidStateError("Operation is closed; can no longer send secret management requests");
    return nullptr;
  }
  if (NS_WARN_IF(!mChannel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  MOZ_ASSERT(!mChannel->Agent().IsDisabled());
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<BerytusChallenge::ConnectResult> prom = aChallenge->Connect(
      mChannel, this);
  auto onResolve = [this, aChallenge, outPromise](void*) {
    ErrorResult rv;
    mChallenges->AddChallenge(aChallenge, rv);
    if (NS_WARN_IF(rv.Failed())) {
      outPromise->MaybeReject(std::move(rv));
      return;
    }
    outPromise->MaybeResolveWithUndefined();
  };
  auto onReject = [outPromise](const berytus::Failure& aFr) {
    ErrorResult rv = aFr.ToErrorResult();
    outPromise->MaybeReject(std::move(rv));
  };
  prom->Then(GetCurrentSerialEventTarget(), __func__,
             std::move(onResolve), std::move(onReject));
  return outPromise.forget();
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountAuthenticationOperation::Finish(ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  if (NS_WARN_IF(!mChannel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  berytus::AgentProxy& agent = mChannel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  berytus::RequestContextWithOperation reqCtx;
  nsresult rv = berytus::Utils_RequestContextWithOperationMetadata(mGlobal, mChannel, this, reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  auto promise = agent.Login_CloseOperation(reqCtx);
  promise->Then(
    GetCurrentSerialEventTarget(),
    __func__,
    [this, outPromise](void*){
      // TODO(berytus): Not a major concern for the PoC;
      // however, a tiny window could be present where mActive = true
      // after Finish() was called but before resolution of the promise.
      mActive = false;
      outPromise->MaybeResolveWithUndefined();
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> BerytusAccountAuthenticationOperation::CreateApproved(
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId) {
  // TODO(berytus): Implement PopulateMetadata()
  RefPtr<BerytusAccountAuthenticationOperation> op = new BerytusAccountAuthenticationOperation(
    aGlobalObject, aChannel, aOperationId);
    return op->PopulateMetadata()->Then(GetCurrentSerialEventTarget(), __func__,
    [op]() -> RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> {
      return BerytusAccountAuthenticationOperation::CreationPromise::CreateAndResolve(op, __func__);
    },
    [](berytus::Failure&& aFr) -> RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> {
    return BerytusAccountAuthenticationOperation::CreationPromise::CreateAndReject(std::move(aFr), __func__);
    }
  );
}

} // namespace mozilla::dom
