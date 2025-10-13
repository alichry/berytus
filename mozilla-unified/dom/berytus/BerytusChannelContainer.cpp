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
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(BerytusChannelContainer)
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

void BerytusChannelContainer::HandleFetchRequest(
      nsPIDOMWindowInner* aWindow,
      SafeRefPtr<InternalRequest>& aRequest,
      const fetch::OwningBodyInit& aReqBody,
      ErrorResult& aRv) {
  MOZ_ASSERT(aWindow);
  if (!aWindow) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  auto* navigator = aWindow->Navigator();
  MOZ_ASSERT(navigator);
  RefPtr<BerytusChannelContainer> impl =
    navigator->BerytusChannel();
  MOZ_ASSERT(impl);
  return impl->HandleFetchRequest(aRequest, aReqBody, aRv);
}

void BerytusChannelContainer::HandleHttpFetch(
    nsPIDOMWindowInner* aWindow,
    SafeRefPtr<InternalRequest>& aRequest,
    nsCOMPtr<nsIChannel>& aChannel,
    ErrorResult& aRv
) {
  MOZ_ASSERT(aWindow);
  if (!aWindow) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  auto* navigator = aWindow->Navigator();
  MOZ_ASSERT(navigator);
  RefPtr<BerytusChannelContainer> impl =
    navigator->BerytusChannel();
  MOZ_ASSERT(impl);
  return impl->HandleHttpFetch(aRequest, aChannel, aRv);
}

void BerytusChannelContainer::HandleFetchRequest(
      SafeRefPtr<InternalRequest>& aRequest,
      const fetch::OwningBodyInit& aReqBody,
      ErrorResult& aRv) {
  // Here, we try to unwrap the body, and we
  // save into a hashmap using the request address
  nsCString reqUrl;
  aRequest->GetURL(reqUrl);
  if (IsSignedUrl(reqUrl)) {
    // TODO: implement Subprocedure (2)
  }
  fetch::OwningBodyInit newReqBody;
  /* Subprocedure (1): */
  bool unmasked = BerytusEncryptedPacket::TryUnmaskAnyPacketInFetchBody(
      aReqBody, newReqBody, reqUrl, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  if (!unmasked) {
    return;
  }
  nsCOMPtr<nsIInputStream> stream;
  nsAutoCString contentTypeWithCharset;
  uint64_t contentLength = 0;
  aRv = ExtractByteStreamFromBody(newReqBody, getter_AddRefs(stream),
                                  contentTypeWithCharset, contentLength);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }

  
}

void HandleHttpFetch(
    SafeRefPtr<InternalRequest>& aRequest,
    nsCOMPtr<nsIChannel>& aChannel,
    ErrorResult& aRv
) {
  // Here, we check if we have previously unwrapped
  // the body containing a BerytusEncryptedPacket,
  // and if so, we replace the channel's upload body with it.

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
