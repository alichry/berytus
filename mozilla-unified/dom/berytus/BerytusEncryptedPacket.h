/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSENCRYPTEDPACKET_H_
#define DOM_BERYTUSENCRYPTEDPACKET_H_

#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/Blob.h"
#include "nsIGlobalObject.h"
#include "nsStringFwd.h"
#include "nsTArrayForwardDeclare.h"
#include "mozilla/dom/BerytusChannel.h"

namespace mozilla::dom {

class OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
namespace fetch {
  using OwningBodyInit = OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
}

void TryUnmaskBerytusEncryptedPacketInFetchBody(
  const fetch::OwningBodyInit& aSrc,
  fetch::OwningBodyInit& aDest,
  const nsCString& aReqUrl,
  ErrorResult& aRv
);

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
  nsTArray<nsCString> mUrlAllowlist;
  bool mAttached;

  virtual already_AddRefed<Blob> UnmaskImpl(ErrorResult& aRv) = 0;

  static bool CreateMaskContent(BerytusEncryptedPacket::Content& aMask);
public:
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
