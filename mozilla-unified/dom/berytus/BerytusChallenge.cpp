/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChallenge.h"
#include "ErrorList.h"
#include "js/RootingAPI.h"
#include "js/TypeDecls.h"
#include "js/Value.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/HoldDropJSObjects.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusChallengeBinding.h"
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusChallenge, (mGlobal, mChannel, mOperation), (mCachedParameters))
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChallenge)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChallenge)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChallenge)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChallenge::BerytusChallenge(nsIGlobalObject* aGlobal,
                                   const BerytusChallengeType& aType,
                                   const nsAString& aID) :
                                   mGlobal(aGlobal),
                                   mChannel(nullptr),
                                   mOperation(nullptr),
                                   mType(aType),
                                   mID(aID),
                                   mActive(false),
                                   mCachedParameters(nullptr)
{
  mozilla::HoldJSObjects(this);
    // Add |MOZ_COUNT_CTOR(BerytusChallenge);| for a non-refcounted object.
}

BerytusChallenge::~BerytusChallenge()
{
  mozilla::DropJSObjects(this);
    // Add |MOZ_COUNT_DTOR(BerytusChallenge);| for a non-refcounted object.
}

nsIGlobalObject* BerytusChallenge::GetParentObject() const { return mGlobal; }

bool BerytusChallenge::Connected() const {
  return mChannel && mOperation;
}

void BerytusChallenge::Connect(const RefPtr<BerytusChannel>& aChannel,
                               const RefPtr<BerytusLoginOperation>& aOperation) {
  mChannel = aChannel;
  mOperation = aOperation;
  mActive = true;
}

BerytusChannel* BerytusChallenge::Channel() {
  MOZ_ASSERT(mChannel);
  return mChannel;
}

BerytusLoginOperation* BerytusChallenge::Operation() {
  MOZ_ASSERT(mOperation);
  return mOperation;
}

bool BerytusChallenge::Active() { return mActive; }

void BerytusChallenge::GetId(nsAString& aRetVal) {
  aRetVal.Assign(mID);
}

BerytusChallengeType BerytusChallenge::Type() const {
  return mType;
}

