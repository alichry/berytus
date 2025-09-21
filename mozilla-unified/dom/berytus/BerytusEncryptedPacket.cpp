/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "BerytusEncryptedPacket.h"
#include "BerytusKeyAgreementParameters.h"
#include "mozilla/Span.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusX509Extension.h"
#include "mozilla/dom/MemoryBlobImpl.h"
#include "mozilla/dom/URLSearchParams.h"
#include "mozilla/dom/FormData.h"
#include "mozilla/dom/Fetch.h"
#include "nsNetUtil.h"
#include "mozilla/dom/BerytusChannel.h"
#include "nsString.h"

namespace mozilla::dom {

#define CONCEALED_HINT "[BerytusJWEPacket.CONCEALED]"

template <>
BerytusEncryptedPacket* TryDowncastBlob(Blob* aBlob) {
  MOZ_ASSERT(aBlob);
  if (!aBlob) {
    return nullptr;
  }
  if (!aBlob->HasBerytusEncryptedPacketInterface()) {
    return nullptr;
  }
  return reinterpret_cast<BerytusEncryptedPacket*>(aBlob);
}

bool BerytusEncryptedPacket::CreateMaskContent(BerytusEncryptedPacket::Content& aMask) {
  uint64_t len = sizeof(CONCEALED_HINT) - 1;
  aMask.mBuf.reset((uint8_t *) malloc(len));
  if (NS_WARN_IF(!aMask.mBuf)) {
    return false;
  }
  aMask.mLen = len;
  memcpy(aMask.mBuf.get(), CONCEALED_HINT, aMask.mLen);
  return true;
}

BerytusEncryptedPacket::BerytusEncryptedPacket(
    nsIGlobalObject* aGlobal,
    Content&& aExposedContent,
    const bool& aConceal) :
        Blob(aGlobal,
            MemoryBlobImpl::CreateWithLastModifiedNow(
            aExposedContent.mBuf.get(),
            aExposedContent.mLen,
            u"BerytusJWEPacket"_ns,
            // TODO(berytus): Content type should always be application/jose
            aConceal ? u"application/jose"_ns : u"text/plain"_ns,
            RTPCallerType::Normal).take()),
        mGlobal(aGlobal), /* mGlobal will live as long as it does under Blob */
        mExposedContent(Span<uint8_t>(aExposedContent.mBuf.release(), aExposedContent.mLen)),
        mConcealed(aConceal),
        mAttached(false) {
  MOZ_ASSERT(aExposedContent.mBuf.get() == nullptr);
}

BerytusEncryptedPacket::~BerytusEncryptedPacket() {}

bool BerytusEncryptedPacket::HasBerytusEncryptedPacketInterface() const {
  return true;
}

nsIGlobalObject* BerytusEncryptedPacket::GetParentObject() const { return mGlobal; }

Span<const uint8_t> BerytusEncryptedPacket::Exposed() const {
  return mExposedContent;
}

bool BerytusEncryptedPacket::Attached() const {
  return mAttached;
}

void BerytusEncryptedPacket::Attach(RefPtr<BerytusChannel>& aChannel,
                                    ErrorResult& aRv) {
  MOZ_ASSERT(aChannel);
  if (Attached()) {
    aRv.ThrowInvalidStateError("Packet already attached.");
    return;
  }
  RefPtr<BerytusKeyAgreementParameters> kap =
    aChannel->GetKeyAgreementParams();
  if (!kap) {
    aRv.ThrowInvalidStateError("Key agreement not prepared.");
    return;
  }
  MOZ_ASSERT(mUrlAllowlist.Length() == 0);
  for (const auto& url : kap->GetSession()->GetUnmaskAllowlist()) {
    nsresult rv;
    already_AddRefed<berytus::UrlSearchExpression> search = berytus::UrlSearchExpression::Create(NS_ConvertUTF16toUTF8(url), rv);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      mUrlAllowlist.Clear();
      aRv.Throw(rv);
      return;
    }
    if (NS_WARN_IF(!mUrlAllowlist.AppendElement(search.take(), fallible))) {
      aRv.ThrowTypeError("Out of memory");
      mUrlAllowlist.Clear();
      return;
    }
  }
  mAttached = true;
}

already_AddRefed<Blob> BerytusEncryptedPacket::Unmask(
    const nsCString& aReqUrl, ErrorResult& aRv) {
  bool discard;
  return Unmask(aReqUrl, discard, aRv);
}

already_AddRefed<Blob> BerytusEncryptedPacket::Unmask(
    const nsCString& aReqUrl,
    bool& aUnmasked, ErrorResult& aRv) {
  nsCOMPtr<nsIURI> uri;
  // NOTE(berytus): NS_NewURI does many more things than simply calling
  // nsIIOService->NewURI.
  if (NS_FAILED(NS_NewURI(getter_AddRefs(uri), aReqUrl, nullptr, nullptr))) {
    aRv.ThrowTypeError<MSG_INVALID_URL>(aReqUrl);
    return nullptr;
  }
  return Unmask(uri, aUnmasked, aRv);
}

void BerytusEncryptedPacket::SerializeExposedToString(
    nsACString& aValue, ErrorResult& aRv) const {
  aRv.ThrowNotSupportedError("Operation not implemented");
}

already_AddRefed<Blob> BerytusEncryptedPacket::Unmask(
    nsIURI* aReqUrl, ErrorResult& aRv) {
  bool discard;
  return Unmask(aReqUrl, discard, aRv);
}

already_AddRefed<Blob> BerytusEncryptedPacket::Unmask(
    nsIURI* aReqUrl, bool& aUnmasked, ErrorResult& aRv) {
  if (!mConcealed) {
    // not concealed, meaning the exposed blob already contains ciphertext
    // By default, JS-created JWE packets are not concealed and not attached.
    aUnmasked = false;
    return do_AddRef(static_cast<Blob*>(this));
  }
  if (!Attached()) {
    aRv.ThrowInvalidStateError("Bad packet");
    return nullptr;
  }
  if (mUrlAllowlist.Length() == 0) {
    // no allowlist defined, meaning we can unmask on any request url
    auto out = UnmaskImpl(aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return nullptr;
    }
    aUnmasked = true;
    return out;
  }
  nsresult rv;
  bool matches = false;
  rv = berytus::UrlSearchExpression::Matches(
      Span(mUrlAllowlist.Elements(), mUrlAllowlist.Length()),
      aReqUrl, matches);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  if (!matches) {
    // no match, meaning we just return the exposed dummy blob.
    aUnmasked = false;
    return do_AddRef(static_cast<Blob*>(this));
  }
  // a match! meaning we can unmask.
  auto out = UnmaskImpl(aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  aUnmasked = true;
  return out;
}

bool BerytusEncryptedPacket::TryUnmaskAnyPacketInFetchBody(
  const fetch::OwningBodyInit& aSrc,
  fetch::OwningBodyInit& aDest,
  const nsCString& aReqUrl,
  ErrorResult& aRv
) {
  if (aSrc.IsBlob()) {
    RefPtr<BerytusEncryptedPacket> packet = TryDowncastBlob<BerytusEncryptedPacket>(
      aSrc.GetAsBlob());
    if (!packet) {
      return false;
    }
    bool hasUnmasked;
    RefPtr<Blob> maybeUnmaskedBlob = packet->Unmask(aReqUrl,hasUnmasked, aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return false;
    }
    MOZ_ASSERT(maybeUnmaskedBlob);
    aDest.SetAsBlob() = maybeUnmaskedBlob;
    return hasUnmasked;
  }
  if (aSrc.IsFormData()) {
    const auto& fd = aSrc.GetAsFormData();
    bool unmaskNeeded = !fd->ForEach([](const nsString&,
                             const OwningBlobOrDirectoryOrUSVString& aValue) -> bool {
      if (!aValue.IsBlob()) {
        return true;
      }
      return !(bool(TryDowncastBlob<BerytusEncryptedPacket>(aValue.GetAsBlob())));
    });
    if (!unmaskNeeded) {
      aDest.SetAsFormData() = fd;
      return false;
    }
    const RefPtr<FormData> unmaskedFd = fd->Clone();
    bool anyHasUnmasked = false;
    for (auto& entry : unmaskedFd->mFormData) {
      if (!entry.value.IsBlob()) {
        continue;
      }
      RefPtr<BerytusEncryptedPacket> packet = TryDowncastBlob<BerytusEncryptedPacket>(
          entry.value.GetAsBlob());
      if (!packet) {
        continue;
      }
      bool hasUnmasked;
      RefPtr<Blob> maybeUnmaskedBlob = packet->Unmask(aReqUrl, hasUnmasked, aRv);
      if (NS_WARN_IF(aRv.Failed())) {
        return false;
      }
      MOZ_ASSERT(maybeUnmaskedBlob);
      entry.value.SetAsBlob() = maybeUnmaskedBlob;
      anyHasUnmasked = anyHasUnmasked || hasUnmasked;
    }
    aDest.SetAsFormData() = unmaskedFd;
    return anyHasUnmasked;
  }
  if (&aSrc == &aDest) {
    // no need to copy
    return false;
  }
  // TODO(berytus): Have a think about potentially not
  // doing this unnecessary copy. A flag can be returned
  // to guide the callee for which BodyInit to use when
  // sending the request.
  if (aSrc.IsArrayBuffer()) {
    if (NS_WARN_IF(!aDest.SetAsArrayBuffer().Init(aSrc.GetAsArrayBuffer().Obj()))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return false;
    }
    return false;
  }
  if (aSrc.IsArrayBufferView()) {
    if (NS_WARN_IF(!aDest.SetAsArrayBufferView().Init(aSrc.GetAsArrayBufferView().Obj()))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return false;
    }
    return false;
  }
  if (aSrc.IsUSVString()) {
    if (NS_WARN_IF(!aDest.SetAsUSVString().Assign(aSrc.GetAsUSVString(), fallible))) {
      aRv.ThrowTypeError("Out of memory");
      return false;
    }
    return false;
  }
  if (aSrc.IsURLSearchParams()) {
    aDest.SetAsURLSearchParams() = aSrc.GetAsURLSearchParams();
    return false;
  }
}

void BerytusEncryptedPacket::HandleFetchRequest(
    SafeRefPtr<InternalRequest>& aRequest,
    const fetch::OwningBodyInit& aReqBody,
    fetch::OwningBodyInit& aNewReqBody,
    ErrorResult& aRv) {
  nsCString reqUrl;
  aRequest->GetURL(reqUrl);
  /* Subprocedure (1): */
  bool unmasked = BerytusEncryptedPacket::TryUnmaskAnyPacketInFetchBody(
      aReqBody, aNewReqBody, reqUrl, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
  if (unmasked) {
    aRequest->SetSkipServiceWorker();
    // short-circuit: No need to go through subproc 2
    // as service worker interception has been already
    // skipped
    return;
  }
  /* Subprocedure (2): */
  // we need a way to get the list of signed urls
  // TODO(berytus): Create ChannelContainer
}

} // namespace mozilla::dom
