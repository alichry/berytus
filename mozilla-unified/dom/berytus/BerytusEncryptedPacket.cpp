/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "BerytusEncryptedPacket.h"
#include "BerytusKeyAgreementParameters.h"
#include "mozilla/RefPtr.h"
#include "mozilla/Span.h"
#include "mozilla/StaticPtr.h"
#include "mozilla/berytus/HttpObserver.h"
#include "mozilla/berytus/MaskManagerChild.h"
#include "mozilla/berytus/PMaskManagerChild.h"
#include "mozilla/berytus/UnmaskerChild.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusX509Extension.h"
#include "mozilla/dom/FetchDriver.h"
#include "mozilla/dom/MemoryBlobImpl.h"
#include "mozilla/dom/URLSearchParams.h"
#include "mozilla/dom/FormData.h"
#include "mozilla/dom/Fetch.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDebug.h"
#include "nsIHttpProtocolHandler.h"
#include "nsISupports.h"
#include "nsNetUtil.h"
#include "mozilla/dom/BerytusChannel.h"
#include "nsString.h"
#include "mozilla/dom/Request.h"
#include "nsIHttpChannel.h"
#include "mozilla/dom/BerytusChannelContainer.h"

namespace mozilla::dom {

#define CONCEALED_HINT "[BerytusJWEPacket.CONCEALED]"

static mozilla::LazyLogModule sLogger("berytus_fetch_observer");

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
        mConcealed(aConceal) {
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
  return mAttachedChannelId.Length() > 0;
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
  RefPtr<BerytusChannelContainer> container =
    BerytusChannelContainer::GetInstance(mGlobal->GetAsInnerWindow());
  if (NS_WARN_IF(!container)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  RefPtr<BerytusEncryptedPacket> self = this;
  RefPtr<PacketObserver> observer = new PacketObserver(self);
  container->HoldObserver(observer);
  mAttachedChannelId.Assign(aChannel->ID());
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
  return false;
}

NS_IMPL_CYCLE_COLLECTION(BerytusEncryptedPacket::PacketObserver, mPacket, mDetectedRequests, mDetectedChannels)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusEncryptedPacket::PacketObserver)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusEncryptedPacket::PacketObserver)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusEncryptedPacket::PacketObserver)
  NS_INTERFACE_MAP_ENTRY(nsIObserver)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIObserver)
NS_INTERFACE_MAP_END

BerytusEncryptedPacket::PacketObserver::PacketObserver(
    RefPtr<BerytusEncryptedPacket>& aPacket)
    : mPacket(aPacket) {
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  MOZ_ASSERT(obs);


  // TODO(beryuts): Add a Create() method as this is fallible.
  MOZ_ASSERT(NS_SUCCEEDED(obs->AddObserver(this, NS_FETCH_REQUEST_CONSTRUCTOR_TOPIC, false)));
  MOZ_ASSERT(NS_SUCCEEDED(obs->AddObserver(this, NS_HTTP_ON_OPENING_REQUEST_TOPIC, false)));
  MOZ_ASSERT(NS_SUCCEEDED(obs->AddObserver(this, NS_FETCH_DRIVER_HTTP_FETCH_TOPIC, false)));
}

BerytusEncryptedPacket::PacketObserver::~PacketObserver() {
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  if (obs) {
    obs->RemoveObserver(this,
                        NS_FETCH_REQUEST_CONSTRUCTOR_TOPIC);
    obs->RemoveObserver(this,
                        NS_HTTP_ON_OPENING_REQUEST_TOPIC);
    obs->RemoveObserver(this, NS_FETCH_DRIVER_HTTP_FETCH_TOPIC);
  }
}

