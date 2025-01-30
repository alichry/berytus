/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSDIGITALSIGNATURECHALLENGE_H_
#define DOM_BERYTUSDIGITALSIGNATURECHALLENGE_H_

#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusChallenge.h"

namespace mozilla::dom {

class BerytusDigitalSignatureChallenge final : public BerytusChallenge {
public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusDigitalSignatureChallenge(nsIGlobalObject* aGlobal,
                                   const nsAString& aID);
protected:
  ~BerytusDigitalSignatureChallenge();
  void CacheParameters(JSContext* aCx, ErrorResult& aRv) override;
  bool PayloadToJSValue(
    JSContext* aCx,
    const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aSrc,
    JS::MutableHandle<JS::Value> aRetVal
  );
public:
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  already_AddRefed<Promise> SelectKey(JSContext* aCx,
                                      const nsAString& aKeyFieldId,
                                      ErrorResult& aRv);
  already_AddRefed<Promise> SignNonce(JSContext* aCx,
                                      const ArrayBufferOrArrayBufferViewOrBerytusEncryptedPacket& aNonce,
                                      ErrorResult& aRv);
  already_AddRefed<Promise> AbortWithPublicKeyMismatchError(
      JSContext* aCx,
      ErrorResult& aRv
  );
  already_AddRefed<Promise> AbortWithInvalidSignatureError(
      JSContext* aCx,
      ErrorResult& aRv
  );

  static already_AddRefed<BerytusDigitalSignatureChallenge> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom


#endif