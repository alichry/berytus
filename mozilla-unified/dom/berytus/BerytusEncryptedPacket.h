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
#include "nsISupports.h"
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

  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;
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
  nsString mAttachedChannelId;

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
  class PacketObserver final : public nsIObserver {
  public:
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS_FINAL
    NS_DECL_CYCLE_COLLECTION_CLASS(PacketObserver)
    NS_DECL_NSIOBSERVER

    PacketObserver(RefPtr<BerytusEncryptedPacket>& aPacket);
  protected:
    ~PacketObserver();

    // TODO(berytus): Switch to WeakPtr
    RefPtr<BerytusEncryptedPacket> mPacket;
    nsRefPtrHashtable<nsUint64HashKey, berytus::UnmaskPacket> mDetectedChannels;
    nsRefPtrHashtable<nsPtrHashKey<InternalRequest>, berytus::UnmaskPacket> mDetectedRequests;
  };
};

template <typename T>
T* TryDowncastBlob(Blob* aBlob);

template <>
BerytusEncryptedPacket* TryDowncastBlob(Blob* aBlob);

} // namespace mozilla::dom

#endif // DOM_BERYTUSENCRYPTEDPACKET_H_
