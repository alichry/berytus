/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCHANNELCONTAINER_H_
#define DOM_BERYTUSCHANNELCONTAINER_H_

#include "js/TypeDecls.h"
#include "mozilla/ErrorResult.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"

namespace mozilla::dom {

class InternalRequest;
class OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
namespace fetch {
  using OwningBodyInit = OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
}

// we need to create a channel observer
// and we need to access the global/inner window's
// BerytusChannelContainer.

class BerytusChannelContainer final : public nsISupports,
                                      public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusChannelContainer)

public:
  BerytusChannelContainer(nsIGlobalObject* aGlobal);

  /**
   * This procedure is the main entrypoint for Berytus' E2E masking
   * of encrypted packet. In particular, it undertakes the following
   * subprocedures: (1) It attempts to detect a BerytusEncryptedPacket
   * in the request body, and if one was found, it attempts to unmask it
   * in a non-interceptible manner; (2) Additionally, regardless of the
   * outcome of the previous step, and if the request url  is a signed
   * url part of the key agreement parameters, it attempts to mask
   * any encrypted packets found in the response body, prior service
   * worker interception.
   */
  static void HandleFetchRequest(
      nsPIDOMWindowInner* aWindow,
      SafeRefPtr<InternalRequest>& aRequest,
      const fetch::OwningBodyInit& aReqBody,
      ErrorResult& aRv);
  // must be called by FetchDriver after creating
  // a channel, and before calling AsyncOpen
  static void HandleHttpFetch(
      nsPIDOMWindowInner* aWindow,
      SafeRefPtr<InternalRequest>& aRequest,
      nsCOMPtr<nsIChannel>& aChannel,
      ErrorResult& aRv
  );
  void HandleFetchRequest(
      SafeRefPtr<InternalRequest>& aRequest,
      const fetch::OwningBodyInit& aReqBody,
      ErrorResult& aRv);
  void HandleHttpFetch(
      SafeRefPtr<InternalRequest>& aRequest,
      nsCOMPtr<nsIChannel>& aChannel,
      ErrorResult& aRv
  );
  bool IsSignedUrl(const nsCString& aReqUrl) const;

protected:
  ~BerytusChannelContainer();
  nsCOMPtr<nsIGlobalObject> mGlobal;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCHANNELCONTAINER_H_
