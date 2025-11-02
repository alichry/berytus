/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/berytus/HttpObserver.h"
#include "mozilla/LoggingCore.h"
#include "mozilla/Services.h"
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDebug.h"
#include "nsIHttpProtocolHandler.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsIHttpChannel.h"
#include "nsIRequest.h"
#include "mozilla/ClearOnShutdown.h"
#include "nsISupports.h"
#include "nsIUploadChannel2.h"
#include "nsNetUtil.h"
#include "nsXULAppAPI.h"
#include <limits>


namespace mozilla::berytus {

static mozilla::LazyLogModule sLogger("berytus_http_observer");

NS_IMPL_CYCLE_COLLECTION(UnmaskPacket, mBody)
NS_IMPL_CYCLE_COLLECTING_ADDREF(UnmaskPacket)
NS_IMPL_CYCLE_COLLECTING_RELEASE(UnmaskPacket)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(UnmaskPacket)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

UnmaskPacket::UnmaskPacket(const nsACString& aBerytusChannelId,
                           uint64_t aHttpChannelId,
                           const nsACString& aContentType,
                           uint64_t aContentLength,
                           nsIInputStream* aBody) : mBerytusChannelId(aBerytusChannelId),
                                                    mHttpChannelId(aHttpChannelId),
                                                    mContentType(aContentType),
                                                    mContentLength(aContentLength),
                                                    mBody(aBody) {}

UnmaskPacket::~UnmaskPacket() {}

const nsCString& UnmaskPacket::BerytusChannelId() const { return mBerytusChannelId; }
uint64_t UnmaskPacket::HttpChannelId() const { return mHttpChannelId; }
nsresult UnmaskPacket::SetHttpChannelId(uint64_t aChannelId) {
  if (mHttpChannelId != 0) {
    return NS_ERROR_FAILURE;
  }
  mHttpChannelId = aChannelId;
  return NS_OK;
}
const nsCString& UnmaskPacket::ContentType() const { return mContentType; }
uint64_t UnmaskPacket::ContentLength() const { return mContentLength; }
nsIInputStream* UnmaskPacket::Body() const { return mBody; }

NS_IMPL_CYCLE_COLLECTION(HttpObserver, mPackets)
NS_IMPL_CYCLE_COLLECTING_ADDREF(HttpObserver)
NS_IMPL_CYCLE_COLLECTING_RELEASE(HttpObserver)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(HttpObserver)
  NS_INTERFACE_MAP_ENTRY(nsIObserver)
  NS_INTERFACE_MAP_ENTRY_AMBIGUOUS(nsISupports, nsIObserver)
NS_INTERFACE_MAP_END

HttpObserver& HttpObserver::Get() {
  static RefPtr<HttpObserver> sInstance;

  if (MOZ_UNLIKELY(!sInstance)) {
    sInstance = new HttpObserver();
    ClearOnShutdown(&sInstance);
  }
  return *sInstance;
}

already_AddRefed<HttpObserver> HttpObserver::GetSingleton() {
  return do_AddRef(&Get());
}

void HttpObserver::HoldUnmasked(RefPtr<UnmaskPacket>& aPacket) {
  MOZ_ASSERT(aPacket);
  MOZ_LOG(sLogger,
          LogLevel::Info,
          ("HoldUnmasked(%p, id=%llu, length=%lld, content-type=%s) ",
          aPacket.get(), aPacket->HttpChannelId(), aPacket->ContentLength(), aPacket->ContentType().get()));
  mPackets.InsertOrUpdate(aPacket->HttpChannelId(), aPacket.get());
}

void HttpObserver::ReleaseUnmasked(uint64_t aChannelId) {
  MOZ_LOG(sLogger,
          LogLevel::Info,
          ("ReleaseUnmasked(id=%llu)", aChannelId));
  mPackets.Remove(aChannelId);
}

HttpObserver::HttpObserver() {
  MOZ_ASSERT(XRE_IsParentProcess());
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  MOZ_ASSERT(obs);

  obs->AddObserver(this, NS_HTTP_ON_MODIFY_REQUEST_TOPIC, false);
}

HttpObserver::~HttpObserver() {
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  if (obs) {
    obs->RemoveObserver(this, NS_HTTP_ON_MODIFY_REQUEST_TOPIC);
  }
}

void HttpObserver::LogUploadBody(nsCOMPtr<nsIHttpChannel>& aChannel, nsCOMPtr<nsIUploadChannel2>& aUpload) {
  if (!MOZ_LOG_TEST(sLogger, LogLevel::Debug)) {
    return;
  }
  int64_t length;
  nsCOMPtr<nsIInputStream> clonedBody;
  NS_ENSURE_SUCCESS(aUpload->CloneUploadStream(&length, getter_AddRefs(clonedBody)), );
  if (!clonedBody) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("PrintUploadBody(channelId=%llu): length=?, body=?", aChannel->ChannelId()));
    return;
  }
  nsCString bodyStr;
  NS_ENSURE_SUCCESS(NS_ReadInputStreamToString(clonedBody, bodyStr, std::min(length, 4096LL)), );
  MOZ_LOG(sLogger, LogLevel::Debug, ("PrintUploadBody(channelId=%llu): length=%lld, body=%s", aChannel->ChannelId(), length, bodyStr.get()));
}

