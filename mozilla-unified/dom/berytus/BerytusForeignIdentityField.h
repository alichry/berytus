/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSFOREIGNIDENTITYFIELD_H_
#define DOM_BERYTUSFOREIGNIDENTITYFIELD_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusForeignIdentityFieldOptions

namespace mozilla::dom {

class StringOrBerytusEncryptedPacket;

class BerytusForeignIdentityField final : public BerytusField
{
public:
  using BerytusField::ValueUnion;

public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusForeignIdentityField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusForeignIdentityFieldOptions&& aFieldOptions
  );

protected:
  BerytusForeignIdentityField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusForeignIdentityFieldOptions&& aFieldOptions,
    Nullable<ValueUnion>&& aFieldValue
  );
  ~BerytusForeignIdentityField();
  BerytusForeignIdentityFieldOptions mOptions;

  bool IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const override;

  void CacheOptions(JSContext* aCx, ErrorResult& aRv) override;
public:
  BerytusForeignIdentityFieldOptions const& Options() const;
  nsIGlobalObject* GetParentObject() const;
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusForeignIdentityField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusForeignIdentityFieldOptions& aOptions,
    const Optional<StringOrBerytusEncryptedPacket>& aDesiredValue,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSFOREIGNIDENTITYFIELD_H_
