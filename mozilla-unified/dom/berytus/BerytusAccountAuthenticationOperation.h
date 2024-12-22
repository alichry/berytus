/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSACCOUNTAUTHENTICATIONOPERATION_H_
#define DOM_BERYTUSACCOUNTAUTHENTICATIONOPERATION_H_

#include "js/TypeDecls.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusAccountBinding.h" // BerytusAccountStatus
#include "mozilla/dom/BerytusAccountMetadata.h"
#include "mozilla/dom/BerytusChallengeBinding.h" // BerytusChallengeType
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusChallengeMap.h"

namespace mozilla {
namespace dom {

class BerytusChallengeParameters;
class Promise;

} // namespace dom
} // namespace mozilla

namespace mozilla::dom {

class BerytusAccountAuthenticationOperation final : public BerytusLoginOperation,
                                                    public BerytusAccountMetadata
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusAccountAuthenticationOperation, BerytusLoginOperation)

  using CreationPromise = MozPromise<RefPtr<BerytusAccountAuthenticationOperation>, berytus::Failure, true>;

public:
  static RefPtr<CreationPromise> CreateApproved(
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId);
protected:
  BerytusAccountAuthenticationOperation(nsIGlobalObject* aGlobalObject,
                                        const RefPtr<BerytusChannel>& aChannel,
                                        const nsAString& aOperationId);
  ~BerytusAccountAuthenticationOperation();

  RefPtr<BerytusChallengeMap> mChallenges;

  BerytusChannel* Channel() const override;
  BerytusLoginOperation* Operation() override;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const override;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusChallengeMap> Challenges() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> CreateChallenge(const nsAString& challengeId, BerytusChallengeType challengeType, const Optional<BerytusChallengeParameters*>& challengeParameters, ErrorResult& aRv);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> Finish(ErrorResult& aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSACCOUNTAUTHENTICATIONOPERATION_H_
