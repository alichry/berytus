/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusIdentificationChallenge.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusIdentificationChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusIdentificationChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusIdentificationChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusIdentificationChallenge::BerytusIdentificationChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::Identification,
                                             aID) {}

BerytusIdentificationChallenge::~BerytusIdentificationChallenge() {}

void BerytusIdentificationChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
  mCachedParameters = nullptr;
}

JSObject*
BerytusIdentificationChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusIdentificationChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusIdentificationChallenge::GetIdentityFields(
    JSContext* aCx,
    const Sequence<nsString>& aIdentityFieldIds,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!ToJSValue(aCx, aIdentityFieldIds, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"GetIdentityFields"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusIdentificationChallenge::AbortWithIdentityDoesNotExistsError(
    JSContext* aCx,
    ErrorResult& aRv
) {
  return Abort(aCx, BerytusChallengeAbortionCode::IdentityDoesNotExists, aRv);
}

already_AddRefed<BerytusIdentificationChallenge> BerytusIdentificationChallenge::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return do_AddRef(
    new BerytusIdentificationChallenge(
      global, aId));
}

} // namespace mozilla::dom