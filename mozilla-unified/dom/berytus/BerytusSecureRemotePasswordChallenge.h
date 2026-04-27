/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSECUREREMOTEPASSWORDCHALLENGE_H_
#define DOM_BERYTUSSECUREREMOTEPASSWORDCHALLENGE_H_

#include "mozilla/dom/BerytusChallenge.h"
#include "mozilla/dom/BerytusChallengeBinding.h"

namespace mozilla::dom {

class BerytusSecureRemotePasswordChallenge final : public BerytusChallenge {
public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusSecureRemotePasswordChallenge(
      nsIGlobalObject* aGlobal,
      const nsAString& aID,
      BerytusSecureRemotePasswordChallengeParameters&& aParameters);
protected:
  ~BerytusSecureRemotePasswordChallenge();
  BerytusSecureRemotePasswordChallengeParameters mParameters;
  void CacheParameters(JSContext* aCx, ErrorResult& aRv) override;
  bool PayloadToJSValue(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aSrc,
    JS::MutableHandle<JS::Value> aRetVal
  );
public:
  BerytusSecureRemotePasswordChallengeParameters const& Parameters() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  already_AddRefed<Promise> SelectSecurePassword(
      JSContext* aCx,
      ErrorResult& aRv);

  already_AddRefed<Promise> ExchangePublicKeys(
      JSContext* aCx,
      const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aWebAppServerPublicKeyB,
      ErrorResult& aRv);

  already_AddRefed<Promise> ComputeClientProof(
      JSContext* aCx,
      const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aSalt,
      ErrorResult& aRv);

  already_AddRefed<Promise> VerifyServerProof(
      JSContext* aCx,
      const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aServerProofM2,
      ErrorResult& aRv);

  already_AddRefed<Promise> AbortWithInvalidProofError(
      JSContext* aCx,
      ErrorResult& aRv
  );

  static already_AddRefed<BerytusSecureRemotePasswordChallenge> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusSecureRemotePasswordChallengeParameters&  aParameters,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif