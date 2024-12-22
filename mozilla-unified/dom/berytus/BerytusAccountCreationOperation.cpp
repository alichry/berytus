/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusAccountCreationOperation.h"
#include "BerytusChannel.h"
#include "BerytusLoginOperation.h"
#include "mozilla/dom/BerytusAccountCreationOperationBinding.h"
#include "mozilla/dom/BerytusFieldMap.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountCreationOperation,
// # BerytusLoginOperation, your, members, here)"
// NS_IMPL_ADDREF_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
// NS_IMPL_RELEASE_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
// NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAccountCreationOperation)
// NS_INTERFACE_MAP_END_INHERITING(BerytusLoginOperation)
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation, mFields, mUserAttributes)
NS_IMPL_ADDREF_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
NS_IMPL_RELEASE_INHERITED(BerytusAccountCreationOperation, BerytusLoginOperation)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAccountCreationOperation)
NS_INTERFACE_MAP_END_INHERITING(BerytusLoginOperation)

BerytusAccountCreationOperation::BerytusAccountCreationOperation(nsIGlobalObject* aGlobalObject,
                                                                 const RefPtr<BerytusChannel>& aChannel,
                                                                 const nsAString& aOperationId)
  : BerytusLoginOperation(aGlobalObject, aChannel, aOperationId, BerytusOnboardingIntent::Register),
    mChannel(aChannel),
    mFields(new BerytusFieldMap(aGlobalObject)),
    mUserAttributes(new BerytusUserAttributeMap(aGlobalObject))
{
    // Add |MOZ_COUNT_CTOR(BerytusAccountCreationOperation);| for a non-refcounted object.
}

BerytusAccountCreationOperation::~BerytusAccountCreationOperation()
{
    // Add |MOZ_COUNT_DTOR(BerytusAccountCreationOperation);| for a non-refcounted object.
}

JSObject*
BerytusAccountCreationOperation::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusAccountCreationOperation_Binding::Wrap(aCx, this, aGivenProto);
}

nsIGlobalObject* BerytusAccountCreationOperation::GetParentObject() const { return mGlobal; }

BerytusChannel* BerytusAccountCreationOperation::Channel() const {
  return mChannel;
}

BerytusLoginOperation* BerytusAccountCreationOperation::Operation() {
  return this;
}

BerytusFieldMap* BerytusAccountCreationOperation::FieldMap() const {
  return mFields;
}
BerytusUserAttributeMap* BerytusAccountCreationOperation::UserAttributeMap() const {
  return mUserAttributes;
}

bool BerytusAccountCreationOperation::Newborn() const {
  return false;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountCreationOperation::Save(ErrorResult& aRv) {
  MOZ_ASSERT(false, "BerytusAccountCreationOperation::Save not impld");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountCreationOperation::TransitionToAuthOperation(ErrorResult& aRv) {
  MOZ_ASSERT(false, "BerytusAccountCreationOperation::TransitionToAuthOperation not impld");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}

RefPtr<BerytusAccountCreationOperation::CreationPromise> BerytusAccountCreationOperation::CreateApproved(
    JSContext* aCx,
    nsIGlobalObject* aGlobalObject,
    const RefPtr<BerytusChannel>& aChannel,
    const nsAString& aOperationId) {
  RefPtr<BerytusAccountCreationOperation> op = new BerytusAccountCreationOperation(
      aGlobalObject, aChannel, aOperationId);
  return op->PopulateUserAttributeMap(aCx)->Then(GetCurrentSerialEventTarget(), __func__,
    [op]() -> RefPtr<BerytusAccountCreationOperation::CreationPromise> {
      return BerytusAccountCreationOperation::CreationPromise::CreateAndResolve(op, __func__);
    },
    [](const berytus::Failure& aFr) -> RefPtr<BerytusAccountCreationOperation::CreationPromise> {
    return BerytusAccountCreationOperation::CreationPromise::CreateAndReject(aFr, __func__);
    }
  );
}

} // namespace mozilla::dom
