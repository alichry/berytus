/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusChannelContainer.h"
#include "mozilla/dom/BerytusChannelContainerBinding.h"
#include "mozilla/dom/Navigator.h"
#include "mozilla/dom/InternalRequest.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusChannelContainer, mGlobal, mPacketObservers)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusChannelContainer)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusChannelContainer)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusChannelContainer)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusChannelContainer::BerytusChannelContainer(nsIGlobalObject* aGlobal)
  : mGlobal(aGlobal)
{
    // Add |MOZ_COUNT_CTOR(BerytusChannelContainer);| for a non-refcounted object.
}

BerytusChannelContainer::~BerytusChannelContainer()
{
    // Add |MOZ_COUNT_DTOR(BerytusChannelContainer);| for a non-refcounted object.
}

nsIGlobalObject* BerytusChannelContainer::GetParentObject() const {
  return mGlobal;
}

void BerytusChannelContainer::HoldObserver(RefPtr<PacketObserver>& aObserver) {
  mPacketObservers.AppendElement(aObserver);
}

already_AddRefed<BerytusChannelContainer> BerytusChannelContainer::GetInstance(
    nsPIDOMWindowInner* aWindow) {
  MOZ_ASSERT(aWindow);
  if (NS_WARN_IF(!aWindow)) {
    return nullptr;
  }
  auto* navigator = aWindow->Navigator();
  MOZ_ASSERT(navigator);
  if (NS_WARN_IF(!navigator)) {
    return nullptr;
  }
  return navigator->BerytusChannel();
}

bool BerytusChannelContainer::IsSignedUrl(const nsCString& aReqUrl) const {
  // TODO(berytus): Implement this. We need to get access
  // to the active channel, and the key agreement parameters
  // (if any) to check if the URL is signed.
  return false;
}

JSObject*
BerytusChannelContainer::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusChannelContainer_Binding::Wrap(aCx, this, aGivenProto);
}


} // namespace mozilla::dom
