/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCHANNEL_H_
#define DOM_BERYTUSCHANNEL_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusChannelBinding.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/TypedArray.h" // ArrayBuffer
#include "mozilla/berytus/AgentProxy.h" // AgentProxy

namespace mozilla {
namespace dom {

struct BerytusChannelOptions;
class BerytusKeyAgreementParameters;
class BerytusSecretManagerActor;
class BerytusWebAppActor;
class GlobalObject;
class Promise;

} // namespace dom
} // namespace mozilla

namespace mozilla::dom {

class BerytusChannel final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                             public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusChannel)

protected:
  BerytusChannel(
    nsIGlobalObject* aGlobal,
    JSObject* aConstraints,
    const RefPtr<BerytusWebAppActor>& aWebAppActor,
    const RefPtr<BerytusSecretManagerActor>& aSecretManagerActor,
    const RefPtr<BerytusKeyAgreementParameters>& aKeyAgreementParams, // can be a nullptr
    const RefPtr<mozilla::berytus::AgentProxy>& aAgent
  );
  ~BerytusChannel();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  JS::Heap<JSObject*> mCachedConstraints;
  RefPtr<BerytusWebAppActor> mWebAppActor;
  RefPtr<BerytusSecretManagerActor> mSecretManagerActor;
  RefPtr<BerytusKeyAgreementParameters> mKeyAgreementParams; // can be a nullptr
  RefPtr<mozilla::berytus::AgentProxy> mAgent;

  bool mActive = true;
public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  bool Active() const;

  const berytus::AgentProxy& Agent() const;

  void GetConstraints(JSContext* aCx, JS::MutableHandle<JSObject*> aRetVal);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusWebAppActor> WebApp() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusSecretManagerActor> GetSecretManager() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusKeyAgreementParameters> GetKeyAgreementParams() const;

  static // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> Create(const GlobalObject& global, const BerytusChannelOptions& options, ErrorResult& aRv);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> Close(ErrorResult& aRv);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> PrepareKeyAgreementParameters(const nsAString& webAppX25519PublicKey, ErrorResult& aRv);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> EnableEndToEndEncryption(const ArrayBuffer& keyAgreementSignature, ErrorResult& aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCHANNEL_H_
