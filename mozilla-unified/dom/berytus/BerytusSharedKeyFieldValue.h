/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSHAREDKEYFIELDVALUE_H_
#define DOM_BERYTUSSHAREDKEYFIELDVALUE_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"
#include "mozilla/dom/BerytusSharedKeyFieldBinding.h" // ArrayBufferOrBerytusEncryptedPacket & OwningArrayBufferOrBerytusEncryptedPacket

namespace mozilla::dom {

class BerytusSharedKeyFieldValue final : public BerytusFieldValueDictionary
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusSharedKeyFieldValue, BerytusFieldValueDictionary)

  BerytusSharedKeyFieldValue(nsIGlobalObject* aGlobal,
                             const RefPtr<BerytusEncryptedPacket>& aAsEncrypted);
  BerytusSharedKeyFieldValue(nsIGlobalObject* aGlobal,
                             CryptoBuffer&& aAsBuffer);
protected:
  ~BerytusSharedKeyFieldValue();

  RefPtr<BerytusEncryptedPacket> mAsEncrypted;
  CryptoBuffer mAsBuffer;

  JS::Heap<JSObject*> mCachedBuffer;

public:
  static already_AddRefed<BerytusSharedKeyFieldValue> Create(
    nsIGlobalObject* aGlobal,
    const RefPtr<BerytusEncryptedPacket>& aPrivateKeyAsEncryptedPacket,
    nsresult* aOutRes
  );
  static already_AddRefed<BerytusSharedKeyFieldValue> Create(
    nsIGlobalObject* aGlobal,
    const ArrayBuffer& aPrivateKeyAsArrayBuffer,
    nsresult* aOutRes
  );
  BerytusFieldType Type() override;

  nsIGlobalObject* GetParentObject() const;
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  void GetPrivateKey(
    JSContext* aCx,
    OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
    ErrorResult& aRv
  );

  void ToJSON(JSContext* aCx,
              JS::MutableHandle<JS::Value> aRetVal,
              ErrorResult& aRv);

  static already_AddRefed<BerytusSharedKeyFieldValue> Constructor(
    GlobalObject& aGlobal,
    const mozilla::dom::ArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket& aPrivateKeyValue,
    ErrorResult& aRv
  );

  already_AddRefed<BerytusSharedKeyFieldValue> Clone(nsresult* aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSSHAREDKEYFIELDVALUE_H_
