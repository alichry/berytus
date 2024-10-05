/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSHAREDKEYFIELDVALUE_H_
#define DOM_BERYTUSSHAREDKEYFIELDVALUE_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusDataVariant.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h" // ArrayBufferOrBerytusEncryptedPacket & OwningArrayBufferOrBerytusEncryptedPacket

namespace mozilla::dom {

class BerytusSharedKeyFieldValue final : public BerytusFieldValueDictionary
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)

  BerytusSharedKeyFieldValue(const BerytusSharedKeyFieldValue& aOther);

protected:
  BerytusSharedKeyFieldValue(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusDataVariant>& aPrivateKey
  );
  ~BerytusSharedKeyFieldValue();
  RefPtr<BerytusDataVariant> mPrivateKey;

public:

  static already_AddRefed<BerytusSharedKeyFieldValue> Create(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusDataVariant>& aPrivateKey,
    nsresult* aOutRes
  );

  static already_AddRefed<BerytusSharedKeyFieldValue> Constructor(
    const GlobalObject& aGlobal,
    const ArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket& aPrivateKey,
    ErrorResult& aRv
  );

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  BerytusFieldType Type() override;
  
  mozIAuthRealmFieldValue* ToAuthRealmFieldValue(nsresult* aOutRes) override;

  void GetPrivateKey(
    JSContext* aCx,
    OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
    ErrorResult& aRv
) const;

};

} // namespace mozilla::dom

#endif // DOM_BERYTUSKEYFIELDVALUE_H_