void BerytusChallenge::GetParameters(JSContext* aCx,
                              JS::MutableHandle<JSObject*> aRetVal,
                              ErrorResult& aRv) {
  CacheParameters(aCx, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  aRetVal.set(mCachedParameters); // Object or null
}

already_AddRefed<Promise> BerytusChallenge::SendMessageRaw(
    JSContext* aCx,
    const nsString& aMessageName,
    JS::Handle<JS::Value> aMessagePayload,
    ErrorResult& aRv) {
  if (!Connected()) {
    aRv.ThrowInvalidStateError("Challenge is not connected to a secret manager.");
    return nullptr;
  }
  if (NS_WARN_IF(!Channel()->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  berytus::AgentProxy& agent = Channel()->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  berytus::RequestContextWithLoginOperation reqCtx;
  nsresult rv = berytus::Utils_RequestContextWithLoginOperationMetadata(mGlobal, Channel(), Operation(), nullptr, reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  JS::Rooted<JS::Value> msg(aCx);
  BuildChallengeMessage(aCx, aMessageName, aMessagePayload, &msg, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<Promise> prom= agent.CallSendQuery(aCx, u"accountAuthentication"_ns,
                             u"respondToChallengeMessage"_ns, reqCtx, msg, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return MaybeCatchBerytusFailure(prom, aRv);
}

template <typename P>
already_AddRefed<Promise> BerytusChallenge::SendMessageRaw(
    JSContext* aCx,
    const nsString& aMessageName,
    P& aMessagePayload,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payloadJs(aCx);
  if (NS_WARN_IF(!berytus::ToJSVal(aCx, aMessagePayload, payloadJs))) {
    aRv.Throw(NS_ERROR_INVALID_ARG);
    return nullptr;
  }
  return SendMessageRaw(aCx, aMessageName, JS::HandleValue(payloadJs), aRv);
}

template <typename T>
RefPtr<BerytusChallenge::SendMessageResult<T>> BerytusChallenge::SendMessage(
    JSContext* aCx,
    const nsString& aMessageName,
    JS::Handle<JS::Value> aMessagePayload,
    ErrorResult& aRv) {
  RefPtr<typename SendMessageResult<T>::Private> outPromise = new typename SendMessageResult<T>::Private(__func__);
  RefPtr<Promise> prom = SendMessageRaw(aCx, aMessageName, aMessagePayload, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                                ErrorResult& aRv,
                                const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    T out;
    if (NS_WARN_IF(!(berytus::FromJSVal<T>(aCx, aValue, out)))) {
      outPromise->Reject(berytus::Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    return Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  MaybeCatchBerytusFailure(prom, aRv, std::move(onResolve));
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return outPromise;
}

template <typename T, typename P>
RefPtr<BerytusChallenge::SendMessageResult<T>> BerytusChallenge::SendMessage(
    JSContext* aCx,
    const nsString& aMessageName,
    P& aMessagePayload,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payloadJs(aCx);
  if (NS_WARN_IF(!berytus::ToJSVal(aCx, aMessagePayload, payloadJs))) {
    aRv.Throw(NS_ERROR_INVALID_ARG);
    return nullptr;
  }
  return SendMessage(aCx, aMessageName, JS::HandleValue(payloadJs), aRv);
}

already_AddRefed<Promise> BerytusChallenge::Seal(JSContext* aCx, ErrorResult& aRv) {
  if (!Connected()) {
    aRv.ThrowInvalidStateError("Challenge is not connected to a secret manager.");
    return nullptr;
  }
  if (NS_WARN_IF(!Channel()->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  berytus::AgentProxy& agent = Channel()->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  berytus::RequestContextWithOperation reqCtx;
  nsresult rv = berytus::Utils_RequestContextWithOperationMetadata(mGlobal, Channel(), Operation(), reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::Value> info(aCx);
  BuildChallengeInfo(aCx, &info, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", info))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  JS::Rooted<JS::Value> args(aCx, JS::ObjectValue(*obj));
  //agent.AccountAuthentication_CloseChallenge(reqCtx, args);
  RefPtr<Promise> promise = agent.CallSendQuery(aCx, u"accountAuthentication"_ns,
                             u"closeChallenge"_ns, reqCtx, args, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  auto onResolve = [this](JSContext* aCx, JS::Handle<JS::Value> aValue,
                                ErrorResult& aRv,
                                const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    mActive = false;
    return Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  return MaybeCatchBerytusFailure(promise, aRv, std::move(onResolve));
}

already_AddRefed<Promise> BerytusChallenge::Abort(
    JSContext* aCx,
    const BerytusChallengeAbortionCode& aCode,
    ErrorResult& aRv) {
  if (!Connected()) {
    aRv.ThrowInvalidStateError("Challenge is not connected to a secret manager.");
    return nullptr;
  }
  if (NS_WARN_IF(!Channel()->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  berytus::AgentProxy& agent = Channel()->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  berytus::RequestContextWithOperation reqCtx;
  nsresult rv = berytus::Utils_RequestContextWithOperationMetadata(mGlobal, Channel(), Operation(), reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::Value> info(aCx);
  BuildChallengeInfo(aCx, &info, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", info))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(static_cast<uint8_t>(aCode) >= sizeof(binding_detail::EnumStrings<BerytusChallengeAbortionCode>::Values) / sizeof(nsLiteralCString))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  const auto& codeStr = binding_detail::EnumStrings<BerytusChallengeAbortionCode>::Values[static_cast<uint8_t>(aCode)];
    JSString* reasonJsStr = JS_NewStringCopyN(
      aCx,
      codeStr.BeginReading(),
      codeStr.Length());
  if (NS_WARN_IF(!reasonJsStr)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    return nullptr;
  }
  JS::Rooted<JS::Value> reason(aCx, JS::StringValue(reasonJsStr));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", reason))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  JS::Rooted<JS::Value> args(aCx, JS::ObjectValue(*obj));
  RefPtr<Promise> promise = agent.CallSendQuery(aCx, u"accountAuthentication"_ns,
                             u"abortChallenge"_ns, reqCtx, args, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  auto onResolve = [this](JSContext* aCx, JS::Handle<JS::Value> aValue,
                                ErrorResult& aRv,
                                const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    mActive = false;
    return Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  return MaybeCatchBerytusFailure(promise, aRv, std::move(onResolve));
}

already_AddRefed<Promise> BerytusChallenge::AbortWithUserInterruptError(
    JSContext* aCx,
    ErrorResult& aRv) {
  return Abort(aCx, BerytusChallengeAbortionCode::UserInterrupt, aRv);
}

already_AddRefed<Promise> BerytusChallenge::AbortWithGenericWebAppFailureError(
    JSContext* aCx,
    ErrorResult& aRv) {
  return Abort(aCx, BerytusChallengeAbortionCode::GenericWebAppFailure, aRv);
}

void BerytusChallenge::BuildChallengeInfo(JSContext* aCx, JS::MutableHandle<JS::Value> aRetVal, ErrorResult& aRv) {
  CacheParameters(aCx, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::Value> id(aCx);
  if (NS_WARN_IF(!berytus::ToJSVal<nsString>(aCx, mID, &id))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", id))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  size_t types = sizeof(binding_detail::EnumStrings<BerytusChallengeType>::Values) / sizeof(nsLiteralCString);
  if (NS_WARN_IF(static_cast<uint8_t>(mType) >= types)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  const auto& typeStr = binding_detail::EnumStrings<BerytusChallengeType>::Values[static_cast<uint8_t>(mType)];
  JSString* typeJsStr = JS_NewStringCopyN(
      aCx,
      typeStr.BeginReading(),
      typeStr.Length());
  // TODO(berytus): In AgentProxy.h, check whether JSString* allocs are ok
  if (NS_WARN_IF(!typeJsStr)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    return;
  }
  JS::Rooted<JS::Value> type(aCx, JS::StringValue(typeJsStr));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", type))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  JS::Rooted<JS::Value> params(aCx, JS::ObjectOrNullValue(mCachedParameters));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", params))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  aRetVal.set(JS::ObjectValue(*obj));
}

already_AddRefed<Promise> BerytusChallenge::Send(
    JSContext *aCx,
    const BerytusChallengeMessageRequestDefinition& aInput,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx, aInput.mRequest);
  return SendMessageRaw(aCx, aInput.mName, JS::HandleValue(payload), aRv);
}

void BerytusChallenge::BuildChallengeMessage(
    JSContext* aCx,
    const nsString& aMessageName,
    JS::Handle<JS::Value> aPayload,
    JS::MutableHandle<JS::Value> aRetVal,
    ErrorResult& aRv) {
  JS::Rooted<JSObject*> msg(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::Value> info(aCx);
  BuildChallengeInfo(aCx, &info, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, msg, "challenge", info))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, msg, "payload", aPayload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  JSString* msgNameJsStr = JS_NewUCStringCopyN(
      aCx,
      aMessageName.get(),
      aMessageName.Length());
  if (NS_WARN_IF(!msgNameJsStr)) {
    aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    return;
  }
  JS::Rooted<JS::Value> msgName(aCx, JS::StringValue(msgNameJsStr));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msg, "name", msgName))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  aRetVal.set(JS::ObjectValue(*msg));
}

template <typename ResolveCallback>
already_AddRefed<Promise> BerytusChallenge::MaybeCatchBerytusFailure(
    const RefPtr<Promise>& aPromise,
    ErrorResult& aRv,
    ResolveCallback&& aOnResolve) {
  auto onReject = [](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    berytus::Failure fr;
    FromJSVal(aCx, aValue, fr);
    ErrorResult err = fr.ToErrorResult();
    return mozilla::dom::Promise::CreateRejectedWithErrorResult(aGlobal, err);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    aPromise->ThenCatchWithCycleCollectedArgs(
      std::forward<ResolveCallback>(aOnResolve),
      std::move(onReject),
      nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    aRv.Throw(thenRes.unwrapErr());
    return nullptr;
  }
  return thenRes.unwrap().forget();
}

already_AddRefed<Promise> BerytusChallenge::MaybeCatchBerytusFailure(
    const RefPtr<Promise>& aPromise,
    ErrorResult& aRv) {
  auto onResolve = [](JSContext* aCx,
                                JS::Handle<JS::Value> aValue,
                                ErrorResult& aRv,
                                const nsCOMPtr<nsIGlobalObject>& aGlobal) -> already_AddRefed<Promise> {
    RefPtr<Promise> newPromise = Promise::Create(aGlobal, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return nullptr;
    }
    // AutoRealm needed?
    newPromise->MaybeResolve(aValue);
    return newPromise.forget();
  };
  return MaybeCatchBerytusFailure(aPromise, aRv, std::move(onResolve));
}

} // namespace mozilla::dom
