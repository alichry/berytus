/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusOffChannelOtpChallenge.h"
#include "mozilla/dom/ToJSValue.h"
#include "mozilla/berytus/AgentProxyUtils.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusOffChannelOtpChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusOffChannelOtpChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusOffChannelOtpChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusOffChannelOtpChallenge::BerytusOffChannelOtpChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID,
    BerytusOffChannelOtpChallengeParameters&& aParameters) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::OffChannelOtp,
                                             aID), mParameters(std::move(aParameters)) {}

BerytusOffChannelOtpChallenge::~BerytusOffChannelOtpChallenge() {}

void BerytusOffChannelOtpChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
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

BerytusOffChannelOtpChallengeParameters const& BerytusOffChannelOtpChallenge::Parameters() const {
  return mParameters;
}

JSObject*
BerytusOffChannelOtpChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusOffChannelOtpChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusOffChannelOtpChallenge::GetOtp(
    JSContext* aCx,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  payload.setNull();
  return SendMessageRaw(aCx, u"GetOtp"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusOffChannelOtpChallenge::AbortWithIncorrectOtpError(
    JSContext* aCx,
    ErrorResult& aRv
) {
  return Abort(aCx, BerytusChallengeAbortionCode::IncorrectOtp, aRv);
}

already_AddRefed<BerytusOffChannelOtpChallenge> BerytusOffChannelOtpChallenge::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusOffChannelOtpChallengeParameters& aParameters,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  BerytusOffChannelOtpChallengeParameters copiedParams;
  if (NS_WARN_IF(!copiedParams.mField.Assign(aParameters.mField, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  return do_AddRef(
    new BerytusOffChannelOtpChallenge(
      global, aId, std::move(copiedParams)));
}

} // namespace mozilla::dom