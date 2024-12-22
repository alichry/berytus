/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChannel.h"
#include "BerytusCryptoWebAppActor.h"
#include "BerytusSecretManagerActor.h"
#include "js/PropertyAndElement.h"
#include "js/Realm.h"
#include "js/Value.h"
#include "mozIBerytusPromptService.h"
#include "mozilla/Components.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/HoldDropJSObjects.h"
#include "mozilla/RefPtr.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusChannelBinding.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h" /* Needed for AddCallbacksWithCycleCollectedArgs */
#include "mozilla/dom/RootedDictionary.h"
#include "nsHashPropertyBag.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "mozilla/dom/BerytusAccountCreationOperation.h"

namespace mozilla::dom {


NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusChannel, (mGlobal, mWebAppActor, mSecretManagerActor, mKeyAgreementParams, mAgent), (mCachedConstraints))
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChannel)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChannel)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChannel)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChannel::BerytusChannel(
  nsIGlobalObject* aGlobal,
  BerytusChannelConstraints&& aConstraints,
  const RefPtr<BerytusWebAppActor>& aWebAppActor,
  const RefPtr<BerytusSecretManagerActor>& aSecretManagerActor,
  const RefPtr<BerytusKeyAgreementParameters>& aKeyAgreementParams,
  const RefPtr<mozilla::berytus::AgentProxy>& aAgent
) : mGlobal(aGlobal),
    mCachedConstraints(nullptr),
    // mConstraints(RootingCx()),
    mConstraints(std::move(aConstraints)),
    mWebAppActor(aWebAppActor),
    mSecretManagerActor(aSecretManagerActor),
    mKeyAgreementParams(aKeyAgreementParams),
    mAgent(aAgent)
{
  nsIDToCString uuidString(nsID::GenerateUUID());
  mId.Assign(NS_ConvertUTF8toUTF16(uuidString.get()));
  mozilla::HoldJSObjects(this);
}

BerytusChannel::~BerytusChannel()
{
  mozilla::DropJSObjects(this);
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

void BerytusChannel::GetConstraints(
    JSContext* aCx,
    JS::MutableHandle<JSObject*> aRetVal,
    ErrorResult& aRv) {
  if (!mCachedConstraints) {
    {
      JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());
      JS::Rooted<JS::Value> ctValJs(aCx);
      if (NS_WARN_IF(!mConstraints.ToObjectInternal(aCx, &ctValJs))) {
        aRv.Throw(NS_ERROR_FAILURE);
        return;
      }
      mCachedConstraints = ctValJs.toObjectOrNull();
    }
    MOZ_ASSERT(mCachedConstraints);
  }
  aRetVal.set(mCachedConstraints);
}

const BerytusChannelConstraints& BerytusChannel::Constraints() const {
  return mConstraints;
}


// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusWebAppActor> BerytusChannel::WebApp() const
{
  MOZ_ASSERT(mWebAppActor);
  return do_AddRef(mWebAppActor);
}

