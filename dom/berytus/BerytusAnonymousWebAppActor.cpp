/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusAnonymousWebAppActor.h"
#include "BerytusWebAppActor.h"
#include "mozilla/dom/BerytusAnonymousWebAppActorBinding.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/Document.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
// # error "If you don't have members that need cycle collection,
// # then remove all the cycle collection bits from this
// # implementation and the corresponding header.  If you do, you
// # want NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAnonymousWebAppActor,
// # BerytusWebAppActor, your, members, here)"
// NS_IMPL_ADDREF_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)
// NS_IMPL_RELEASE_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)
// NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusAnonymousWebAppActor)
// NS_INTERFACE_MAP_END_INHERITING(BerytusWebAppActor)

// NS_IMPL_ADDREF_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)
// NS_IMPL_RELEASE_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)
// NS_INTERFACE_MAP_BEGIN(BerytusAnonymousWebAppActor)
// NS_INTERFACE_MAP_END_INHERITING(BerytusWebAppActor)

NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusAnonymousWebAppActor, BerytusWebAppActor)
NS_IMPL_CYCLE_COLLECTION_INHERITED(BerytusAnonymousWebAppActor, BerytusWebAppActor)

BerytusAnonymousWebAppActor::BerytusAnonymousWebAppActor(nsIGlobalObject* aGlobal) : BerytusWebAppActor(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusAnonymousWebAppActor);| for a non-refcounted object.
}

BerytusAnonymousWebAppActor::BerytusAnonymousWebAppActor(
  const BerytusAnonymousWebAppActor& rhs
) : BerytusWebAppActor(rhs.mGlobal) { }

already_AddRefed<BerytusWebAppActor> BerytusAnonymousWebAppActor::Clone() const {
  RefPtr<BerytusAnonymousWebAppActor> cl = new BerytusAnonymousWebAppActor(*this);
  return cl.forget();
}

BerytusAnonymousWebAppActor::~BerytusAnonymousWebAppActor()
{
    // Add |MOZ_COUNT_DTOR(BerytusAnonymousWebAppActor);| for a non-refcounted object.
}

BerytusWebAppActorType BerytusAnonymousWebAppActor::Type() const {
  return BerytusWebAppActorType::OriginActor;
}

nsIURI* BerytusAnonymousWebAppActor::GetOriginalURI() const {
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    return nullptr;
  }
  Document* doc = inner->GetDoc();
  nsIURI* uri = doc->GetOriginalURI();
  return uri;
}
nsIURI* BerytusAnonymousWebAppActor::GetCurrentURI() const {
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    return nullptr;
  }
  Document* doc = inner->GetDoc();
  nsIURI* uri = doc->GetDocumentURI();
  return uri;
}

nsIGlobalObject* BerytusAnonymousWebAppActor::GetParentObject() const { return mGlobal; }

JSObject*
BerytusAnonymousWebAppActor::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusAnonymousWebAppActor_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusAnonymousWebAppActor> BerytusAnonymousWebAppActor::Constructor(
  const GlobalObject& aGlobal
)
{
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    return nullptr;
  }

  RefPtr<BerytusAnonymousWebAppActor> object = new BerytusAnonymousWebAppActor(global);
  return object.forget();
}

} // namespace mozilla::dom
