/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSSHAREDKEYFIELD_H_
#define DOM_BERYTUSSHAREDKEYFIELD_H_

#include "BerytusSharedKeyFieldValue.h"
#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusFieldOptionsBinding.h" // BerytusSharedKeyFieldOptions

namespace mozilla::dom {

class BerytusSharedKeyField final : public BerytusField
{
public:
  using BerytusField::ValueUnion;

public:
  NS_DECL_ISUPPORTS_INHERITED
  BerytusSharedKeyField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusSharedKeyFieldOptions&& aFieldOptions
  );

protected:
  BerytusSharedKeyField(
    nsIGlobalObject* aGlobal,
    const nsAString& aFieldId,
    BerytusSharedKeyFieldOptions&& aFieldOptions,
    Nullable<ValueUnion>&& aFieldValue
  );
  ~BerytusSharedKeyField();
  BerytusSharedKeyFieldOptions mOptions;

  void CacheOptions(JSContext* aCx, ErrorResult& aRv) override;
public:
  BerytusSharedKeyFieldOptions const& Options() const;
  nsIGlobalObject* GetParentObject() const;
  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  bool IsValueValid(const Nullable<ValueUnion>& aValue) const override;

  static already_AddRefed<BerytusSharedKeyField> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aId,
    const BerytusSharedKeyFieldOptions& aOptions,
    const Optional<NonNull<BerytusSharedKeyFieldValue>>& aDesiredValue,
    ErrorResult& aRv
  );
};

} // namespace mozilla::dom


#endif // DOM_BERYTUSSHAREDKEYFIELD_H_
