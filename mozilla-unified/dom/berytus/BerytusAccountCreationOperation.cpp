/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusAccountCreationOperation.h"
#include "BerytusAccountAuthenticationOperation.h"
#include "BerytusChannel.h"
#include "BerytusLoginOperation.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountCreationOperationBinding.h"
#include "mozilla/dom/BerytusFieldMap.h"
#include "mozilla/dom/Promise.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountCreationOperation,
// # BerytusLoginOperation, your, members, here)"
// NS_IMPL_ADDREF_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
// NS_IMPL_RELEASE_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
// NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAccountCreationOperation)
// NS_INTERFACE_MAP_END_INHERITING(BerytusLoginOperation)
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation, mFields, mUserAttributes)
NS_IMPL_ADDREF_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
NS_IMPL_RELEASE_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAccountCreationOperation)
NS_INTERFACE_MAP_END_INHERITING(BerytusLoginOperation)

BerytusAccountCreationOperation::BerytusAccountCreationOperation(nsIGlobalObject* aGlobalObject,
                                                                 const RefPtr<BerytusChannel>& aChannel,
                                                                 const nsAString& aOperationId)
  : BerytusLoginOperation(aGlobalObject, aChannel, aOperationId, BerytusOnboardingIntent::Register),
    mChannel(aChannel),
    mFields(new BerytusFieldMap(aGlobalObject)),
    mUserAttributes(new BerytusUserAttributeMap(aGlobalObject))
{
    // Add |MOZ_COUNT_CTOR(BerytusAccountCreationOperation);| for a non-refcounted object.
}

BerytusAccountCreationOperation::~BerytusAccountCreationOperation()
{
    // Add |MOZ_COUNT_DTOR(BerytusAccountCreationOperation);| for a non-refcounted object.
}

JSObject*
BerytusAccountCreationOperation::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusAccountCreationOperation_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusAccountCreationOperation::GetParentObject() const { return mGlobal; }

BerytusChannel* BerytusAccountCreationOperation::Channel() const {
  return mChannel;
}

BerytusLoginOperation* BerytusAccountCreationOperation::Operation() {
  return this;
}

bool BerytusAccountCreationOperation::Active() const {
  return mActive;
}

BerytusFieldMap* BerytusAccountCreationOperation::FieldMap() const {
  return mFields;
}
BerytusUserAttributeMap* BerytusAccountCreationOperation::UserAttributeMap() const {
  return mUserAttributes;
}

bool BerytusAccountCreationOperation::Newborn() const {
  return false;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountCreationOperation::Save(ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  if (!Active()) {
    aRv.ThrowInvalidStateError("Operation is closed; can no longer send secret management requests");
    return nullptr;
  }
  if (NS_WARN_IF(!mChannel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active; can no longer send secret management requests");
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
  auto promise = agent.Login_CloseOpeation(reqCtx);
  promise->Then(
    GetCurrentSerialEventTarget(),
    __func__,
    [this, outPromise](void*){
      // TODO(berytus): Not a major concern for the PoC;
      // however, a tiny window could be present where mActive = true
      // after Save() was called but the promise not yet resolved.
      mActive = false;
      outPromise->MaybeResolveWithUndefined();
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountCreationOperation::TransitionToAuthOperation(ErrorResult& aRv) {
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
  berytus::ApproveTransitionToAuthOpArgs reqArgs;
  reqArgs.mNewAuthOp.mIntent.SetAsAuthenticate();
  reqArgs.mNewAuthOp.mType.SetAsAuthentication();
  nsIDToCString oUuidString(nsID::GenerateUUID());
  nsString oId = NS_ConvertUTF8toUTF16(oUuidString.get());
  reqArgs.mNewAuthOp.mId.Assign(oId);
  reqArgs.mNewAuthOp.mStatus.SetAsPending();
  // TODO(berytus): if AccountAuthentication op should provide
  // user attribute info, we need to populate mRequestedUserAttributes

  auto promise = agent.AccountCreation_ApproveTransitionToAuthOp(reqCtx, reqArgs);
  promise->Then(
    GetCurrentSerialEventTarget(),
    __func__,
    [this, oId](void*) -> RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> {
      // TODO(berytus): Not a major concern for the PoC;
      // however, a tiny window could be present where mActive = true
      // after Save() was called but before resolution of the promise.
      mActive = false;
      return BerytusAccountAuthenticationOperation::CreateApproved(mGlobal, mChannel, oId);
    },
    [](berytus::Failure&& aFr) -> RefPtr<BerytusAccountAuthenticationOperation::CreationPromise> {
      return BerytusAccountAuthenticationOperation::CreationPromise::CreateAndReject(std::move(aFr), __func__);
    }
  )->Then(
    GetCurrentSerialEventTarget(), __func__,
    [outPromise](const RefPtr<BerytusAccountAuthenticationOperation>& aOp) {
      outPromise->MaybeResolve(aOp);
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    });
  return outPromise.forget();
}

RefPtr<BerytusAccountCreationOperation::CreationPromise> BerytusAccountCreationOperation::CreateApproved(
    JSContext* aCx,
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId) {
  RefPtr<BerytusAccountCreationOperation> op = new BerytusAccountCreationOperation(
      aGlobalObject, aChannel, aOperationId);
  return op->PopulateUserAttributeMap(aCx)->Then(GetCurrentSerialEventTarget(), __func__,
    [op]() -> RefPtr<BerytusAccountCreationOperation::CreationPromise> {
      return BerytusAccountCreationOperation::CreationPromise::CreateAndResolve(op, __func__);
    },
    [](const berytus::Failure& aFr) -> RefPtr<BerytusAccountCreationOperation::CreationPromise> {
    return BerytusAccountCreationOperation::CreationPromise::CreateAndReject(aFr, __func__);
    }
  );
}

} // namespace mozilla::dom
