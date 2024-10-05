/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSKEYFIELDVALUE_H_
#define DOM_BERYTUSKEYFIELDVALUE_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusDataVariant.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusKeyFieldBinding.h" // ArrayBufferOrBerytusEncryptedPacket & OwningArrayBufferOrBerytusEncryptedPacket

namespace mozilla::dom {

class BerytusKeyFieldValue final : public BerytusFieldValueDictionary
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusKeyFieldValue, BerytusFieldValueDictionary)

protected:
  BerytusKeyFieldValue(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusDataVariant>& aPublicKey
  );
  ~BerytusKeyFieldValue();
  RefPtr<BerytusDataVariant> mPublicKey;

public:
  static already_AddRefed<BerytusKeyFieldValue> Create(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusDataVariant>& aPublicKey,
    nsresult* aOutRes
  );
  BerytusFieldType Type() override;
  
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  mozIAuthRealmFieldValue* ToAuthRealmFieldValue(nsresult* aOutRes) override;

  void GetPublicKey(
    JSContext* aCx,
    OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
    ErrorResult& aRv
  ) const;

};

} // namespace mozilla::dom

#endif // DOM_BERYTUSKEYFIELDVALUE_H_
