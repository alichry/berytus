/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCHALLENGEMAP_H_
#define DOM_BERYTUSCHALLENGEMAP_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

class BerytusChallenge;

class BerytusChallengeMap final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                                  public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusChallengeMap)

public:
  BerytusChallengeMap(nsIGlobalObject* aGlobal);

protected:
  ~BerytusChallengeMap();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsTArray<RefPtr<BerytusChallenge>> mChallenges;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  const nsTArray<RefPtr<BerytusChallenge>>& List() const;

  void AddChallenge(BerytusChallenge* aChallenge, ErrorResult& aRv);

  /* caller should aRv if it has failed first, before checking the returned bool */
  bool HasChallenge(const nsAString& aChallengeId, ErrorResult& aRv);
  /* caller should aRv if it has failed first, before checking the returned bool */
  bool HasActiveChallenge(const nsAString& aChallengeId, ErrorResult& aRv);

  already_AddRefed<BerytusChallenge> GetChallenge(
    const nsAString& aChallengeId,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCHALLENGEMAP_H_
