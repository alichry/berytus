/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_
#define DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

class BerytusKeyAgreementParameters final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                                            public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusKeyAgreementParameters)

public:
  BerytusKeyAgreementParameters(
    nsIGlobalObject* aGlobal,
    const nsAString& aSessionId,
    const nsAString& aScmX25519Key,
    const nsAString& aWebAppX25519Key,
    const nsAString& aHkdfHash,
    const nsTArray<uint8_t>& aHkdfSalt,
    const nsTArray<uint8_t>& aHkdfInfo,
    const uint16_t& aAesKeyLength
  );

protected:
  ~BerytusKeyAgreementParameters();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  const nsString mSessionId;
  const nsString mScmX25519Key;
  const nsString mWebAppX25519Key;
  const nsString mHkdfHash;
  nsTArray<uint8_t> mHkdfSalt;
  nsTArray<uint8_t> mHkdfInfo;
  const uint16_t mAesKeyLength;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  void GetSessionId(nsString& aRetVal) const;

  void GetScmX25519Key(nsString& aRetVal) const;

  void GetWebAppX25519Key(nsString& aRetVal) const;

  void GetHkdfHash(nsString& aRetVal) const;

  void GetHkdfSalt(JSContext* cx, JS::MutableHandle<JSObject*> aRetVal);
  [[nodiscard]] nsresult GetHkdfSaltAsBase64(nsACString& aRetVal) const;

  void GetHkdfInfo(JSContext* cx, JS::MutableHandle<JSObject*> aRetVal);
  [[nodiscard]] nsresult GetHkdfInfoAsBase64(nsACString& aRetVal) const;

  uint16_t AesKeyLength() const;

  void ToJSON(nsString& aRetVall, ErrorResult& aRv) const;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_
