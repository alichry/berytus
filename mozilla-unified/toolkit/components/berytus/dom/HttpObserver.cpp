/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/berytus/HttpObserver.h"
#include "mozilla/Services.h"
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

namespace mozilla::berytus {

static mozilla::LazyLogModule sLogger("berytus_http_observer");

NS_IMPL_CYCLE_COLLECTION(UnmaskPacket, mBody)
NS_IMPL_CYCLE_COLLECTING_ADDREF(UnmaskPacket)
NS_IMPL_CYCLE_COLLECTING_RELEASE(UnmaskPacket)
NS_INTERFACE_MAP_BEGIN(UnmaskPacket)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

UnmaskPacket::UnmaskPacket(uint64_t aChannelId,
                           const nsACString& aContentType,
                           int64_t aContentLength,
                           nsIInputStream* aBody) : mChannelId(aChannelId),
                                                    mContentType(aContentType),
                                                    mContentLength(aContentLength),
                                                    mBody(aBody) {}

UnmaskPacket::~UnmaskPacket() {}

uint64_t UnmaskPacket::ChannelId() const { return mChannelId; }
const nsCString& UnmaskPacket::ContentType() const { return mContentType; }
int64_t UnmaskPacket::ContentLength() const { return mContentLength; }
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
          aPacket.get(), aPacket->ChannelId(), aPacket->ContentLength(), aPacket->ContentType().get()));
  mPackets.InsertOrUpdate(aPacket->ChannelId(), aPacket.get());
}

void HttpObserver::ReleaseUnmasked(uint64_t aChannelId) {
  MOZ_LOG(sLogger,
          LogLevel::Info,
          ("ReleaseUnmasked(id=%llu)", aChannelId));
  mPackets.Remove(aChannelId);
}

HttpObserver::HttpObserver() {
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
    MOZ_LOG(sLogger,
            LogLevel::Info,
            ("Detected(%llu, %s)",
            httpChannel->ChannelId(), spec.get()));
    if (!mPackets.Contains(httpChannel->ChannelId())) {
      MOZ_LOG(sLogger, LogLevel::Info,
          ("Could not find packet for Channel(id:%llu)",
          httpChannel->ChannelId()));
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
    MOZ_LOG(sLogger, LogLevel::Info, ("Calling Suspend() on Channel(id:%llu)", httpChannel->ChannelId()));
    rv = request->Suspend();
    NS_ENSURE_SUCCESS(rv, rv);
    nsCOMPtr<nsIInputStream> body = packet->Body();
    MOZ_ASSERT(body);
    nsCString method;
    rv = httpChannel->GetRequestMethod(method);
    NS_ENSURE_SUCCESS(rv, rv);
    MOZ_LOG(sLogger, LogLevel::Info, ("Replacing UploadStream of Channel(id:%llu) with method=%s, content-length=%lld, content-type=%s", httpChannel->ChannelId(), method.get(), packet->ContentLength(), packet->ContentType().get()));
    uploadChannel->ExplicitSetUploadStream(body,
                                            packet->ContentType(),
                                            packet->ContentLength(),
                                            method,
                                            false);
    ReleaseUnmasked(httpChannel->ChannelId());
    MOZ_LOG(sLogger, LogLevel::Info, ("Calling Resume() on Channel(id:%llu)", httpChannel->ChannelId()));
    rv = request->Resume();
    NS_ENSURE_SUCCESS(rv, rv);
    return NS_OK;
  }
  MOZ_ASSERT_UNREACHABLE("Unexpected topic");
  return NS_ERROR_FAILURE;
}



}