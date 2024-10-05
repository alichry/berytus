/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSUSERATTRIBUTE_H_
#define DOM_BERYTUSUSERATTRIBUTE_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h" // OwningStringOrArrayBufferOrBerytusEncryptedPacket
#include "mozilla/dom/BerytusUserAttributeBinding.h"

namespace mozilla::dom {

class BerytusUserAttribute : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                             public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  using ValueType = OwningStringOrArrayBufferOrBerytusEncryptedPacket;
  using SourceValueType = OwningStringOrArrayBufferOrBerytusEncryptedPacket;
  using JSONValueType = OwningStringOrBerytusEncryptedPacketJSON;

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusUserAttribute)

public:
  BerytusUserAttribute(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo
  );

protected:
  virtual ~BerytusUserAttribute();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  const nsString mId;
  const nsString mMimeType;
  const nsString mInfo;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  void GetId(nsString& aRetVal) const;

  void GetMimeType(nsString& aRetVal) const;

  void GetInfo(nsString& aRetVal) const;

  virtual void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const = 0;

  void ToJSON(BerytusUserAttributeJSON& aRetVal,
              ErrorResult& aErr) const;
protected:
  virtual void PopulateValueInJSON(JSONValueType& aRetVal,
                                   ErrorResult& aErr) const = 0;
};

class BerytusUserAttributeString final : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusUserAttributeString, BerytusUserAttribute)

  BerytusUserAttributeString(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo,
    const nsAString& aValue
  );

  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
protected:
  ~BerytusUserAttributeString();
  nsString mValue;

  void PopulateValueInJSON(JSONValueType& aRetVal,
                           ErrorResult& aErr) const override;
};

class BerytusUserAttributeArrayBuffer final : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusUserAttributeArrayBuffer, BerytusUserAttribute)
  
  BerytusUserAttributeArrayBuffer(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo,
    const ArrayBuffer& aValue
  );

  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
protected:
  ~BerytusUserAttributeArrayBuffer();
  JS::Heap<JSObject*> mValue;

  void PopulateValueInJSON(JSONValueType& aRetVal,
                           ErrorResult& aErr) const override;
};

class BerytusUserAttributeEncryptedPacket final : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusUserAttributeEncryptedPacket, BerytusUserAttribute)

  BerytusUserAttributeEncryptedPacket(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo,
    const RefPtr<BerytusEncryptedPacket>& aValue
  );

  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
protected:
  ~BerytusUserAttributeEncryptedPacket();
  RefPtr<BerytusEncryptedPacket> mValue;

  void PopulateValueInJSON(JSONValueType& aRetVal,
                           ErrorResult& aErr) const override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSUSERATTRIBUTE_H_
