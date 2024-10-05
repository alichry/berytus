/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSUSERATTRIBUTEMAP_H_
#define DOM_BERYTUSUSERATTRIBUTEMAP_H_

#include "js/TypeDecls.h"
#include "mozilla/Attributes.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusUserAttribute.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

class BerytusUserAttributeMap final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                                      public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusUserAttributeMap)

public:
  BerytusUserAttributeMap(nsIGlobalObject* aGlobal);

protected:
  ~BerytusUserAttributeMap();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsTArray<RefPtr<BerytusUserAttribute>> mAttributes; // keep track of the attributes in an array as well. I am doing this to ensure the attribute is still referenced...

public:
  // TODO(berytus): Perhaps add a `FromUserAttributesJS` method?
  
  void AddAttribute(BerytusUserAttribute* aAttribute, ErrorResult& aRv);
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSUSERATTRIBUTEMAP_H_
