/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSWEBAPPACTOR_H_
#define DOM_BERYTUSWEBAPPACTOR_H_

#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

enum BerytusWebAppActorType {
  OriginActor,
  CryptoActor
};

class BerytusWebAppActor : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                           public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusWebAppActor)

public:
  virtual BerytusWebAppActorType Type() const = 0;

protected:
  BerytusWebAppActor(nsIGlobalObject* aGlobal);
  virtual ~BerytusWebAppActor();

  nsCOMPtr<nsIGlobalObject> mGlobal;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSWEBAPPACTOR_H_
