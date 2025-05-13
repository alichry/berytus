/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "BerytusCryptoWebAppActor.h"
#include "BerytusSecretManagerActor.h"
#include "js/PropertyAndElement.h"
#include "js/Realm.h"
#include "js/RootingAPI.h"
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
#include "mozilla/dom/ToJSValue.h"
#include "mozilla/dom/TypedArray.h"
#include "nsCOMPtr.h"
#include "nsDebug.h"
#include "nsHashPropertyBag.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"
#include "mozilla/dom/BerytusAccountCreationOperation.h"
#include "nsString.h"
#include "nsStringFwd.h"
#include "nsTHashSet.h"
#include "nsTHashMap.h"
#include "mozilla/dom/Document.h" // inner->GetDoc()
#include "nsThreadUtils.h"

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
  const nsString& aChannelId,
  BerytusChannelConstraints&& aConstraints,
  const RefPtr<BerytusWebAppActor>& aWebAppActor,
  const RefPtr<BerytusSecretManagerActor>& aSecretManagerActor,
  const RefPtr<BerytusX509Extension>& aCertExt,
  const RefPtr<BerytusKeyAgreementParameters>& aKeyAgreementParams,
  const RefPtr<mozilla::berytus::OwnedAgentProxy>& aAgent
) : mId(aChannelId),
    mGlobal(aGlobal),
    mCachedConstraints(nullptr),
    // mConstraints(RootingCx()),
    mConstraints(std::move(aConstraints)),
    mWebAppActor(aWebAppActor),
    mSecretManagerActor(aSecretManagerActor),
    mCertExtension(aCertExt),
    mKeyAgreementParams(aKeyAgreementParams),
    mAgent(aAgent)
{
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

const nsString& BerytusChannel::ID() const {
  return mId;
}

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

bool BerytusChannel::E2EEEnabled() const {
  return mE2EE;
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
                              constraintsJs](JSContext* aCx,
                                             JS::Handle<JS::Value> aValue,
                                             ErrorResult& aRv,
                                             const nsCOMPtr<nsIGlobalObject>& aGlobal)  {
    nsString selectedId;
    if (NS_WARN_IF(!mozilla::berytus::FromJSVal(aCx, aValue, selectedId))) {
      outPromise->MaybeReject(NS_ERROR_FAILURE);
      return;
    }
    RefPtr<CreationPromise> prom = CreateForScm(aGlobal,
                                                aCx,
                                                selectedId,
                                                webAppActor,
                                                constraintsJs,
                                                certExt);
    prom->Then(
      GetCurrentSerialEventTarget(), __func__,
      [outPromise](const RefPtr<BerytusChannel>& aChannel) {
        outPromise->MaybeResolve(aChannel);
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
RefPtr<BerytusChannel::CreationPromise> BerytusChannel::CreateForScm(
  nsIGlobalObject* aGlobal,
  JSContext* aCx,
  const nsString& aSecretManagerId,
  const RefPtr<BerytusWebAppActor>& aWebAppActor,
  JS::PersistentRooted<JS::Value> aConstraints,
  const RefPtr<BerytusX509Extension>& aCertExt
) {
  nsresult rv;
  nsIDToCString uuidString(nsID::GenerateUUID());
  nsString channelId = NS_ConvertUTF8toUTF16(uuidString.get());
  RefPtr<mozilla::berytus::OwnedAgentProxy> proxy =
    new mozilla::berytus::OwnedAgentProxy(aGlobal,
                                          aSecretManagerId);
  berytus::PreliminaryRequestContext reqCx;
  rv = berytus::Utils_PreliminaryRequestContext(aGlobal, reqCx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return CreationPromise::CreateAndReject(
        berytus::Failure(rv),
        __func__);
  }
  berytus::GetSigningKeyArgs reqArgs;
  rv = berytus::Utils_WebAppActorToVariant(aWebAppActor,
                                           reqArgs.mWebAppActor);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return CreationPromise::CreateAndReject(
      berytus::Failure(rv),
      __func__);
  }
  RefPtr<berytus::ManagerGetSigningKeyResult> prom =
    proxy->Manager_GetSigningKey(reqCx, reqArgs);
  //JS::PersistentRooted<JS::Value> persistentCt(aCx, aConstraints);
  return prom->Then(
    GetCurrentSerialEventTarget(), __func__,
    [channelId, reqCx = std::move(reqCx), aGlobal, aCx, aWebAppActor, aCertExt, proxy, persistentCt = aConstraints](const nsString& scmEd25519Key) -> RefPtr<CreationPromise> {
      RootedDictionary<BerytusChannelConstraints> ct(aCx);
      {
        JSAutoRealm ar(aCx, aGlobal->GetGlobalJSObject());
        if (NS_WARN_IF(!ct.Init(aCx, persistentCt))) {
          return CreationPromise::CreateAndReject(berytus::Failure(), __func__);
        }
      }
      RefPtr<BerytusSecretManagerActor> scmActor = new BerytusSecretManagerActor(aGlobal, scmEd25519Key);
      berytus::CreateChannelArgs args;
      nsresult rv;
      rv = berytus::Utils_ChannelMetadata(aGlobal,
                                          channelId,
                                          ct,
                                          aWebAppActor,
                                          scmActor,
                                          false,
                                          args.mChannel);
      if (NS_WARN_IF(NS_FAILED(rv))) {
        return CreationPromise::CreateAndReject(berytus::Failure(rv), __func__);
      }
      return proxy->Channel_CreateChannel(reqCx, args)
        ->Then(
          GetCurrentSerialEventTarget(), __func__,
          [aCx, channelId, aGlobal, aWebAppActor, aCertExt, scmActor, proxy, persistentCt](void*) -> RefPtr<CreationPromise> {
            RootedDictionary<BerytusChannelConstraints> ct(aCx);
            {
              // TODO(berytus): Find a way to improve passing of constraints into the lambda.
              JSAutoRealm ar(aCx, aGlobal->GetGlobalJSObject());
              if (NS_WARN_IF(!ct.Init(aCx, persistentCt))) {
                return CreationPromise::CreateAndReject(berytus::Failure(), __func__);
              }
            }
            RefPtr<BerytusChannel> ch = new BerytusChannel(
              aGlobal,
              channelId,
              std::move(ct),
              aWebAppActor,
              scmActor,
              aCertExt,
              nullptr,
              proxy
            );
            return CreationPromise::CreateAndResolve(ch, __func__);
          },
          [](berytus::Failure&& aFr) -> RefPtr<CreationPromise> {
            return CreationPromise::CreateAndReject(std::move(aFr), __func__);
          }
        );
    },
    [](berytus::Failure&& aFr) -> RefPtr<CreationPromise> {
      return CreationPromise::CreateAndReject(std::move(aFr), __func__);
    });
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
  if (!mActive) {
    aRv.ThrowInvalidStateError("Channel already closed.");
    return nullptr;
  }
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
    const BerytusKeyAgreementInput& aInput,
    ErrorResult& aRv) {
  if (!mActive) {
    aRv.ThrowInvalidStateError("Channel already closed.");
    return nullptr;
  }
  if (mKeyAgreementParams) {
    aRv.ThrowInvalidStateError("Key agreement parameters have been prepared already.");
    return nullptr;
  }
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  berytus::RequestContext reqCx;
  if (NS_WARN_IF(NS_FAILED(berytus::Utils_RequestContext(mGlobal, this, reqCx)))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<berytus::ChannelGenerateX25519KeyResult> prom =
    mAgent->Channel_GenerateX25519Key(reqCx);
  prom->Then(
    GetCurrentSerialEventTarget(), __func__,
    [this,
     outPromise,
     webAppX25519Spki = nsString(aInput.mPublic),
     unmaskList = aInput.mUnmaskAllowlist.WasPassed() ? Optional<Sequence<nsString>>(aInput.mUnmaskAllowlist.Value()) : Optional<Sequence<nsString>>()
    ](const berytus::GenerateX25519KeyResult& aGen) mutable {
      ErrorResult rv;
      nsTArray<nsString> urls;
      if (unmaskList.WasPassed()) {
        for (const auto& url : unmaskList.Value()) {
          if (NS_WARN_IF(!urls.AppendElement(url, fallible))) {
            outPromise->MaybeRejectWithTypeError("Out of memory");
            return;
          }
        }
      }
      RefPtr<BerytusKeyAgreementParameters> pams =
        BerytusKeyAgreementParameters::Create(this, 
                                              webAppX25519Spki,
                                              aGen.mPublic,
                                              std::move(urls),
                                              rv);
      if (NS_WARN_IF(rv.Failed())) {
        outPromise->MaybeReject(std::move(rv));
        return;
      }
      mKeyAgreementParams = pams;
      outPromise->MaybeResolve(pams);
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

/**
 * JSContext needed to clone ArrayBuffer.
 */
already_AddRefed<Promise> BerytusChannel::ExchangeKeyAgreementSignatures(
    JSContext* aCx,
    const ArrayBuffer& aKeyAgreementSignature,
    ErrorResult& aRv) {
  if (!mActive) {
    aRv.ThrowInvalidStateError("Channel already closed.");
    return nullptr;
  }
  if (!mKeyAgreementParams) {
    aRv.ThrowInvalidStateError("Key agreement parameters have not been prepared yet.");
    return nullptr;
  }
  berytus::RequestContext reqCx;
  berytus::VerifySignedKeyExchangeParametersArgs verifyArgs;
  berytus::SignKeyAgreementParametersArgs signArgs;
  if (NS_WARN_IF(NS_FAILED(berytus::Utils_RequestContext(mGlobal, this, reqCx)))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  verifyArgs.mWebAppSignature.Init(aKeyAgreementSignature.Obj());
  mKeyAgreementParams->ToCanonicalJSON(verifyArgs.mCanonicalJson, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  signArgs.mCanonicalJson.Assign(verifyArgs.mCanonicalJson);
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  RefPtr<berytus::ChannelVerifySignedKeyExchangeParametersResult> res =
    mAgent->Channel_VerifySignedKeyExchangeParameters(reqCx,
                                                      verifyArgs);
  res->Then(
    GetCurrentSerialEventTarget(), __func__,
    [this, reqCx = std::move(reqCx), signArgs = std::move(signArgs)](void*) {
      return mAgent->Channel_SignKeyExchangeParameters(reqCx,
                                                       signArgs);
    },
    [](berytus::Failure&& aFr) {
      return berytus::ChannelSignKeyExchangeParametersResult::CreateAndReject(std::move(aFr), __func__);
    }
  )->Then(
    GetCurrentSerialEventTarget(), __func__,
    [aCx, this, outPromise](const berytus::SignKeyAgreementParametersResult& aSigned) {
      JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());
      JS::Rooted<JS::Value> ab(aCx, JS::ObjectOrNullValue(aSigned.mScmSignature.Obj()));
      outPromise->MaybeResolveWithClone(aCx, ab);
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

already_AddRefed<Promise> BerytusChannel::EnableEndToEndEncryption(ErrorResult& aRv) {
  if (!mActive) {
    aRv.ThrowInvalidStateError("Channel already closed.");
    return nullptr;
  }
  RefPtr<Promise> outPromise = Promise::Create(mGlobal, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  berytus::RequestContext reqCx;
  if (NS_WARN_IF(NS_FAILED(berytus::Utils_RequestContext(mGlobal, this, reqCx)))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<berytus::ChannelEnableEndToEndEncryptionResult> res =
    mAgent->Channel_EnableEndToEndEncryption(reqCx);
  res->Then(
    GetCurrentSerialEventTarget(), __func__,
    [outPromise, this](void*) mutable {
      mE2EE = true;
      outPromise->MaybeResolveWithUndefined();
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

void BerytusChannel::GetID(nsString& aRv) const {
  aRv.Assign(mId);
}

} // namespace mozilla::dom
