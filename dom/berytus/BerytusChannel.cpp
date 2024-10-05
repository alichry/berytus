/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChannel.h"
#include "BerytusCryptoWebAppActor.h"
#include "BerytusSecretManagerActor.h"
#include "js/Value.h"
#include "mozIBerytusPromptService.h"
#include "mozilla/Components.h"
#include "mozilla/RefPtr.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusChannelBinding.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h" /* Needed for AddCallbacksWithCycleCollectedArgs */
#include "nsHashPropertyBag.h"
#include "mozilla/berytus/AgentProxyUtils.h"

namespace mozilla::dom {


NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusChannel, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChannel)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChannel)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChannel)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChannel::BerytusChannel(
  nsIGlobalObject* aGlobal,
  Maybe<BerytusChannelConstraints>&& aConstraints,
  const RefPtr<BerytusWebAppActor>& aWebAppActor,
  const RefPtr<BerytusSecretManagerActor>& aSecretManagerActor,
  const RefPtr<BerytusKeyAgreementParameters>& aKeyAgreementParams
) : mGlobal(aGlobal), mConstraints(std::move(aConstraints)), mWebAppActor(aWebAppActor), mSecretManagerActor(aSecretManagerActor), mKeyAgreementParams(aKeyAgreementParams)
{
    // Add |MOZ_COUNT_CTOR(BerytusChannel);| for a non-refcounted object.
}

BerytusChannel::~BerytusChannel()
{
    // Add |MOZ_COUNT_DTOR(BerytusChannel);| for a non-refcounted object.
}

JSObject*
BerytusChannel::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusChannel_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusChannel::GetParentObject() const { return mGlobal; }

bool BerytusChannel::Active() const
{
  return mActive;
}

