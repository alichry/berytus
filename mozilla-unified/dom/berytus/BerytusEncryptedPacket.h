/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSENCRYPTEDPACKET_H_
#define DOM_BERYTUSENCRYPTEDPACKET_H_

#include "mozilla/ErrorResult.h"
#include "mozilla/UniquePtr.h"
#include "mozilla/dom/BerytusX509Extension.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/Blob.h"
#include "mozilla/dom/InternalRequest.h"
#include "nsHashKeys.h"
#include "nsIGlobalObject.h"
#include "nsStringFwd.h"
#include "nsTArrayForwardDeclare.h"
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/berytus/HttpObserver.h"
#include "nsRefPtrHashtable.h"

namespace mozilla::dom {

class OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
namespace fetch {
  using OwningBodyInit = OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
}

class BerytusChannelContainer;
class BerytusChannel;

class BerytusEncryptedPacket : public Blob,
                               public BerytusChannel::BaseAttachable {
public:
  enum PacketType : uint8_t {
    JWE
  };

  Span<const uint8_t> Exposed() const;
  virtual PacketType Type() const = 0;
  virtual void SerializeExposedToString(nsACString& aValue, ErrorResult& aRv) const;

  already_AddRefed<Blob> Unmask(const nsCString& aReqUrl, ErrorResult& aRv);
  already_AddRefed<Blob> Unmask(nsIURI* aReqUrl, ErrorResult& aRv);

  bool HasBerytusEncryptedPacketInterface() const override;
  bool Attached() const override;
  void Attach(RefPtr<BerytusChannel>& aChannel, ErrorResult& aRv) override;
protected:
  struct Content {
    UniquePtr<uint8_t[]> mBuf = nullptr;
    uint64_t mLen = 0;
  };

  BerytusEncryptedPacket(nsIGlobalObject* aGlobal,
                         Content&& aExposedContent,
                         const bool& aConceal);
  virtual ~BerytusEncryptedPacket();
  nsIGlobalObject* mGlobal;
  Span<uint8_t> mExposedContent;
  bool mConcealed;
  /**
   * Note(berytus): Unfortunately, Blob's QI method is marked
   * as final; hence, we cannot define a new cycle collection
   * participant, and therefore we cannot include new members
   * to be cycle collected. Fortunately, UrlSearchExpression
   * does not have cycle collected members of its own but
   * it is ref-counted. Here, mUrlAllowlist would be a leaf
   * node. Hence, as long as we do not share the items
   * with class owners, nsTArray's destructor should be
   * sufficient, calling the destructor of RefPtr<..>.
   */
  nsTArray<RefPtr<berytus::UrlSearchExpression>> mUrlAllowlist;
  bool mAttached;

  already_AddRefed<Blob> Unmask(const nsCString& aReqUrl, bool& aUnmasked, ErrorResult& aRv);
  already_AddRefed<Blob> Unmask(nsIURI* aReqUrl, bool& aUnmasked, ErrorResult& aRv);
  virtual already_AddRefed<Blob> UnmaskImpl(ErrorResult& aRv) = 0;

  static bool CreateMaskContent(BerytusEncryptedPacket::Content& aMask);

  friend class BerytusChannelContainer;
  static bool TryUnmaskAnyPacketInFetchBody(
    const fetch::OwningBodyInit& aSrc,
    fetch::OwningBodyInit& aDest,
    const nsCString& aReqUrl,
    ErrorResult& aRv
  );

public:
  class RequestObserver final : public nsIObserver {
  public:
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_CLASS(RequestObserver)
    NS_DECL_NSIOBSERVER

    RequestObserver(RefPtr<BerytusEncryptedPacket>& aPacket);
  protected:
    ~RequestObserver();

    RefPtr<BerytusEncryptedPacket> mPacket;
    nsRefPtrHashtable<nsUint64HashKey, berytus::HttpObserver::UnmaskPacket> mDetectedChannels;
    nsRefPtrHashtable<nsPtrHashKey<InternalRequest>, berytus::HttpObserver::UnmaskPacket> mDetectedRequests;
  };
  /**
   * This procedure is the main entrypoint for Berytus' E2E masking
   * of encrypted packet. In particular, it undertakes the following
   * subprocedures: (1) It attempts to detect a BerytusEncryptedPacket
   * in the request body, and if one was found, it attempts to unmask it,
   * disabling service worker interception along the way; (2) Additionally,
   * regardless of the outcome of the previous step, it disables service
   * worker interception if the request url is a signed url part of
   * the key agreement parameters. This is to prepare for masking of
   * encrypted packets coming from the web app, avoiding service worker
   * interception before masking takes place.
   *
   */
  static void HandleFetchRequest(SafeRefPtr<InternalRequest>& aRequest,
                                 const fetch::OwningBodyInit& aReqBody,
                                 ErrorResult& aRv);

  /**
   * Here, if the request url is a signed url, this procedure
   * attempts to locate encrypted packets in the response body,
   * and it masks them, returning a masked encrypted packet that
   * would not be exposed on the DOM. Rather, it will be unmasked
   * when sent to the secret manager.
   */
  //static void HandleFetchResponse();

  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;
};

template <typename T>
T* TryDowncastBlob(Blob* aBlob);

template <>
BerytusEncryptedPacket* TryDowncastBlob(Blob* aBlob);

} // namespace mozilla::dom

#endif // DOM_BERYTUSENCRYPTEDPACKET_H_
