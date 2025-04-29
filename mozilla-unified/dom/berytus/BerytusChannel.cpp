/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

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
#include "mozilla/dom/BerytusKeyAgreementParameters.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/BerytusX509Extension.h"
#include "mozilla/dom/BindingUtils.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h" /* Needed for AddCallbacksWithCycleCollectedArgs */
#include "mozilla/dom/RootedDictionary.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsHashPropertyBag.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "mozilla/dom/BerytusAccountCreationOperation.h"
#include "nsString.h"
#include "nsTHashSet.h"
#include "nsTHashMap.h"
#include "mozilla/dom/Document.h" // inner->GetDoc()

namespace mozilla::dom {

NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusChannel, (mGlobal, mWebAppActor, mSecretManagerActor, mKeyAgreementParams, mAgent), (mCachedConstraints))
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChannel)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChannel)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChannel)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

/**
 * A few notes regarding the "at most one channel
 * is active under a browsing context/window" implementation:
 * AFAIK, present Firefox design enables a child process to
 * manage multiple browsing contexts. Thus, within each child process,
 * we maintain a set of inner window ids where channels are active
 * or in the process of being created. If one process manages one
 * browsing context/window, there would not be a need for a set.
 *
 * Also, NOTE(berytus): We assume that in the case when an
 * inner window is destroyed, any BerytusChannel instance should
 * have been deconstructed a priori. Otherwise, an observer for
 * destruction of deleted inner windows should be setup to 
 * clean up entries from the set. TODO(berytus): Verify the above.
 */
nsTHashSet<uint64_t> BerytusChannel::mRegisteredWindows = nsTHashSet<uint64_t>();

BerytusChannel::BerytusChannel(
  nsIGlobalObject* aGlobal,
  BerytusChannelConstraints&& aConstraints,
  const RefPtr<BerytusWebAppActor>& aWebAppActor,
  const RefPtr<BerytusSecretManagerActor>& aSecretManagerActor,
  const RefPtr<BerytusX509Extension>& aCertExt,
  const RefPtr<BerytusKeyAgreementParameters>& aKeyAgreementParams,
  const RefPtr<mozilla::berytus::OwnedAgentProxy>& aAgent
) : mGlobal(aGlobal),
    mCachedConstraints(nullptr),
    // mConstraints(RootingCx()),
    mConstraints(std::move(aConstraints)),
    mWebAppActor(aWebAppActor),
    mSecretManagerActor(aSecretManagerActor),
    mCertExtension(aCertExt),
    mKeyAgreementParams(aKeyAgreementParams),
    mAgent(aAgent)
{
  nsIDToCString uuidString(nsID::GenerateUUID());
  mId.Assign(NS_ConvertUTF8toUTF16(uuidString.get()));
  mozilla::HoldJSObjects(this);
  MOZ_ASSERT(aGlobal);
  nsPIDOMWindowInner* inner = aGlobal->GetAsInnerWindow();
  MOZ_ASSERT(inner);
  MOZ_ASSERT(!CanRegisterInWindow(inner));
  mInnerWindowId = inner->WindowID();
}