void BerytusChannel::GetConstraints(JSContext* aCx, JS::MutableHandle<JSObject*> aRetVal) {
  if (!mConstraints) {
    return;
  }
  JS::Rooted<JS::Value> val(aCx);
  mConstraints->ToObjectInternal(aCx, &val);
  aRetVal.set(&val.toObject());
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusWebAppActor> BerytusChannel::WebApp() const
{
  return do_AddRef(mWebAppActor);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusSecretManagerActor> BerytusChannel::GetSecretManager() const
{
    if (! mSecretManagerActor) {
    return nullptr;
  }
  return do_AddRef(mSecretManagerActor);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusKeyAgreementParameters> BerytusChannel::GetKeyAgreementParams() const
{
  NS_WARN_IF(true);
  return nullptr;
}

/*
BerytusChannel.create({
  webApp: new BerytusAnonymousWebAppActor(),
  secretManagerPublicKey: ['A', 'B'],
  enableEndToEndEncryption: true,
  account: {
     identity: { 'username': 'ali' },
     schemaVersion: 1,
     category: "user"
  }
})
*/
already_AddRefed<Promise> BerytusChannel::Create(const GlobalObject& aGlobal, const BerytusChannelOptions& aOptions, ErrorResult& aRv) {
  nsresult rv;
  nsCOMPtr<nsIGlobalObject> nsGlobal =
      do_QueryInterface(aGlobal.GetAsSupports());
  if (NS_WARN_IF(!nsGlobal)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<Promise> outPromise = Promise::Create(nsGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  nsCOMPtr<mozIBerytusPromptService> ps =
    mozilla::components::BerytusPromptServiceProxy::Create(&rv);

  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  nsPIDOMWindowInner* inner = nsGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  Promise* selectPromise;
  nsCOMPtr<nsIWritablePropertyBag2> identity = new nsHashPropertyBag();
  nsString webAppEd25519Key;
  if (aOptions.mWebApp->Type() == BerytusWebAppActorType::CryptoActor) {
    static_cast<BerytusCryptoWebAppActor*>(aOptions.mWebApp.get())->GetEd25519Key(webAppEd25519Key);
  }
  if (aOptions.mConstraints.WasPassed() &&
      aOptions.mConstraints.Value().mAccount.WasPassed() &&
      aOptions.mConstraints.Value().mAccount.Value().mIdentity.WasPassed()
  ) {
    const Record<nsString, nsString>& identityRecord =
      aOptions.mConstraints.Value().mAccount.Value().mIdentity.Value();
    for (const auto& entry : identityRecord.Entries()) {
      identity->SetPropertyAsAString(entry.mKey, entry.mValue);
    }
  }
  // TODO(berytus): Add Secret Manager Public Key filter
  rv = ps->PromptUsingPopupNotification(
    inner,
    webAppEd25519Key,
    aOptions.mConstraints.WasPassed() && aOptions.mConstraints.Value().mAccount.WasPassed() && aOptions.mConstraints.Value().mAccount.Value().mCategory.WasPassed()
    ? aOptions.mConstraints.Value().mAccount.Value().mCategory.Value()
    : u""_ns,
    aOptions.mConstraints.WasPassed() && aOptions.mConstraints.Value().mAccount.WasPassed() && aOptions.mConstraints.Value().mAccount.Value().mSchemaVersion.WasPassed()
    ? aOptions.mConstraints.Value().mAccount.Value().mSchemaVersion.Value()
    : 0,
    identity.get(),
    &selectPromise
  );
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  Maybe<BerytusChannelConstraints> constraints;
  if (aOptions.mConstraints.WasPassed()) {
    // let's clone it.
    constraints.emplace();
    JSContext* cx = aGlobal.Context();
    JS::Rooted<JS::Value> ctJs(cx);
    if (NS_WARN_IF(!aOptions.mConstraints.Value().ToObjectInternal(cx, &ctJs))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
    constraints->Init(cx, ctJs);
  }
  auto onResolve = [outPromise, webAppActor = RefPtr<BerytusWebAppActor>(aOptions.mWebApp), &constraints](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    nsString selectedId;
    if (NS_WARN_IF(!mozilla::berytus::StringFromJSVal(aCx, aValue, selectedId))) {
      outPromise->MaybeReject(NS_ERROR_FAILURE);
      return;
    }
    RefPtr<mozilla::berytus::AgentProxy> proxy = new mozilla::berytus::AgentProxy(aGlobal, selectedId);
    berytus::PreliminaryRequestContext reqCx;
    if (NS_WARN_IF(NS_FAILED(berytus::Utils_PreliminaryRequestContext(aGlobal, reqCx)))) {
      outPromise->MaybeReject(NS_ERROR_FAILURE);
      return;
    }
    berytus::GetSigningKeyArgs reqArgs;
    reqArgs.mWebAppActor = berytus::Utils_WebAppActorToVariant(webAppActor);
    RefPtr<berytus::ManagerGetSigningKeyResult> prom =
      proxy->Manager_GetSigningKey(reqCx, reqArgs);
    prom->Then(
      //aGlobal->SerialEventTarget(),
      GetCurrentSerialEventTarget(), __func__,
      [outPromise, aGlobal, webAppActor, &constraints](const nsString& scmEd25519Key) {
        RefPtr<BerytusChannel> ch = new BerytusChannel(
          aGlobal,
          Maybe<BerytusChannelConstraints>(),
          webAppActor,
          new BerytusSecretManagerActor(aGlobal, scmEd25519Key),
          nullptr
        );
        outPromise->MaybeResolve(ch);
      }, [outPromise](const berytus::Failure& aRs) {
        outPromise->MaybeReject(aRs.ToErrorResult());
      });

  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {

    outPromise->MaybeReject(NS_ERROR_FAILURE);
  };
  selectPromise->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsGlobal);
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::Close(ErrorResult& aRv)
{
  NS_WARN_IF(true);
  return nullptr;
}

already_AddRefed<Promise> BerytusChannel::PrepareKeyAgreementParameters(const nsAString& webAppX25519PublicKey, ErrorResult& aRv)
{
  NS_WARN_IF(true);
  return nullptr;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusChannel::EnableEndToEndEncryption(const ArrayBuffer& keyAgreementSignature, ErrorResult& aRv)
{
  NS_WARN_IF(true);
  return nullptr;
}

} // namespace mozilla::dom
