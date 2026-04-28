/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusIdentificationChallenge.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusIdentificationChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusIdentificationChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusIdentificationChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusIdentificationChallenge::BerytusIdentificationChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID,
    BerytusIdentificationChallengeParameters&& aParameters) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::Identification,
                                             aID),
                                             mParameters(std::move(aParameters)) {}

BerytusIdentificationChallenge::~BerytusIdentificationChallenge() {}

void BerytusIdentificationChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
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

BerytusIdentificationChallengeParameters const& BerytusIdentificationChallenge::Parameters() const {
  return mParameters;
}

JSObject*
BerytusIdentificationChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusIdentificationChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusIdentificationChallenge::GetIdentityFields(
    JSContext* aCx,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  payload.setNull();
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
  const BerytusIdentificationChallengeParameters& aParameters,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (aParameters.mFields.Length() == 0) {
    aRv.ThrowTypeError("Parameter `field` must be a non-empty list of field ids");
    return nullptr;
  }
  BerytusIdentificationChallengeParameters copiedParams;
  if (NS_WARN_IF(!copiedParams.mFields.Assign(aParameters.mFields))) {
    aRv.ThrowInvalidStateError("Out of memory");
    return nullptr;
  }
  return do_AddRef(
    new BerytusIdentificationChallenge(
      global, aId, std::move(copiedParams)));
}

} // namespace mozilla::dom