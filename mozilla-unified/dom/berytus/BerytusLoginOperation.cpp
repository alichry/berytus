/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusLoginOperation.h"
#include "js/Realm.h"
#include "js/Value.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusLoginOperationBinding.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/BerytusChannel.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "mozilla/dom/BerytusAccountCreationOperation.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusLoginOperation, mGlobal, mChannel)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusLoginOperation)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusLoginOperation)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusLoginOperation)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusLoginOperation::BerytusLoginOperation(
  nsIGlobalObject* aGlobal,
  const RefPtr<BerytusChannel>& aChannel,
  const nsAString& aOperationId,
  const BerytusOnboardingIntent& aIntent
)
  : mGlobal(aGlobal), mChannel(aChannel), mId(aOperationId), mIntent(aIntent), mActive(true)
{
    // Add |MOZ_COUNT_CTOR(BerytusLoginOperation);| for a non-refcounted object.
  nsIDToCString uuidString(nsID::GenerateUUID());
  mId.Assign(NS_ConvertUTF8toUTF16(uuidString.get()));
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

void BerytusLoginOperation::GetID(nsString& aRv) const {
  aRv.Assign(mId);
}


already_AddRefed<Promise> BerytusLoginOperation::Close(ErrorResult& aRv) {
  nsresult rv;
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  berytus::RequestContextWithOperation ctx;
  rv = berytus::Utils_RequestContextWithOperationMetadata(
      mGlobal, mChannel, this, ctx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  if (!mChannel->Active()) {
    aRv.ThrowInvalidStateError("Channel has been closed!");
    return nullptr;
  }
  RefPtr<berytus::LoginCloseOpeationResult> res = mChannel->Agent().Login_CloseOpeation(ctx);
  res->Then(
    GetCurrentSerialEventTarget(), __func__,
    [outPromise]() {
      outPromise->MaybeResolve(JS::UndefinedValue());
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

RefPtr<BerytusLoginOperation::CreationPromise> BerytusLoginOperation::Create(
    JSContext* aCx,
    nsIGlobalObject* aGlobal,
    RefPtr<BerytusChannel>& aChannel,
    const BerytusOnboardingOptions& aOptions) {
  berytus::RequestContext reqCtx;
  nsresult rv;
  rv = berytus::Utils_RequestContext(aGlobal, aChannel, reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return CreationPromise::CreateAndReject(berytus::Failure(rv), __func__);
  }
  berytus::ApproveOperationArgs args;
  nsIDToCString oUuidString(nsID::GenerateUUID());
  nsString oId = NS_ConvertUTF8toUTF16(oUuidString.get());

  args.mOperation.mId.Assign(oId);
  args.mOperation.mType.SetAsPendingDeclaration();
  if (aOptions.mIntent.WasPassed()) {
    if (aOptions.mIntent.Value() == dom::BerytusOnboardingIntent::Unknown) {
      args.mOperation.mIntent.SetAsPendingDeclaration();
    } else if (aOptions.mIntent.Value() == dom::BerytusOnboardingIntent::Authenticate) {
      args.mOperation.mIntent.SetAsAuthenticate();
    } else {
      args.mOperation.mIntent.SetAsRegister();
    }
  }
  args.mOperation.mStatus.SetAsPending();
  if (aOptions.mRequiredUserAttributes.WasPassed()) {
    for (const auto& attr : aOptions.mRequiredUserAttributes.Value().Entries()) {
      // TODO(berytus): AgentProxy.cpp does not handle literal values/patterns.
      // Attribute ID is generated as Variant<nsString> but in reality it should
      // be a union of literal values, e.g. "username" | "nickname"
      using ThisVariant = decltype(berytus::RequestedUserAttribute::mId);
      ThisVariant vKey;
      vKey.Init(attr.mKey);
      args.mOperation.mRequestedUserAttributes.AppendElement(
          berytus::RequestedUserAttribute(
              std::move(vKey),
              bool(attr.mValue)));
    }
  }

  RefPtr<berytus::LoginApproveOperationResult> prom =
    aChannel->Agent().Login_ApproveOperation(reqCtx, args);
  return prom->Then(GetCurrentSerialEventTarget(), __func__,
    [aCx, aGlobal, aChannel, oId](const berytus::ELoginUserIntent& aIntent) -> RefPtr<BerytusLoginOperation::CreationPromise> {
      RefPtr<MozPromise<RefPtr<BerytusLoginOperation>, nsresult, false>::Private> nextProm;
      RefPtr<dom::BerytusLoginOperation> operation;
      if (aIntent.IsAuthenticate()) {
        return BerytusAccountAuthenticationOperation::CreateApproved(aGlobal, aChannel, oId)
          ->Then(GetCurrentSerialEventTarget(), __func__,
            [](const RefPtr<BerytusAccountAuthenticationOperation>& op) -> RefPtr<CreationPromise> {
              return CreationPromise::CreateAndResolve(static_cast<RefPtr<BerytusLoginOperation>>(op), __func__);
            }, [](const berytus::Failure& aFr) -> RefPtr<CreationPromise> {
              return CreationPromise::CreateAndReject(aFr, __func__);
            });
      }
      JSAutoRealm ar(aCx, aGlobal->GetGlobalJSObject());
      return BerytusAccountCreationOperation::CreateApproved(aCx, aGlobal, aChannel, oId)
        ->Then(GetCurrentSerialEventTarget(), __func__,
          [](const RefPtr<BerytusAccountCreationOperation>& op) -> RefPtr<CreationPromise> {
            return CreationPromise::CreateAndResolve(static_cast<RefPtr<BerytusLoginOperation>>(op), __func__);
          }, [](const berytus::Failure& aFr) -> RefPtr<CreationPromise> {
            return CreationPromise::CreateAndReject(aFr, __func__);
          });
    },
    [](const berytus::Failure& aFr) {
      return CreationPromise::CreateAndReject(aFr, __func__);
    }
  );
}

bool BerytusLoginOperation::Active() const {
  return mActive;
}

} // namespace mozilla::dom
