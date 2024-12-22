/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSFIELDMAP_H_
#define DOM_BERYTUSFIELDMAP_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/Record.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

class BerytusFieldMap final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                              public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusFieldMap)

public:
  BerytusFieldMap(nsIGlobalObject* aGlobal);

protected:
  ~BerytusFieldMap();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsTArray<RefPtr<BerytusField>> mFields;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  const nsTArray<RefPtr<BerytusField>>& List() const;

  void AddField(BerytusField* aField, ErrorResult& aRv);
  /* caller should aRv if it has failed first, before checking the returned bool */
  bool HasField(const nsAString& aFieldId, ErrorResult& aRv);
  already_AddRefed<BerytusField> GetField(const nsAString& aFieldId, ErrorResult& aRv);

  void ToRecord(Record<nsString, RefPtr<BerytusField>>& aOutRecord);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSFIELDMAP_H_
