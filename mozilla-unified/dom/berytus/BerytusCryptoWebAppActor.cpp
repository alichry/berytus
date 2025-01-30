/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusCryptoWebAppActor.h"
#include "mozilla/dom/BerytusCryptoWebAppActorBinding.h"
#include "mozilla/dom/BindingUtils.h"
#include "nsCycleCollectionParticipant.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusCryptoWebAppActor,
// # BerytusWebAppActor, your, members, here)"
// TEST:
// NS_IMPL_ADDREF_INHERITED(BerytusCryptoWebAppActor, BerytusWebAppActor)
// NS_IMPL_RELEASE_INHERITED(BerytusCryptoWebAppActor, BerytusWebAppActor)
// NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusCryptoWebAppActor)
// NS_INTERFACE_MAP_END_INHERITING(BerytusWebAppActor)

NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusCryptoWebAppActor, BerytusWebAppActor)
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusCryptoWebAppActor, BerytusWebAppActor)

BerytusCryptoWebAppActor::BerytusCryptoWebAppActor(
  nsIGlobalObject* aGlobal,
  const nsAString& aEd25519Key
) : BerytusWebAppActor(aGlobal), mEd25519Key(aEd25519Key)
{
    // Add |MOZ_COUNT_CTOR(BerytusCryptoWebAppActor);| for a non-refcounted object.
}

BerytusCryptoWebAppActor::BerytusCryptoWebAppActor(
  const BerytusCryptoWebAppActor& rhs
) : BerytusWebAppActor(rhs.mGlobal), mEd25519Key(rhs.mEd25519Key) { }

already_AddRefed<BerytusWebAppActor> BerytusCryptoWebAppActor::Clone() const {
  RefPtr<BerytusCryptoWebAppActor> cl = new BerytusCryptoWebAppActor(*this);
  return cl.forget();
}

BerytusCryptoWebAppActor::~BerytusCryptoWebAppActor()
{
    // Add |MOZ_COUNT_DTOR(BerytusCryptoWebAppActor);| for a non-refcounted object.
}

already_AddRefed<BerytusCryptoWebAppActor> BerytusCryptoWebAppActor::Constructor(
  const GlobalObject &aGlobal,
  const nsAString &aEd25519Key,
  ErrorResult &aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<BerytusCryptoWebAppActor> obj = new BerytusCryptoWebAppActor(global, aEd25519Key);
  return obj.forget();
}

BerytusWebAppActorType BerytusCryptoWebAppActor::Type() const {
  return BerytusWebAppActorType::CryptoActor;
}

nsIGlobalObject* BerytusCryptoWebAppActor::GetParentObject() const { return mGlobal; }

JSObject*
BerytusCryptoWebAppActor::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusCryptoWebAppActor_Binding::Wrap(aCx, this, aGivenProto);
}

void BerytusCryptoWebAppActor::GetEd25519Key(nsString& aRetVal) const
{
  return aRetVal.Assign(mEd25519Key);
}

} // namespace mozilla::dom
