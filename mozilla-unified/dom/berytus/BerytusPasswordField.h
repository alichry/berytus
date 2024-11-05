/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSPASSWORDFIELD_H_
#define DOM_BERYTUSPASSWORDFIELD_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusPasswordFieldOptions

namespace mozilla::dom {

class StringOrBerytusEncryptedPacket;

class BerytusPasswordField final : public BerytusField
{
public:
  using BerytusField::BerytusField;
  using BerytusField::ValueType;

public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusPasswordField, BerytusField)

  BerytusPasswordField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    JS::Handle<JSObject*> aOptions,
    BerytusEncryptedPacket* aValue
  );

  BerytusPasswordField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    JS::Handle<JSObject*> aOptions,
    const nsAString& aValue
  );

  bool HasValue() const override;

  void GetValue(JSContext* aCx,
                        Nullable<ValueType>& aRetVal,
                        ErrorResult& aRv) const override;
protected:
  BerytusPasswordField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    const BerytusFieldType& aFieldType,
    JS::Handle<JSObject*> aOptions,
    BerytusEncryptedPacket* aAsEncrypted,
    const nsAString& aAsString
  );

  ~BerytusPasswordField();
  RefPtr<BerytusEncryptedPacket> mAsEncrypted;
  nsString mAsString;

  void AddValueToJSON(JSContext* aCx,
                      JS::Handle<JSObject*> aObj,
                      ErrorResult& aRv) override;
  void SetValueImpl(JSContext* aCx,
                            const Nullable<ValueType>& aValue,
                            ErrorResult& aRv) override;
public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusPasswordField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusPasswordFieldOptions& aOptions,
    const Optional<StringOrBerytusEncryptedPacket>& aDesiredValue,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSPASSWORDFIELD_H_
