/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSECUREPASSWORDFIELDVALUE_H_
#define DOM_BERYTUSSECUREPASSWORDFIELDVALUE_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusDataVariant.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"

namespace mozilla::dom {

class OwningStringOrBerytusEncryptedPacket;
class StringOrBerytusEncryptedPacket;

} // namespace mozilla

namespace mozilla::dom {

class BerytusSecurePasswordFieldValue final : public BerytusFieldValueDictionary
{
public:
  //NS_DECL_ISUPPORTS_INHERITED
  //NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusSecurePasswordFieldValue, BerytusFieldValueDictionary)

public:
  BerytusSecurePasswordFieldValue(
    nsIGlobalObject* aGlobal,
    const nsAString& aSaltBase64,
    const nsAString& aVerifierBase64
  );

protected:
  ~BerytusSecurePasswordFieldValue();
  RefPtr<BerytusDataVariant> mSalt;
  RefPtr<BerytusDataVariant> mVerifier;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  BerytusFieldType Type() override;

  mozIAuthRealmFieldValue* ToAuthRealmFieldValue(nsresult* aOutRes) override;

  void GetSalt(OwningStringOrBerytusEncryptedPacket& aRetVal) const;

  void GetVerifier(OwningStringOrBerytusEncryptedPacket& aRetVal) const;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSSECUREPASSWORDFIELDVALUE_H_