BerytusChannel::~BerytusChannel()
{
  mozilla::DropJSObjects(this);
  UnregisterInWindow(mInnerWindowId);
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

const BerytusWebAppActor* BerytusChannel::GetWebAppActor() const {
  MOZ_ASSERT(mWebAppActor);
  return mWebAppActor;
}

const BerytusSecretManagerActor* BerytusChannel::GetSecretManagerActor() const {
  MOZ_ASSERT(mSecretManagerActor);
  return mSecretManagerActor;
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

berytus::AgentProxy& BerytusChannel::Agent() const {
  MOZ_ASSERT(mAgent);
  return *mAgent;
}

bool BerytusChannel::RegisterInWindow(nsPIDOMWindowInner* aInner) {
  MOZ_ASSERT(aInner);
  if (mRegisteredWindows.Contains(aInner->WindowID())) {
    return false;
  }
  mRegisteredWindows.Insert(aInner->WindowID());
  return true;
}

void BerytusChannel::UnregisterInWindow(const uint64_t& aInnerWindowId) {
  mRegisteredWindows.Remove(aInnerWindowId);
}

bool BerytusChannel::CanRegisterInWindow(nsPIDOMWindowInner* aInner) {
  // NOTE(berytus): Assuming only the main thread can construct objects.
  return !mRegisteredWindows.Contains(aInner->WindowID());
}


already_AddRefed<Promise> BerytusChannel::Create(
  const GlobalObject& aGlobal,
  JSContext* aCx,
  const BerytusChannelOptions& aOptions,
  ErrorResult& aRv) {
  return CreateGuard(aGlobal, aCx, aOptions, aRv);
}

already_AddRefed<Promise> BerytusChannel::CreateGuard(
  const GlobalObject& aGlobal,
  JSContext* aCx,
  const BerytusChannelOptions& aOptions,
  ErrorResult& aRv) {
  nsCOMPtr<nsIGlobalObject> nsGlobal =
      do_QueryInterface(aGlobal.GetAsSupports());
  if (NS_WARN_IF(!nsGlobal)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  nsPIDOMWindowInner* innerWindow = nsGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!innerWindow)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!CanRegisterInWindow(innerWindow)) {
    aRv.ThrowInvalidStateError("Channel cannot be created as another one is active in the same window.");
    return nullptr;
  }
  uint64_t innWinId = innerWindow->WindowID();
  MOZ_ASSERT(RegisterInWindow(innerWindow));
  RefPtr<Promise> res = CreateInner(nsGlobal, aCx, aOptions, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    UnregisterInWindow(innWinId);
    return nullptr;
  }
  res->AddCallbacksWithCycleCollectedArgs([](JSContext* aCx, JS::Handle<JS::Value> aValue, ErrorResult& aRv){},
                               [innWinId](JSContext* aCx, JS::Handle<JS::Value> aValue, ErrorResult& aRv) {
      BerytusChannel::UnregisterInWindow(innWinId);
    });
  return res.forget();
}

already_AddRefed<Promise> BerytusChannel::CreateInner(
    const nsCOMPtr<nsIGlobalObject>& aGlobal,
    JSContext* aCx,
    const BerytusChannelOptions& aOptions,
    ErrorResult& aRv) {
  nsresult rv;
  RefPtr<Promise> outPromise = Promise::Create(aGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  nsCOMPtr<mozIBerytusPromptService> ps =
    mozilla::components::BerytusPromptServiceProxy::Create(&rv);

  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  nsPIDOMWindowInner* inner = aGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  Promise* selectPromise;
  nsCOMPtr<nsIWritablePropertyBag2> identity = new nsHashPropertyBag();
  nsString webAppEd25519Key;
  RefPtr<BerytusX509Extension> certExt = nullptr;
  if (aOptions.mWebApp->Type() == BerytusWebAppActorType::CryptoActor) {
    static_cast<BerytusCryptoWebAppActor*>(aOptions.mWebApp.get())->GetEd25519Key(webAppEd25519Key);
    certExt = BerytusX509Extension::Create(inner, rv);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.ThrowInvalidStateError("Error processing certificate extension");
      return nullptr;
    }
    bool isAllowed;
    rv = certExt->IsAllowed(NS_ConvertUTF16toUTF8(webAppEd25519Key),
                             inner->GetDoc()->GetOriginalURI(),
                             isAllowed);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.ThrowInvalidStateError("Error processing certificate extension");
      return nullptr;
    }
    if (!isAllowed) {
      aRv.ThrowNotAllowedError("Web app signing key cannot be used in this document.");
      return nullptr;
    }
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

  // NOTE(berytus): Have I thought of manually copying aOptions.mConstraints
  // 's members to a new locally constructed BerytusChannelConstraints?
  // This could be moved into the lambda perhaps, but lambda needs to copy it...
  // Is RootedDictionary<...> copyable?
  JS::PersistentRooted<JS::Value> constraintsJs(aCx);
  constraintsJs.setNull();
  if (aOptions.mConstraints.WasPassed()
      && NS_WARN_IF(!aOptions.mConstraints.Value().ToObjectInternal(aCx, &constraintsJs))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  auto onResolve = [outPromise,
                              webAppActor = RefPtr<BerytusWebAppActor>(aOptions.mWebApp),
                              certExt,
                              constraintsJs](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal)  {
    nsString selectedId;
    nsresult rv;
    if (NS_WARN_IF(!mozilla::berytus::FromJSVal(aCx, aValue, selectedId))) {
      outPromise->MaybeReject(NS_ERROR_FAILURE);
      return;
    }
    RefPtr<mozilla::berytus::OwnedAgentProxy> proxy = new mozilla::berytus::OwnedAgentProxy(aGlobal, selectedId);
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
      [outPromise, aGlobal, aCx, webAppActor, certExt, proxy, constraintsJs](const nsString& scmEd25519Key) {
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
          certExt,
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
    FromJSVal(aCx, aValue, fr);
    outPromise->MaybeReject(fr.ToErrorResult());
  };
  selectPromise->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), aGlobal);
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::Close(ErrorResult& aRv)
{
  if (!mActive) {
    aRv.ThrowInvalidStateError("Channel already closed.");
    return nullptr;
  }
  MOZ_RELEASE_ASSERT(!mAgent->IsDisabled());
  berytus::RequestContext reqCtx;
  nsresult rv = berytus::Utils_RequestContext(mGlobal, this, reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<berytus::ChannelCloseChannelResult> prom =
    mAgent->Channel_CloseChannel(reqCtx);
  prom->Then(
    GetCurrentSerialEventTarget(), __func__,
    [outPromise, this](void* aIgnore) {
      mActive = false;
      UnregisterInWindow(mInnerWindowId);
      mAgent->Disable();
      outPromise->MaybeResolveWithUndefined();
    }, [outPromise](const berytus::Failure& aRs) {
      outPromise->MaybeReject(aRs.ToErrorResult());
    });
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::Login(JSContext* aCx, const BerytusOnboardingOptions& aOptions, ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<BerytusChannel> ch = this;
  RefPtr<BerytusLoginOperation::CreationPromise> prom =
    BerytusLoginOperation::Create(aCx, mGlobal, ch, aOptions);
  prom->Then(
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

already_AddRefed<Promise> BerytusChannel::PrepareKeyAgreementParameters(
    const nsAString& aWebAppX25519PublicKey,
    ErrorResult& aRv)
{
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  RefPtr<BerytusKeyAgreementParameters> pams = BerytusKeyAgreementParameters::Create(this, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  pams->GetExchange()->SetWebApp(aWebAppX25519PublicKey);
  outPromise->MaybeResolve(pams);
  return outPromise.forget();
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
