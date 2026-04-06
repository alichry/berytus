/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusPasswordChallenge.h"
#include "mozilla/dom/ToJSValue.h"
#include "mozilla/fallible.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusPasswordChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusPasswordChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusPasswordChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusPasswordChallenge::BerytusPasswordChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID,
    BerytusPasswordChallengeParameters&& aParameters) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::Password,
                                             aID),
                                             mParameters(std::move(aParameters)) {}

BerytusPasswordChallenge::~BerytusPasswordChallenge() {}

void BerytusPasswordChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
  if (mCachedParameters) {
    return;
  }
  JS::Rooted<JS::Value> options(aCx);
  if (NS_WARN_IF(!mParameters.ToObjectInternal(aCx, &options))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedParameters = options.toObjectOrNull();
}

BerytusPasswordChallengeParameters const& BerytusPasswordChallenge::Parameters() const {
  return mParameters;
}

JSObject*
BerytusPasswordChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusPasswordChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusPasswordChallenge::GetPasswordFields(
    JSContext* aCx,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  payload.setNull();
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
  const BerytusPasswordChallengeParameters& aParameters,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  BerytusPasswordChallengeParameters copiedParams;
  if (NS_WARN_IF(!copiedParams.mFields.Assign(aParameters.mFields))) {
    aRv.ThrowInvalidStateError("Out of memory");
    return nullptr;
  }
  return do_AddRef(
    new BerytusPasswordChallenge(
      global, aId, std::move(copiedParams)));
}

} // namespace mozilla::dom