/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSFIELD_H_
#define DOM_BERYTUSFIELD_H_

#include "BerytusChannel.h"
#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldBinding.h" // BerytusFieldType
#include "mozilla/Variant.h"

namespace mozilla::dom {

class BerytusDataVariant;

class BerytusField : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                     public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */,
                     public BerytusChannel::Attachable
{
public:
  using ValueUnion = OwningStringOrBerytusEncryptedPacketOrBerytusFieldValueDictionary;

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusField)

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  void GetId(nsString& aRetVal) const;

  BerytusFieldType Type() const;

  virtual bool IsValueValid(const Nullable<ValueUnion>& aValue) const = 0;

  void SetValue(JSContext* aCx,
                const Nullable<ValueUnion>& aValue,
                ErrorResult& aRv);

  void GetValue(JSContext* aCx,
                        Nullable<ValueUnion>& aRetVal,
                        ErrorResult& aRv) const;

  Nullable<ValueUnion> const& GetValue() const;

  void GetOptions(JSContext* aCx,
                  JS::MutableHandle<JSObject*> aRetVal,
                  ErrorResult& aRv);

  void ToJSON(JSContext* aCx,
              JS::MutableHandle<JSObject*> aRetVal,
              ErrorResult& aRv);
 protected:
  BerytusField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    const BerytusFieldType& aFieldType,
    Nullable<ValueUnion>&& aFieldValue
  );
  virtual ~BerytusField();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  const nsString mFieldId;
  const BerytusFieldType mFieldType;
  Nullable<ValueUnion> mFieldValue;
  JS::Heap<JSObject*> mCachedOptions;

  virtual void SetValueImpl(JSContext* aCx,
                const Nullable<ValueUnion>& aValue,
                ErrorResult& aRv);

  virtual void CacheOptions(JSContext* aCx, ErrorResult& aRv) = 0;

  void AddValueToJSON(JSContext* aCx,
                      JS::Handle<JSObject*> aObj,
                      ErrorResult& aRv);
private:
  JS::Heap<JSObject*> mCachedJson;

  void AddFieldMetadataToCachedJSON(JSContext* aCx, ErrorResult& aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSFIELD_H_
