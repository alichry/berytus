/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSUSERATTRIBUTE_H_
#define DOM_BERYTUSUSERATTRIBUTE_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/UnionTypes.h" /* OwningStringOrArrayBufferOrBerytusEncryptedPacket */
#include "nsCycleCollectionParticipant.h"
#include "nsISupports.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusUserAttributeBinding.h" /* OwningStringOrArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket */
#include "mozilla/dom/BerytusChannel.h"

namespace mozilla::dom {

class BerytusUserAttribute : public nsISupports, /* or NonRefcountedDOMObject if this is a non-refcounted object */
                             public nsWrapperCache, /* Change wrapperCache in the binding configuration if you don't want this */
                             public BerytusChannel::Attachable
{
public:
  using ValueType = OwningStringOrArrayBufferOrBerytusEncryptedPacket;
  using SourceValueType = OwningStringOrArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket;
  using JSONValueType = nsAString;

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusUserAttribute)
protected:
  BerytusUserAttribute(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo
  );
  virtual ~BerytusUserAttribute();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  const nsString mId;
  const nsString mMimeType;
  const nsString mInfo;
  ValueType mValue;

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

  virtual bool CanSetValue(const SourceValueType& aVal) const = 0;
  virtual void SetValue(JSContext* aCx,
                        const SourceValueType& aVal,
                        ErrorResult& aRv) = 0;

  void ToJSON(BerytusUserAttributeJSON& aRetVal,
              ErrorResult& aRv) const;

  virtual BerytusUserAttributeValueEncodingType ValueEncodingType() const = 0;

  static already_AddRefed<BerytusUserAttribute> Create(
      JSContext* aCx,
      nsIGlobalObject* aGlobal,
      RefPtr<BerytusChannel>& aChannel, /* can be nullptr */
      const nsAString& aId,
      const nsAString& aMimeType,
      const nsAString& aInfo,
      const SourceValueType& aValue,
      ErrorResult& aRv
  );
protected:
  virtual void PopulateValueInJSON(JSONValueType& aValue,
                                   ErrorResult& aRv) const = 0;
};

template <typename T>
class BerytusUserAttributeImpl final : public BerytusUserAttribute {};

template<>
class BerytusUserAttributeImpl<nsString> : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
protected:
  ~BerytusUserAttributeImpl();
public:
  BerytusUserAttributeImpl(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo
  );
  bool CanSetValue(const SourceValueType& aVal) const override;
  void SetValue(JSContext* aCx,
                const SourceValueType& aVal,
                ErrorResult& aRv) override;
  void SetValueInternal(const nsString& aValue);
  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
  BerytusUserAttributeValueEncodingType ValueEncodingType() const override;
protected:
  void PopulateValueInJSON(JSONValueType& aValue,
                           ErrorResult& aRv) const override;
};

template<>
class BerytusUserAttributeImpl<ArrayBuffer> final : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
protected:
  ~BerytusUserAttributeImpl();
public:
  BerytusUserAttributeImpl(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo
  );
  bool CanSetValue(const SourceValueType& aVal) const override;
  void SetValue(JSContext* aCx,
                const SourceValueType& aVal,
                ErrorResult& aRv) override;
  void SetValueInternal(const ArrayBuffer& aValue, ErrorResult& aRv);
  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
  BerytusUserAttributeValueEncodingType ValueEncodingType() const override;
protected:
  void PopulateValueInJSON(JSONValueType& aValue,
                           ErrorResult& aRv) const override;
};

template<>
class BerytusUserAttributeImpl<BerytusEncryptedPacket> final : public BerytusUserAttribute {
public:
  NS_DECL_ISUPPORTS_INHERITED
protected:
  ~BerytusUserAttributeImpl();
public:
  BerytusUserAttributeImpl(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo
  );
  bool CanSetValue(const SourceValueType& aVal) const override;
  void SetValue(JSContext* aCx,
                const SourceValueType& aVal,
                ErrorResult& aRv) override;
  void SetValueInternal(const RefPtr<BerytusEncryptedPacket>& aValue,
                        ErrorResult& aRv);
  void GetValue(
    JSContext* aCx,
    ValueType& aRetVal,
    ErrorResult& aRv
  ) const override;
  BerytusUserAttributeValueEncodingType ValueEncodingType() const override;
protected:
  void PopulateValueInJSON(JSONValueType& aValue,
                           ErrorResult& aRv) const override;
};


} // namespace mozilla::dom

#endif // DOM_BERYTUSUSERATTRIBUTE_H_
