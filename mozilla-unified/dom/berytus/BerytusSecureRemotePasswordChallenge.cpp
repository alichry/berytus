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
    const nsAString& aSecurePasswordFieldId,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!ToJSValue(aCx, aSecurePasswordFieldId, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"SelectSecurePassword"_ns, JS::HandleValue(payload), aRv);
}

nsresult BerytusSecureRemotePasswordChallenge::IsCorrectPayloadType(
    const ArrayBufferOrArrayBufferViewOrStringOrBerytusEncryptedPacket& aSrc
) const {
  if (!mParameters.mEncoding.WasPassed() ||
      mParameters.mEncoding.Value() ==
          BerytusSecureRemotePasswordChallengeEncodingType::None) {
    if (aSrc.IsArrayBuffer() ||
        aSrc.IsArrayBufferView() ||
        aSrc.IsBerytusEncryptedPacket()) {
      return NS_OK;
    }
    return NS_ERROR_INVALID_ARG;
  }
  if (!aSrc.IsString()) {
    return NS_ERROR_INVALID_ARG;
  }
  // Should be Base64URL, let's consume the string to validate it.
  // Can be optimised by implementing a "blackhole" FallibleTArray
  FallibleTArray<uint8_t> data;
  return Base64URLDecode(NS_ConvertUTF16toUTF8(aSrc.GetAsString()), Base64URLDecodePaddingPolicy::Ignore, data);
}

bool BerytusSecureRemotePasswordChallenge::PayloadToJSValue(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrStringOrBerytusEncryptedPacket& aSrc,
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
  if (aSrc.IsString()) {
    const auto& val = aSrc.GetAsString();
    return ToJSValue(aCx, val, aRetVal);
  }
  MOZ_ASSERT(aSrc.IsBerytusEncryptedPacket());
  const auto& val = OwningNonNull(aSrc.GetAsBerytusEncryptedPacket());
  berytus::BerytusEncryptedPacket packetProxy;
  berytus::utils::ToProxy::BerytusEncryptedPacket(aCx, val, packetProxy);
  return berytus::ToJSVal(aCx, packetProxy, aRetVal);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::ExchangePublicKeys(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrStringOrBerytusEncryptedPacket& aWebAppServerPublicKeyB,
    ErrorResult& aRv) {
  nsresult rv = IsCorrectPayloadType(aWebAppServerPublicKeyB);
  if (rv == NS_ERROR_INVALID_ARG) {
    aRv.ThrowTypeError("Incorrect JavaScript type for arg 0");
    return nullptr;
  }
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aWebAppServerPublicKeyB, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  // TODO(berytus): Ensure chrome code validates resolve type per the encoding type in ch parameters.
  return SendMessageRaw(aCx, u"ExchangePublicKeys"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::ComputeClientProof(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrStringOrBerytusEncryptedPacket& aSalt,
    ErrorResult& aRv) {
  nsresult rv = IsCorrectPayloadType(aSalt);
  if (rv == NS_ERROR_INVALID_ARG) {
    aRv.ThrowTypeError("Incorrect JavaScript type for arg 0");
    return nullptr;
  }
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aSalt, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  // TODO(berytus): Ensure chrome code validates resolve type per the encoding type in ch parameters.
  return SendMessageRaw(aCx, u"ComputeClientProof"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusSecureRemotePasswordChallenge::VerifyServerProof(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrStringOrBerytusEncryptedPacket& aServerProofM2,
    ErrorResult& aRv) {
  nsresult rv = IsCorrectPayloadType(aServerProofM2);
  if (rv == NS_ERROR_INVALID_ARG) {
    aRv.ThrowTypeError("Incorrect JavaScript type for arg 0");
    return nullptr;
  }
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
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
  if (aParameters.mEncoding.WasPassed()) {
    copiedParams.mEncoding.Construct(aParameters.mEncoding.Value());
  }
  return do_AddRef(
    new BerytusSecureRemotePasswordChallenge(
      global, aId, std::move(copiedParams)));
}

} // namespace mozilla::dom