NS_IMETHODIMP HttpObserver::Observe(nsISupports* aSubject,
                                    const char* aTopic,
                                    const char16_t*) {
  MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %s)", aSubject, aTopic));
  if (strcmp(aTopic, "app-startup") == 0) {
    return NS_OK;
  }
  if (strcmp(aTopic, NS_HTTP_ON_MODIFY_REQUEST_TOPIC) == 0) {
    MOZ_LOG(sLogger, LogLevel::Info, ("In NS_HTTP_ON_MODIFY_REQUEST_TOPIC"));
    nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(httpChannel, NS_ERROR_FAILURE);
    nsCOMPtr<nsIRequest> request = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(request, NS_ERROR_FAILURE);
    nsCOMPtr<nsIUploadChannel2> uploadChannel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(uploadChannel, NS_ERROR_FAILURE);
    nsIURI* uri;
    nsresult rv = httpChannel->GetURI(&uri);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      return rv;
    }
    nsCString spec;
    rv = uri->GetSpec(spec);
    NS_ENSURE_SUCCESS(rv, rv);
    LogUploadBody(httpChannel, uploadChannel);
    MOZ_LOG(sLogger,
            LogLevel::Info,
            ("Detected(id=%llu, url=%s)",
            httpChannel->ChannelId(), spec.get()));
    if (!mPackets.Contains(httpChannel->ChannelId())) {
      MOZ_LOG(sLogger, LogLevel::Info,
          ("Could not find packet for Channel(id=%llu, url=%s)",
          httpChannel->ChannelId(), spec.get()));
      return NS_OK;
    }
    RefPtr<UnmaskPacket> packet = mPackets.Get(httpChannel->ChannelId());
    MOZ_ASSERT(packet);
    MOZ_LOG(sLogger, LogLevel::Info,
            ("Found packet for Channel(id:%llu)",
            httpChannel->ChannelId()));
    bool hasHeaders;
    rv = uploadChannel->GetUploadStreamHasHeaders(&hasHeaders);
    NS_ENSURE_SUCCESS(rv, rv);
    if (NS_WARN_IF(hasHeaders)) {
      MOZ_LOG(sLogger, LogLevel::Info,
        ("Channel(id:%llu) has headers; refusing to replace upload stream.",
        httpChannel->ChannelId()));
      return NS_OK;
    }
    nsCOMPtr<nsIInputStream> body = packet->Body();
    MOZ_ASSERT(body);
    nsCString method;
    rv = httpChannel->GetRequestMethod(method);
    NS_ENSURE_SUCCESS(rv, rv);
    nsCString bodyStr;
    bodyStr.Assign("?");
    if (MOZ_LOG_TEST(sLogger, LogLevel::Debug)) {
      rv = NS_ReadInputStreamToString(body, bodyStr, (int64_t) std::min(packet->ContentLength(), 4096ULL));
      NS_ENSURE_SUCCESS(rv, rv);
    }
    if (NS_WARN_IF(packet->ContentLength() > std::numeric_limits<int64_t>::max())) {
      MOZ_LOG(sLogger, LogLevel::Info,
              ("Packet content length %llu exceeds int64_t max; refusing to replace upload stream.",
               packet->ContentLength()));
      return NS_ERROR_FAILURE;
    }
    MOZ_LOG(sLogger,
            LogLevel::Info,
            ("Replacing UploadStream of Channel(id:%llu) with method=%s, content-length=%lld, content-type=%s, body=%s",
              httpChannel->ChannelId(), method.get(),
              packet->ContentLength(), packet->ContentType().get(),
              bodyStr.get()));
    rv = uploadChannel->ExplicitSetUploadStream(body,
                                            packet->ContentType(),
                                            (int64_t) packet->ContentLength(),
                                            method,
                                            false);
    NS_ENSURE_SUCCESS(rv, rv);
    LogUploadBody(httpChannel, uploadChannel);
    MOZ_LOG(sLogger, LogLevel::Info,
            ("Adding Berytus Header to Channel(id=%llu)",
             httpChannel->ChannelId()));
    rv = httpChannel->SetRequestHeader(BERYTUS_HTTP_HEADER_CHANNEL_ID,
                                       packet->BerytusChannelId(),
                                       false);
    NS_ENSURE_SUCCESS(rv, rv);
    ReleaseUnmasked(httpChannel->ChannelId());
    return NS_OK;
  }
  MOZ_ASSERT_UNREACHABLE("Unexpected topic");
  return NS_ERROR_FAILURE;
}



}