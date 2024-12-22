/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSANONYMOUSWEBAPPACTOR_H_
#define DOM_BERYTUSANONYMOUSWEBAPPACTOR_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIURI.h"
#include "nsWrapperCache.h"
#include "BerytusWebAppActor.h"

namespace mozilla::dom {

class BerytusAnonymousWebAppActor final : public BerytusWebAppActor
{
public:
  //NS_DECL_ISUPPORTS_INHERITED
  //NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)
  NS_DECL_ISUPPORTS_INHERITED
  NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)

public:
  BerytusAnonymousWebAppActor(nsIGlobalObject *aGlobal);
  BerytusAnonymousWebAppActor(const BerytusAnonymousWebAppActor& rhs);
  BerytusWebAppActorType Type() const override;
  nsIURI* GetOriginalURI(nsresult& aRv) const;
  nsIURI* GetCurrentURI(nsresult& aRv) const;

protected:
  ~BerytusAnonymousWebAppActor();

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusAnonymousWebAppActor> Constructor(
    const GlobalObject& aGlobal
  );

  already_AddRefed<BerytusWebAppActor> Clone() const override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSANONYMOUSWEBAPPACTOR_H_
