/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSKEYFIELD_H_
#define DOM_BERYTUSKEYFIELD_H_

#include "BerytusKeyFieldValue.h"
#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusKeyFieldOptions

namespace mozilla::dom {

class BerytusKeyField final : public BerytusField
{
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusKeyField, BerytusField)

public:
    using BerytusField::BerytusField;
    using BerytusField::ValueType;

  bool HasValue() const override;

  void GetValue(JSContext* aCx,
                Nullable<ValueType>& aRetVal,
                ErrorResult& aRv) const override;
protected:
  BerytusKeyField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    const BerytusFieldType& aFieldType,
    JS::Handle<JSObject*> aOptions
  );
  ~BerytusKeyField();
  RefPtr<BerytusKeyFieldValue> mValue;

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

  static already_AddRefed<BerytusKeyField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusKeyFieldOptions& aOptions,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSKEYFIELD_H_
