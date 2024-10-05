/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSFIELD_H_
#define DOM_BERYTUSFIELD_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsVariant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldBinding.h" // BerytusFieldType
#include "mozilla/Variant.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusFieldValueDictionary.h"

namespace mozilla::dom {

class BerytusDataVariant;

class BerytusField : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                     public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  using ValueType = OwningStringOrArrayBufferOrBerytusEncryptedPacketOrBerytusFieldValueDictionary;

  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusField)

public:
  BerytusField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    const BerytusFieldType& aFieldType,
    JS::Handle<JSObject*> aOptions
  );
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  void GetId(nsString& aRetVal) const;

  BerytusFieldType Type() const;

  virtual bool HasValue() const = 0;

  void SetValue(JSContext* aCx,
                const Nullable<ValueType>& aValue,
                ErrorResult& aRv);

  virtual void GetValue(JSContext* aCx,
                        Nullable<ValueType>& aRetVal,
                        ErrorResult& aRv) const = 0;

  void GetOptions(JSContext* aCx, JS::MutableHandle<JSObject*> aRetVal) const;

  void ToJSON(JSContext* aCx,
              JS::MutableHandle<JSObject*> aRetVal,
              ErrorResult& aRv);

protected:
  virtual ~BerytusField();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  const nsString mFieldId;
  const BerytusFieldType mFieldType;
  JS::Heap<JSObject*> mOptions;

  virtual void AddValueToJSON(JSContext* aCx,
                      JS::Handle<JSObject*> aObj,
                      ErrorResult& aRv) = 0;

  virtual void SetValueImpl(JSContext* aCx,
                            const Nullable<ValueType>& aValue,
                            ErrorResult& aRv) = 0;
private:
  JS::Heap<JSObject*> mCachedJson;

  void AddFieldMetadataToCachedJSON(JSContext* aCx, ErrorResult& aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSFIELD_H_
