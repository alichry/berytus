/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSHAREDKEYFIELD_H_
#define DOM_BERYTUSSHAREDKEYFIELD_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BerytusSharedKeyFieldValue.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusSharedKeyFieldOptions

namespace mozilla::dom {

class UTF8StringOrBerytusEncryptedPacket;

class BerytusSharedKeyField final : public BerytusField
{
public:
  using BerytusField::BerytusField;
public:
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusSharedKeyField, BerytusField)

protected:
  ~BerytusSharedKeyField();

public:
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  already_AddRefed<BerytusField> Clone() override;

  mozIAuthRealmRegistrationBasicFieldCreationOptions::FieldType AuthRealmFieldType() const override;

  static already_AddRefed<BerytusSharedKeyField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusSharedKeyFieldOptions& aOptions,
    const Optional<NonNull<BerytusSharedKeyFieldValue>>& aDesiredValue,
    ErrorResult& aRv
  );

  bool ValidateValue(BerytusDataVariant* aValue) override;

  void GetOptions(JSContext* cx, JS::MutableHandle<JS::Value> aRetVal) const;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSIDENTITYFIELD_H_
