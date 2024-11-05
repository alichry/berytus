/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChallengeParameters.h"
#include "mozilla/dom/BerytusChallengeParametersBinding.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(BerytusChallengeParameters)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChallengeParameters)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChallengeParameters)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChallengeParameters)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChallengeParameters::BerytusChallengeParameters(nsIGlobalObject* aGlobal) : mGlobal(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusChallengeParameters);| for a non-refcounted object.
}

BerytusChallengeParameters::~BerytusChallengeParameters()
{
    // Add |MOZ_COUNT_DTOR(BerytusChallengeParameters);| for a non-refcounted object.
}

nsIGlobalObject* BerytusChallengeParameters::GetParentObject() const { return mGlobal; }

JSObject*
BerytusChallengeParameters::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusChallengeParameters_Binding::Wrap(aCx, this, aGivenProto);
}


} // namespace mozilla::dom
