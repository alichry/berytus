/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusOffChannelOtpChallenge.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {

NS_IMPL_ADDREF_INHERITED(BerytusOffChannelOtpChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusOffChannelOtpChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusOffChannelOtpChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusOffChannelOtpChallenge::BerytusOffChannelOtpChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::OffChannelOtp,
                                             aID) {}

BerytusOffChannelOtpChallenge::~BerytusOffChannelOtpChallenge() {}

void BerytusOffChannelOtpChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
  mCachedParameters = nullptr;
}

JSObject*
BerytusOffChannelOtpChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusOffChannelOtpChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusOffChannelOtpChallenge::GetOtp(
    JSContext* aCx,
    const nsAString& aForeignIdentityField,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!ToJSValue(aCx, aForeignIdentityField, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
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
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return do_AddRef(
    new BerytusOffChannelOtpChallenge(
      global, aId));
}

} // namespace mozilla::dom