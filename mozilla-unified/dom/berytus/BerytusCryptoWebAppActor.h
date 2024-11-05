/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCRYPTOWEBAPPACTOR_H_
#define DOM_BERYTUSCRYPTOWEBAPPACTOR_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BerytusWebAppActor.h"

namespace mozilla::dom {

class BerytusCryptoWebAppActor final : public BerytusWebAppActor
{
public:
  //NS_DECL_ISUPPORTS_INHERITED
  ////NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusCryptoWebAppActor, BerytusWebAppActor)
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusCryptoWebAppActor, BerytusWebAppActor)

public:
  BerytusCryptoWebAppActor(nsIGlobalObject* aGlobal, const nsAString& aEd25519Key);
  BerytusCryptoWebAppActor(const BerytusCryptoWebAppActor& rhs);
  BerytusWebAppActorType Type() const override;

protected:
  ~BerytusCryptoWebAppActor();
  const nsString mEd25519Key;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusCryptoWebAppActor> Constructor(
    const GlobalObject& aGlobal,
    const nsAString& aEd25519Key,
    ErrorResult& aRv
  );

  void GetEd25519Key(nsString& aRetVal) const;

  already_AddRefed<BerytusWebAppActor> Clone() const override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCRYPTOWEBAPPACTOR_H_
