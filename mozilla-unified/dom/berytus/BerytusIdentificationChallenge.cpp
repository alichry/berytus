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
    const Sequence<OwningStringOrBerytusEncryptedPacket>& aIdentityFieldIds,
    ErrorResult& aRv) {
  if (aIdentityFieldIds.IsEmpty()) {
    aRv.ThrowTypeError("At least one identity field identifier must be provided");
    return nullptr;
  }
  JS::Rooted<JS::Value> payload(aCx);
  Sequence<nsString> asStrings;
  Sequence<OwningNonNull<BerytusEncryptedPacket>> asPackets;
  for (const auto& item : aIdentityFieldIds) {
    if (item.IsString()) {
      if (asPackets.Length() > 0) {
        aRv.Throw(NS_ERROR_FAILURE);
        return nullptr;
      }
      if (NS_WARN_IF(!asStrings.AppendElement(item.GetAsString(), fallible))) {
        aRv.ThrowTypeError("Mixed types in identity field identifiers are not allowed");
        return nullptr;
      }
    } else {
      if (asStrings.Length() > 0) {
        aRv.ThrowTypeError("Mixed types in identity field identifiers are not allowed");
        return nullptr;
      }
      if (NS_WARN_IF(!asPackets.AppendElement(item.GetAsBerytusEncryptedPacket(), fallible))) {
        aRv.Throw(NS_ERROR_FAILURE);
        return nullptr;
      }
    }
  }
  if (asStrings.Length() > 0) {
    if (NS_WARN_IF(!ToJSValue(aCx, asStrings, &payload))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
  } else {
    if (NS_WARN_IF(!ToJSValue(aCx, asPackets, &payload))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
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