BerytusWebAppActor* BerytusChannel::GetWebAppActor() const {
  MOZ_ASSERT(mWebAppActor);
  return mWebAppActor;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusSecretManagerActor> BerytusChannel::GetSecretManager() const
{
  if (!mSecretManagerActor) {
    return nullptr;
  }
  return do_AddRef(mSecretManagerActor);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<BerytusKeyAgreementParameters> BerytusChannel::GetKeyAgreementParams() const
{
  if (!mKeyAgreementParams) {
    return nullptr;
  }
  return do_AddRef(mKeyAgreementParams);
}

const berytus::AgentProxy& BerytusChannel::Agent() const {
  MOZ_ASSERT(mAgent);
  return *mAgent;
}

already_AddRefed<Promise> BerytusChannel::Create(
    const GlobalObject& aGlobal,
    JSContext* aCx,
    const BerytusChannelOptions& aOptions,
    ErrorResult& aRv) {
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

  nsTArray<nsString> scmKeys;
  if (aOptions.mConstraints.WasPassed() && aOptions.mConstraints.Value().mSecretManagerPublicKey.WasPassed()) {
    for (const auto& key : aOptions.mConstraints.Value().mSecretManagerPublicKey.Value()) {
      scmKeys.AppendElement(key);
    }
  }
  rv = ps->PromptUsingPopupNotification(
    inner,
    webAppEd25519Key,
    scmKeys,
    aOptions.mConstraints.WasPassed() && aOptions.mConstraints.Value().mEnableEndToEndEncryption.WasPassed()
    ? aOptions.mConstraints.Value().mEnableEndToEndEncryption.Value()
    : false,
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

  JS::PersistentRooted<JS::Value> constraintsJs(aCx);
  constraintsJs.setNull();
  if (aOptions.mConstraints.WasPassed()
      && NS_WARN_IF(!aOptions.mConstraints.Value().ToObjectInternal(aCx, &constraintsJs))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  auto onResolve = [outPromise,
                              webAppActor = RefPtr<BerytusWebAppActor>(aOptions.mWebApp),
                              constraintsJs](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal)  {
    nsString selectedId;
    nsresult rv;
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
    rv = berytus::Utils_WebAppActorToVariant(webAppActor, reqArgs.mWebAppActor);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      outPromise->MaybeReject(rv);
      return;
    }
    RefPtr<berytus::ManagerGetSigningKeyResult> prom =
      proxy->Manager_GetSigningKey(reqCx, reqArgs);
    prom->Then(
      //aGlobal->SerialEventTarget(),
      GetCurrentSerialEventTarget(), __func__,
      [outPromise, aGlobal, aCx, webAppActor, proxy, constraintsJs](const nsString& scmEd25519Key) {
        RootedDictionary<BerytusChannelConstraints> ct(aCx); // RootingCx()
        {
          JSAutoRealm ar(aCx, aGlobal->GetGlobalJSObject());
          if (NS_WARN_IF(!ct.Init(aCx, constraintsJs))) {
            outPromise->MaybeReject(NS_ERROR_FAILURE);
            return;
          }
        }
        RefPtr<BerytusChannel> ch = new BerytusChannel(
          aGlobal,
          std::move(ct),
          webAppActor,
          new BerytusSecretManagerActor(aGlobal, scmEd25519Key),
          nullptr,
          proxy
        );
        outPromise->MaybeResolve(ch);
      }, [outPromise](const berytus::Failure& aRs) {
        outPromise->MaybeReject(aRs.ToErrorResult());
      });
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    berytus::Failure fr;
    berytus::Failure::FromJSVal(aCx, aValue, fr);
    outPromise->MaybeReject(fr.ToErrorResult());
  };
  selectPromise->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsGlobal);
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::Close(ErrorResult& aRv)
{
  MOZ_ASSERT(false, "BerytusChannel::Close() not impld");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}

already_AddRefed<Promise> BerytusChannel::Login(JSContext* aCx, const BerytusOnboardingOptions& aOptions, ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<BerytusChannel> ch = this;
  BerytusLoginOperation::Create(aCx, mGlobal, ch, aOptions)
      ->Then(
        GetCurrentSerialEventTarget(), __func__,
        [outPromise](const RefPtr<BerytusLoginOperation>& aOperation) {
          outPromise->MaybeResolve(aOperation);
        },
        [outPromise](const berytus::Failure& aFr) {
          outPromise->MaybeReject(aFr.ToErrorResult());
        }
      );
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::PrepareKeyAgreementParameters(const nsAString& webAppX25519PublicKey, ErrorResult& aRv)
{
  MOZ_ASSERT(false, "BerytusChannel::PrepareKeyAgreementParameters");
  return nullptr;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusChannel::EnableEndToEndEncryption(const ArrayBuffer& keyAgreementSignature, ErrorResult& aRv)
{
  MOZ_ASSERT(false, "BerytusChannel::EnableEndToEndEncryption");
  return nullptr;
}
void BerytusChannel::GetID(nsString& aRv) const {
  aRv.Assign(mId);
}

} // namespace mozilla::dom
