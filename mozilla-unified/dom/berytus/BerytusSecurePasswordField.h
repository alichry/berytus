/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSECUREPASSWORDFIELD_H_
#define DOM_BERYTUSSECUREPASSWORDFIELD_H_

#include "BerytusSecurePasswordFieldValue.h"
#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusSecurePasswordFieldOptions

namespace mozilla::dom {

class BerytusSecurePasswordField final : public BerytusField
{
public:
  using BerytusField::ValueUnion;

public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusSecurePasswordField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusSecurePasswordFieldOptions&& aFieldOptions
  );

protected:
  BerytusSecurePasswordField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusSecurePasswordFieldOptions&& aFieldOptions,
    Nullable<ValueUnion>&& aFieldValue
  );
  ~BerytusSecurePasswordField();
  BerytusSecurePasswordFieldOptions mOptions;

  bool IsValueValid(JSContext* aCx, const Nullable<ValueUnion>& aValue) const override;

  void CacheOptions(JSContext* aCx, ErrorResult& aRv) override;
public:
  BerytusSecurePasswordFieldOptions const& Options() const;
  nsIGlobalObject* GetParentObject() const;
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusSecurePasswordField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusSecurePasswordFieldOptions& aOptions,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSSECUREPASSWORDFIELD_H_
