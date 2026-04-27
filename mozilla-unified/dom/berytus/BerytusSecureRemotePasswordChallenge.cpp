/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusSecureRemotePasswordChallenge.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/Base64.h"
#include "mozilla/dom/BerytusChallengeBinding.h"
#include "mozilla/dom/ToJSValue.h"

namespace mozilla::dom {


NS_IMPL_ADDREF_INHERITED(BerytusSecureRemotePasswordChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusSecureRemotePasswordChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusSecureRemotePasswordChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusSecureRemotePasswordChallenge::BerytusSecureRemotePasswordChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID,
    BerytusSecureRemotePasswordChallengeParameters&& aParameters) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::SecureRemotePassword,
                                             aID),
                            mParameters(std::move(aParameters)) {}

BerytusSecureRemotePasswordChallenge::~BerytusSecureRemotePasswordChallenge() {}

void BerytusSecureRemotePasswordChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
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

BerytusSecureRemotePasswordChallengeParameters const& BerytusSecureRemotePasswordChallenge::Parameters() const {
  return mParameters;
}

JSObject*
BerytusSecureRemotePasswordChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusSecureRemotePasswordChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::SelectSecurePassword(
    JSContext* aCx,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  payload.setNull();
  return SendMessageRaw(aCx, u"SelectSecurePassword"_ns, JS::HandleValue(payload), aRv);
}

bool BerytusSecureRemotePasswordChallenge::PayloadToJSValue(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aSrc,
    JS::MutableHandle<JS::Value> aRetVal
) {
  if (aSrc.IsArrayBuffer()) {
    const auto& val = aSrc.GetAsArrayBuffer();
    aRetVal.setObject(*val.Obj());
    return true;
  }
  if (aSrc.IsArrayBufferView()) {
    // TODO(berytus): When we disallow ArrayBufferViews upstream, amend this.
    const auto& val = aSrc.GetAsArrayBufferView();
    aRetVal.setObject(*val.Obj());
    return true;
  }
  MOZ_ASSERT(aSrc.IsBerytusEncryptedPacket());
  const auto& val = OwningNonNull(aSrc.GetAsBerytusEncryptedPacket());
  berytus::BerytusEncryptedPacket packetProxy;
  NS_ENSURE_TRUE(berytus::utils::ToProxy::BerytusEncryptedPacket(aCx, val, packetProxy), false);
  NS_ENSURE_TRUE(berytus::ToJSVal(aCx, packetProxy, aRetVal), false);
  return true;
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::ExchangePublicKeys(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aWebAppServerPublicKeyB,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aWebAppServerPublicKeyB, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"ExchangePublicKeys"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::ComputeClientProof(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aSalt,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aSalt, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"ComputeClientProof"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::VerifyServerProof(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aServerProofM2,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aServerProofM2, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"VerifyServerProof"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::AbortWithInvalidProofError(
    JSContext* aCx,
    ErrorResult& aRv
) {
  return Abort(aCx, BerytusChallengeAbortionCode::InvalidProof, aRv);
}

already_AddRefed<BerytusSecureRemotePasswordChallenge> BerytusSecureRemotePasswordChallenge::Constructor(
  const GlobalObject& aGlobal,
  const nsAString& aId,
  const BerytusSecureRemotePasswordChallengeParameters& aParameters,
  ErrorResult& aRv
) {
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  BerytusSecureRemotePasswordChallengeParameters copiedParams;
  if (NS_WARN_IF(!copiedParams.mField.Assign(aParameters.mField, fallible))) {
    aRv.ThrowInvalidStateError("Out of memory");
    return nullptr;
  }
  return do_AddRef(
    new BerytusSecureRemotePasswordChallenge(
      global, aId, std::move(copiedParams)));
}

} // namespace mozilla::dom