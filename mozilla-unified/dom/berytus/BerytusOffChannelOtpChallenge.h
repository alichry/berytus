/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSOFFCHANNELOTPCHALLENGE_H_
#define DOM_BERYTUSOFFCHANNELOTPCHALLENGE_H_

#include "mozilla/dom/BerytusChallenge.h"

namespace mozilla::dom {

class BerytusOffChannelOtpChallenge final : public BerytusChallenge {
public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusOffChannelOtpChallenge(nsIGlobalObject* aGlobal,
                   const nsAString& aID);
protected:
  ~BerytusOffChannelOtpChallenge();
  void CacheParameters(JSContext* aCx, ErrorResult& aRv) override;

public:
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  already_AddRefed<Promise> GetOtp(JSContext* aCx,
                                              const nsAString& aForeignIdentityFieldIds,
                                              ErrorResult& aRv);
  already_AddRefed<Promise> AbortWithIncorrectOtpError(
      JSContext* aCx,
      ErrorResult& aRv
  );

  static already_AddRefed<BerytusOffChannelOtpChallenge> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif