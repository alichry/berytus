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
already_AddRefed<Promise> BerytusAccountAuthenticationOperation::CreateChallenge(
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
  const berytus::AgentProxy& agent = mChannel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  berytus::RequestContextWithOperation reqCtx;
  nsresult rv = berytus::Utils_RequestContextWithOperationMetadata(mGlobal, mChannel, this, reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  berytus::ApproveChallengeRequestArgs reqArgs;
  berytus::utils::ToProxy::BerytusChallengeInfoUnion(aChallenge, reqArgs.mChallenge);
  // TODO(berytus): Use BuildChallengeInfo
  RefPtr<Promise> prom = agent.CallSendQuery(aCx,
                      u"accountAuthentication"_ns,
                      u"approveChallengeRequest"_ns,
                      reqCtx,
                      &reqArgs,
                      aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  auto onResolve = [this, aChallenge](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) -> already_AddRefed<Promise> {
    mChallenges->AddChallenge(aChallenge, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return nullptr;
    }
    aChallenge->Connect(mChannel, this);
    return Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    berytus::Failure fr;
    FromJSVal(aCx, aValue, fr);
    ErrorResult err = fr.ToErrorResult();
    return mozilla::dom::Promise::CreateRejectedWithErrorResult(aGlobal, err);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    aRv.Throw(thenRes.unwrapErr());
    return nullptr;
  }
  return thenRes.unwrap().forget();
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
  const berytus::AgentProxy& agent = mChannel->Agent();
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
