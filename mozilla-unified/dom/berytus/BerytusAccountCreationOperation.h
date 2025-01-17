/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSACCOUNTCREATIONOPERATION_H_
#define DOM_BERYTUSACCOUNTCREATIONOPERATION_H_

#include "BerytusChannel.h"
#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusAccountMetadata.h"
#include "mozilla/dom/BerytusAccount.h"

namespace mozilla {
namespace dom {

class Promise;

} // namespace dom
} // namespace mozilla

namespace mozilla::dom {


class BerytusAccountCreationOperation final : public BerytusLoginOperation,
                                              public BerytusAccountMetadata, // both define a mGlobal member
                                              public BerytusAccount
{
public:
  //NS_DECL_ISUPPORTS_INHERITED
  ////NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)


using CreationPromise = MozPromise<RefPtr<BerytusAccountCreationOperation>, berytus::Failure, true>;

public:
  static RefPtr<CreationPromise> CreateApproved(
    JSContext* aCx, 
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId);
protected:
  BerytusAccountCreationOperation(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId
  );
  ~BerytusAccountCreationOperation();
  BerytusChannel* Channel() const override;
  BerytusFieldMap* FieldMap() const override;
  BerytusUserAttributeMap* UserAttributeMap() const override;
  BerytusLoginOperation* Operation() override;
  bool Active() const override;

  RefPtr<BerytusChannel> mChannel;
  RefPtr<BerytusFieldMap> mFields;
  RefPtr<BerytusUserAttributeMap> mUserAttributes;
  
public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const override;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  bool Newborn() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> Save(ErrorResult& aRv);

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> TransitionToAuthOperation(ErrorResult& aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSACCOUNTCREATIONOPERATION_H_
