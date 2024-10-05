/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSecretManagerActor.h"
#include "mozilla/dom/BerytusSecretManagerActorBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(BerytusSecretManagerActor)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusSecretManagerActor)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusSecretManagerActor)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusSecretManagerActor)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusSecretManagerActor::BerytusSecretManagerActor(
  nsIGlobalObject* aGlobal,
  const nsAString& aEd25519Key
) : mGlobal(aGlobal), mEd25519Key(aEd25519Key)
{
    // Add |MOZ_COUNT_CTOR(BerytusSecretManagerActor);| for a non-refcounted object.
}

BerytusSecretManagerActor::~BerytusSecretManagerActor()
{
    // Add |MOZ_COUNT_DTOR(BerytusSecretManagerActor);| for a non-refcounted object.
}

nsIGlobalObject* BerytusSecretManagerActor::GetParentObject() const { return mGlobal; }

JSObject*
BerytusSecretManagerActor::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusSecretManagerActor_Binding::Wrap(aCx, this, aGivenProto);
}

void BerytusSecretManagerActor::GetEd25519Key(nsString& aRetVal) const
{
  aRetVal.Assign(mEd25519Key);
}


} // namespace mozilla::dom
