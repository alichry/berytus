/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSLOGINOPERATION_H_
#define DOM_BERYTUSLOGINOPERATION_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusChannelBinding.h" // BerytusOnboardingIntent

namespace mozilla::dom {

class BerytusChannel;

class BerytusLoginOperation : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                              public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusLoginOperation)

  using CreationPromise = MozPromise<RefPtr<BerytusLoginOperation>, berytus::Failure, true>;
protected:
  BerytusLoginOperation(
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId,
    const BerytusOnboardingIntent& aIntent
  );
  virtual ~BerytusLoginOperation();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  RefPtr<BerytusChannel> mChannel;
  nsString mId;
  const BerytusOnboardingIntent mIntent;
  bool mActive;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  BerytusOnboardingIntent Intent() const;

  bool Active() const;

  already_AddRefed<Promise> Close(ErrorResult& aRv);

  void GetID(nsString& aRv) const;

  static RefPtr<CreationPromise> Create(
      JSContext* aCx, 
      nsIGlobalObject* aGlobal,
      RefPtr<BerytusChannel>& aChannel,
      const BerytusOnboardingOptions& aOptions);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSLOGINOPERATION_H_
