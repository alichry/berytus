/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusPasswordChallenge.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusPasswordChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusPasswordChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusPasswordChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusPasswordChallenge::BerytusPasswordChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::Password,
                                             aID) {}

BerytusPasswordChallenge::~BerytusPasswordChallenge() {}

void BerytusPasswordChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
  mCachedParameters = nullptr;
}

JSObject*
BerytusPasswordChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusPasswordChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusPasswordChallenge::GetPasswordFields(
    JSContext* aCx,
    const Sequence<nsString>& aPasswordFieldIds,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!ToJSValue(aCx, aPasswordFieldIds, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"GetPasswordFields"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusPasswordChallenge::AbortWithIncorrectPasswordError(
    JSContext* aCx,
    ErrorResult& aRv
) {
  return Abort(aCx, BerytusChallengeAbortionCode::IncorrectPassword, aRv);
}

already_AddRefed<BerytusPasswordChallenge> BerytusPasswordChallenge::Constructor(
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
    new BerytusPasswordChallenge(
      global, aId));
}

} // namespace mozilla::dom