NS_IMETHODIMP BerytusEncryptedPacket::PacketObserver::Observe(nsISupports* aSubject,
                                       const char* aTopic,
                                       const char16_t* aExtra) {
  NS_ENSURE_TRUE(NS_IsMainThread(), NS_ERROR_NOT_SAME_THREAD);
  nsresult rv;
  MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %p, %s)", this, aSubject, aTopic));
  if (strcmp(aTopic, NS_FETCH_REQUEST_CONSTRUCTOR_TOPIC) == 0) {
    // here, we try to detect if a masked packet is part of the request body
    // if so, we add the request to the list of detected requests
    RefPtr<Request::ConstructorNotification> notif =
        RefPtr(static_cast<Request::ConstructorNotification*>(aSubject));
    RequestInit const* init;
    rv = notif->GetInit(&init);
    NS_ENSURE_SUCCESS(rv, rv);
    RequestOrUTF8String const* input;
    rv = notif->GetInput(&input);
    NS_ENSURE_SUCCESS(rv, rv);
    Request const* request;
    rv = notif->GetResult(&request);
    NS_ENSURE_SUCCESS(rv, rv);
    SafeRefPtr<InternalRequest> internalRequest = request->GetInternalRequest();
    if (!init->mBody.WasPassed() || init->mBody.Value().IsNull()) {
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Body is unset/null. Checking input", this, aSubject, aTopic));
      if (input->IsUTF8String()) {
        MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Input is a URL.", this, aSubject, aTopic));
        return NS_OK;
      }
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Input is a Request. Checking existence", this, aSubject, aTopic));
      RefPtr<Request> inputReq = &input->GetAsRequest();
      SafeRefPtr<InternalRequest> inputInternalReq = inputReq->GetInternalRequest();
      if (!mDetectedRequests.Contains(inputInternalReq.unsafeGetRawPtr())) {
        MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Input is an UnDetectedRequest<%p>", this, aSubject, aTopic, inputInternalReq.unsafeGetRawPtr()));
        return NS_OK;
      }
      RefPtr<berytus::UnmaskPacket> packet =
          mDetectedRequests.Get(inputInternalReq.unsafeGetRawPtr());
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Input is a DetectedRequest<%p, %p>", this, aSubject, aTopic, inputInternalReq.unsafeGetRawPtr(), packet.get()));
      mDetectedRequests.InsertOrUpdate(
        internalRequest.unsafeGetRawPtr(),
        packet);
      MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %p, %s): Created DetectedRequestEntry<%p, %p>(length=%llu, content-type=%s)", this, aSubject, aTopic, internalRequest.unsafeGetRawPtr(), packet.get(), packet->ContentLength(), packet->ContentType().get()));
      //MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Skipping Service Worker Interception for DetectedRequestEntry<%p>", this, aSubject, aTopic, internalRequest.unsafeGetRawPtr()));
      //internalRequest->SetSkipServiceWorker();
      return NS_OK;
    }
    const fetch::OwningBodyInit& bodyInit =
      init->mBody.Value().Value();
    nsCString reqUrl;
    internalRequest->GetURL(reqUrl);
    fetch::OwningBodyInit newReqBody;
    ErrorResult erv;
    bool unmasked = BerytusEncryptedPacket::TryUnmaskAnyPacketInFetchBody(
        bodyInit, newReqBody, reqUrl, erv);
    if (NS_WARN_IF(erv.Failed())) {
      return erv.StealNSResult();
    }
    if (!unmasked) {
      // no masked packet found, nothing to do
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): No masked packet found", this, aSubject, aTopic));
      return NS_OK;
    }
    nsCOMPtr<nsIInputStream> stream;
    nsAutoCString contentTypeWithCharset;
    uint64_t contentLength = 0;
    rv = ExtractByteStreamFromBody(newReqBody, getter_AddRefs(stream),
                                   contentTypeWithCharset, contentLength);
    NS_ENSURE_SUCCESS(rv, rv);
    // TODO(berytus): Change mContentLength to uint64_t
    RefPtr<berytus::UnmaskPacket> packet = new berytus::UnmaskPacket(
      NS_ConvertUTF16toUTF8(mPacket->mAttachedChannelId),
      0,
      contentTypeWithCharset,
      contentLength,
      stream);

    mDetectedRequests.InsertOrUpdate(
      internalRequest.unsafeGetRawPtr(),
      packet);
    MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %p, %s): Created DetectedRequestEntry<%p, %p>(url=%s, length=%llu, content-type=%s)", this, aSubject, aTopic, internalRequest.unsafeGetRawPtr(), packet.get(), reqUrl.get(), contentLength, contentTypeWithCharset.get()));
    //MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): Skipping Service Worker Interception for DetectedRequestEntry<%p>", this, aSubject, aTopic, internalRequest.unsafeGetRawPtr()));
    //internalRequest->SetSkipServiceWorker();
    return NS_OK;
  }
  if (strcmp(aTopic, NS_FETCH_DRIVER_HTTP_FETCH_TOPIC) == 0) {
    nsCOMPtr<nsIStreamListener> streamListener = do_QueryInterface(aSubject);
    if (NS_WARN_IF(!streamListener)) {
      return NS_ERROR_FAILURE;
    }
    RefPtr<FetchDriver> fetchDriver = static_cast<FetchDriver*>(streamListener.get());
    SafeRefPtr<InternalRequest> request = fetchDriver->GetRequest();
    MOZ_ASSERT(request);
    nsCString url;
    request->GetURL(url);
    if (!mDetectedRequests.Contains(const_cast<InternalRequest*>(request.unsafeGetRawPtr()))) {
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): UndetectedRequest<%p>(url=%s)", this, aSubject, aTopic, request.unsafeGetRawPtr(), url.get()));
      {
        for (auto& entry : mDetectedRequests) {
          MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): [*] DetectedRequestEntry<%p, %p>", this, aSubject, aTopic, entry.GetKey(), entry.GetData().get()));
        }
      }
      return NS_OK;
    }
    RefPtr<berytus::UnmaskPacket> packet =
      mDetectedRequests.Get(const_cast<InternalRequest*>(request.unsafeGetRawPtr()));
    NS_ENSURE_TRUE(packet, NS_ERROR_FAILURE);
    const uint64_t* channelId = reinterpret_cast<const uint64_t*>(aExtra);
    NS_ENSURE_TRUE(channelId, NS_ERROR_INVALID_ARG);
    MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): DetectedRequest<%p, %p>(url=%s, channelId=%llu)", this, aSubject, aTopic, request.unsafeGetRawPtr(), packet.get(), url.get(), *channelId));
    if (NS_WARN_IF(mDetectedChannels.Contains(*channelId))) {
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): DuplicateDetection<%p, %llu>", this, aSubject, aTopic, request.unsafeGetRawPtr(), *channelId));
      {
        for (auto& entry : mDetectedChannels) {
          MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): [*] DetectedChannelEntry<%llu, %p>", this, aSubject, aTopic, entry.GetKey(), entry.GetData().get()));
        }
      }
      return NS_OK;
    }
    rv = packet->SetHttpChannelId(*channelId);
    NS_ENSURE_SUCCESS(rv, rv);
    mDetectedChannels.InsertOrUpdate(*channelId, packet);
    MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %p, %s): Created DetectedChannelEntry<%llu, %p>(length=%lld, content-type=%s)", this, aSubject, aTopic, *channelId, packet.get(), packet->ContentLength(), packet->ContentType().get()));
    return NS_OK;
  }
  if (strcmp(aTopic, NS_HTTP_ON_OPENING_REQUEST_TOPIC) == 0) {
    nsCOMPtr<nsIHttpChannel> channel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(channel, NS_ERROR_FAILURE);
    nsCOMPtr<nsIRequest> request = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(request, NS_ERROR_FAILURE);
    uint64_t channelId = channel->ChannelId();
    nsCOMPtr<nsIURI> uri;
    rv = channel->GetOriginalURI(getter_AddRefs(uri));
    NS_ENSURE_SUCCESS(rv, rv);
    nsCString uriSpec;
    rv = uri->GetSpec(uriSpec);
    NS_ENSURE_SUCCESS(rv, rv);
    if (!mDetectedChannels.Contains(channelId)) {
      MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): UndetectedChannel<%llu>(url=%s)", this, aSubject, aTopic, channelId, uriSpec.get()));
      {
        for (auto& entry : mDetectedChannels) {
          MOZ_LOG(sLogger, LogLevel::Debug, ("Observe(%p, %p, %s): [*] DetectedChannelEntry<%llu, %p>(length=%lld, content-type=%s)", this, aSubject, aTopic, entry.GetKey(), entry.GetData().get(), entry.GetData()->ContentLength(), entry.GetData()->ContentType().get()));
        }
      }
      return NS_OK;
    }
    RefPtr<berytus::UnmaskPacket> packet =
      mDetectedChannels.Get(channelId);
    MOZ_ASSERT(channelId == packet->HttpChannelId());
    MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %p, %s): Retrieved DetectedChannelEntry<%llu, %p>(length=%lld, content-type=%s)", this, aSubject, aTopic, channelId, packet.get(), packet->ContentLength(), packet->ContentType().get()));
    RefPtr<BerytusChannelContainer> container =
      BerytusChannelContainer::GetInstance(mPacket->mGlobal->GetAsInnerWindow());
    if (NS_WARN_IF(!container)) {
      return NS_ERROR_FAILURE;
    }
    RefPtr<mozilla::berytus::UnmaskerChild> unmasker = container->CreateUnmasker();
    NS_ENSURE_TRUE(unmasker, NS_ERROR_FAILURE);

    Maybe<mozilla::ipc::IPCStream> ipcStream;
    if (NS_WARN_IF(!mozilla::ipc::SerializeIPCStream(do_AddRef(packet->Body()), ipcStream, false))) {
      return NS_ERROR_FAILURE;
    }
    // nsLoadFlags loadFlags;
    // if (NS_SUCCEEDED(channel->GetLoadFlags(&loadFlags))) {
    //   channel->SetLoadFlags(loadFlags |
    //                         nsIChannel::LOAD_BYPASS_SERVICE_WORKER);
    // }
    rv = request->Suspend();
    NS_ENSURE_SUCCESS(rv, rv);
    auto promise =
      unmasker->SendUnmaskInChannel(packet->BerytusChannelId(),
                                    packet->HttpChannelId(),
                                    ipcStream.value(),
                                    packet->ContentLength(),
                                    packet->ContentType());
    promise->Then(
      GetMainThreadSerialEventTarget(), __func__,
      [request]() {
        nsresult rv = request->Resume();
        NS_ENSURE_SUCCESS(rv, );
      },
      [request](mozilla::ipc::ResponseRejectReason&& aReason) {
        NS_WARNING("Unmasking failed");
        nsresult rv = request->Resume();
        NS_ENSURE_SUCCESS(rv, );
      }
    );
    return NS_OK;
  }
  return NS_ERROR_INVALID_ARG;
}

} // namespace mozilla::dom}