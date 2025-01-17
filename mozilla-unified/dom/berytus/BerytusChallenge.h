/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCHALLENGE_H_
#define DOM_BERYTUSCHALLENGE_H_

#include "js/TypeDecls.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/MozPromise.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusChallengeBinding.h"
#include "mozilla/dom/BerytusChallengeMessageBinding.h" // BerytusChallengeMessageRequestDefinition
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusAccountAuthenticationOperation.h"

namespace mozilla::dom {

/**
 * Not-too-bad of an impl with a bit of slapdash (bypassed ProxyUtils pattern in favour of native impls of To-JS-Value-ish routines).
 * TODO(berytus): Enhance impl quality in this class and its children.
 */
class BerytusChallenge : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                         public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusChallenge)

  using AnyMessageResponse = berytus::AccountAuthenticationRespondToChallengeMessageResult::ResolveValueType;

  template <typename T>
  using SendMessageResult = MozPromise<T, berytus::Failure, true>;

public:
  BerytusChallenge(nsIGlobalObject* aGlobal,
                   const BerytusChallengeType& aType,
                   const nsAString& aID);

protected:
  virtual ~BerytusChallenge();
  nsCOMPtr<nsIGlobalObject> mGlobal;
private:
  /* potentially both could be nullptrs,
   * using method-getters instead */
  RefPtr<BerytusChannel> mChannel;
  RefPtr<BerytusLoginOperation> mOperation;
  void Connect(const RefPtr<BerytusChannel>& aChannel,
               const RefPtr<BerytusLoginOperation>& aOperation);
  friend already_AddRefed<Promise> BerytusAccountAuthenticationOperation::CreateChallenge(
    JSContext* aCx,
    const OwningNonNull<BerytusChallenge>& aChallenge,
    ErrorResult& aRv);
protected:
  BerytusChallengeType mType;
  nsString mID;
  bool mActive;
  JS::Heap<JSObject*> mCachedParameters;

  BerytusChannel* Channel();
  BerytusLoginOperation* Operation();

public:
  bool Connected() const;
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  void GetId(nsAString& aRetVal);

  BerytusChallengeType Type() const;

  bool Active();

  void GetParameters(JSContext* aCx,
                     JS::MutableHandle<JSObject*> aRetVal,
                     ErrorResult& aRv);

  already_AddRefed<Promise> Send(JSContext *aCx,
                                 const BerytusChallengeMessageRequestDefinition& aInput,
                                 ErrorResult& aRv);

  already_AddRefed<Promise> SendMessageRaw(JSContext* aCx,
                                        const nsString& aMessageName,
                                        JS::Handle<JS::Value> aMessagePayload,
                                        ErrorResult& aRv);

  template <typename P>
  already_AddRefed<Promise> SendMessageRaw(JSContext* aCx,
                                        const nsString& aMessageName,
                                        P& aMessagePayload,
                                        ErrorResult& aRv);

  template <typename T = AnyMessageResponse, typename P>
  RefPtr<SendMessageResult<T>> SendMessage(JSContext* aCx,
                                           const nsString& aMessageName,
                                           P& aMessagePayload,
                                           ErrorResult& aRv);

  template <typename T = AnyMessageResponse>
  RefPtr<SendMessageResult<T>> SendMessage(JSContext* aCx,
                                           const nsString& aMessageName,
                                           JS::Handle<JS::Value> aMessagePayload,
                                           ErrorResult& aRv);

  already_AddRefed<Promise> Seal(JSContext* aCx, ErrorResult& aRv);

  already_AddRefed<Promise> Abort(
      JSContext* aCx,
      const BerytusChallengeAbortionCode& aCode,
      ErrorResult& aRv);

  already_AddRefed<Promise> AbortWithUserInterruptError(
      JSContext* aCx,
      ErrorResult& aRv);

  already_AddRefed<Promise> AbortWithGenericWebAppFailureError(
      JSContext* aCx,
      ErrorResult& aRv);
protected:
  virtual void CacheParameters(JSContext* aCx, ErrorResult& aRv) = 0;

  void BuildChallengeInfo(JSContext* aCx, JS::MutableHandle<JS::Value> aRetVal, ErrorResult& aRv);
  void BuildChallengeMessage(
      JSContext* aCx,
      const nsString& aMessageName,
      JS::Handle<JS::Value> aPayload,
      JS::MutableHandle<JS::Value> aRetVal,
      ErrorResult& aRv);

  already_AddRefed<Promise> MaybeCatchBerytusFailure(
    const RefPtr<Promise>& aPromise,
    ErrorResult& aRv);

  template <typename ResolveCallback>
  already_AddRefed<Promise> MaybeCatchBerytusFailure(
      const RefPtr<Promise>& aPromise,
      ErrorResult& aRv,
      ResolveCallback&& aOnResolve);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCHALLENGE_H_
