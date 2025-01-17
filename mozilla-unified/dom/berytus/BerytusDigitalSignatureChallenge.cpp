/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusDigitalSignatureChallenge.h"
#include "mozilla/RefPtr.h"
#include "mozilla/berytus/AgentProxyUtils.h"

namespace mozilla::dom {


NS_IMPL_ADDREF_INHERITED(BerytusDigitalSignatureChallenge, BerytusChallenge)
NS_IMPL_RELEASE_INHERITED(BerytusDigitalSignatureChallenge, BerytusChallenge)
NS_INTERFACE_MAP_BEGIN(BerytusDigitalSignatureChallenge)
NS_INTERFACE_MAP_END_INHERITING(BerytusChallenge)

BerytusDigitalSignatureChallenge::BerytusDigitalSignatureChallenge(
    nsIGlobalObject* aGlobal,
    const nsAString& aID) : BerytusChallenge(aGlobal,
                                             BerytusChallengeType::DigitalSignature,
                                             aID) {}

BerytusDigitalSignatureChallenge::~BerytusDigitalSignatureChallenge() {}

void BerytusDigitalSignatureChallenge::CacheParameters(JSContext* aCx, ErrorResult& aRv) {
  mCachedParameters = nullptr;
}

JSObject*
BerytusDigitalSignatureChallenge::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) {
  return BerytusDigitalSignatureChallenge_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<Promise> BerytusDigitalSignatureChallenge::SelectKey(
    JSContext* aCx,
    const nsAString& aKeyFieldId,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!ToJSValue(aCx, aKeyFieldId, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  // TODO(berytus): Resolve with a BerytusKeyFieldValue
  return SendMessageRaw(aCx, u"SelectKey"_ns, JS::HandleValue(payload), aRv);
}

// TODO(berytus): This impl overlaps with that of BerytusSecureRemotePasswordChallenge
bool BerytusDigitalSignatureChallenge::PayloadToJSValue(
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
    const auto& val = aSrc.GetAsArrayBufferView();
    aRetVal.setObject(*val.Obj());
    return true;
  }
  MOZ_ASSERT(aSrc.IsBerytusEncryptedPacket());
  const auto& val = OwningNonNull(aSrc.GetAsBerytusEncryptedPacket());
  berytus::BerytusEncryptedPacket packetProxy;
  berytus::utils::ToProxy::BerytusEncryptedPacket(aCx, val, packetProxy);
  return berytus::ToJSVal(aCx, packetProxy, aRetVal);
}

already_AddRefed<Promise> BerytusDigitalSignatureChallenge::SignNonce(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aNonce,
    ErrorResult& aRv) {
  JS::Rooted<JS::Value> payload(aCx);
  if (NS_WARN_IF(!PayloadToJSValue(aCx, aNonce, &payload))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return SendMessageRaw(aCx, u"SignNonce"_ns, JS::HandleValue(payload), aRv);
}

already_AddRefed<Promise> BerytusDigitalSignatureChallenge::AbortWithInvalidSignatureError(
    JSContext* aCx,
    ErrorResult& aRv
) {
  return Abort(aCx, BerytusChallengeAbortionCode::InvalidSignature, aRv);
}

already_AddRefed<BerytusDigitalSignatureChallenge> BerytusDigitalSignatureChallenge::Constructor(
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
    new BerytusDigitalSignatureChallenge(
      global, aId));
}

} // namespace mozilla::dom