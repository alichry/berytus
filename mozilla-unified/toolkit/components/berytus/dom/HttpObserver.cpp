/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/berytus/HttpObserver.h"
#include "ErrorList.h"
#include "mozilla/LoggingCore.h"
#include "mozilla/Services.h"
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDebug.h"
#include "nsICloneableInputStream.h"
#include "nsIHttpProtocolHandler.h"
#include "nsIObserver.h"
#include "nsIObserverService.h"
#include "nsIHttpChannel.h"
#include "nsIRequest.h"
#include "mozilla/ClearOnShutdown.h"
#include "nsISupports.h"
#include "nsIUploadChannel2.h"
#include "nsNetUtil.h"
#include "nsStringFwd.h"
#include "nsXULAppAPI.h"
#include <limits>
#include "nsStreamUtils.h"


namespace mozilla::berytus {

/**
 * Adapted from Fetch.cpp's FetchBody<Derived>::GetMimeType
 */
nsresult GetMimeType(nsCOMPtr<nsIHttpChannel>& aHttpChannel,
                     nsACString& aMimeType,
                     nsACString& aMixedCaseMimeType) {
  nsCString contentTypeValues;
  nsresult rv = aHttpChannel->GetResponseHeader(
    BERYTUS_HTTP_HEADER_CHANNEL_ID,
    contentTypeValues);

  NS_ENSURE_SUCCESS(rv, NS_ERROR_NOT_AVAILABLE);

  // HTTP ABNF states Content-Type may have only one value.
  // This is from the "parse a header value" of the fetch spec.
  if (!contentTypeValues.IsVoid() && contentTypeValues.Find(",") == -1) {
    // Convert from a bytestring to a UTF8 CString.
    CopyLatin1toUTF8(contentTypeValues, aMimeType);
    aMixedCaseMimeType = aMimeType;
    ToLowerCase(aMimeType);
    return NS_OK;
  }
  return NS_ERROR_DOM_INVALID_HEADER_VALUE;
}

/**
 * Copied from BodyUtil.cpp
 */
bool IsFormDataMimeType(const nsACString& aMimeType) {
  constexpr auto formDataMimeType = "multipart/form-data"_ns;
  // Allow semicolon separated boundary/encoding suffix like
  // multipart/form-data; boundary= but disallow multipart/form-datafoobar.
  bool isValidFormDataMimeType = StringBeginsWith(aMimeType, formDataMimeType);

  if (isValidFormDataMimeType &&
      aMimeType.Length() > formDataMimeType.Length()) {
    isValidFormDataMimeType = aMimeType[formDataMimeType.Length()] == ';';
  }
  return isValidFormDataMimeType;
}

/**
 * Copied from BodyUtil.cpp
 */
bool IsUrlDataMimeType(const nsACString& aMimeType) {
constexpr auto urlDataMimeType = "application/x-www-form-urlencoded"_ns;
  bool isValidUrlEncodedMimeType = StringBeginsWith(aMimeType, urlDataMimeType);

  if (isValidUrlEncodedMimeType &&
      aMimeType.Length() > urlDataMimeType.Length()) {
    isValidUrlEncodedMimeType = aMimeType[urlDataMimeType.Length()] == ';';
  }
  return isValidUrlEncodedMimeType;
}

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

  obs->AddObserver(this, NS_HTTP_ON_OPENING_REQUEST_TOPIC, false);
  obs->AddObserver(this, NS_HTTP_ON_MODIFY_REQUEST_TOPIC, false);
  obs->AddObserver(this, NS_HTTP_ON_STOP_REQUEST_TOPIC, false);
  obs->AddObserver(this, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC, false);
}

HttpObserver::~HttpObserver() {
  nsCOMPtr<nsIObserverService> obs = mozilla::services::GetObserverService();
  if (obs) {
    obs->RemoveObserver(this, NS_HTTP_ON_OPENING_REQUEST_TOPIC);
    obs->RemoveObserver(this, NS_HTTP_ON_MODIFY_REQUEST_TOPIC);
    obs->RemoveObserver(this, NS_HTTP_ON_STOP_REQUEST_TOPIC);
    obs->RemoveObserver(this, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC);
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

// TODO: To implement masking of packets coming from
// the web app, we must apply a fetch() dom hook that
// intercepts the about-to-be-resolved Response object
// We would want to potentially return a ddifferent
// Response object.

// Response Exposed
// Response Wrapped (Masked)
// Exposed.formData() -> Wrapped.formData()
//  then, foreach element in formdata
//   if element is a blob, and its mime type
//   is a jwe packet, we wrap the blob in
//   a BerytusEncryptedPacket, else add
//   the element as is.
// Exposed.blob() -> Wrapped.blob()
//   if content-type is jwe packet,
//      we wrap Wrapped.blob() in a BerytusEncryptedPacket
//   else if content-type is application/octet-stream
//      call Exposed.arrayBuffer()
//      and wrap the return buffer as a blob.
//   else return Wrapped.blob() as is
// Exposed.arrayBuffer() ->
//    if content-type is a jwe packet:
//      return _CONCEALED PACKET_
//      i.e. return Exposed.blob()'s bytes in an ArrayBuffer
//    elif content-type multi part
//      parse as form data "wrapped"
//      create a new form data "exposed"
//      for each element in "wrapped"
//        if element is a packet
//          exposed.set(element.name, _CONCEALED_PACKEt_)
//          continue
//        exposed.set(element.name, element.value)
//      return serialization of 'exposed'
//    else
//      return Wrapped.arrayBuffer()
// Exposed.text() -> TextDecode(Exposed.arrrayBuffer(), "utf-8")
// Exposed.json() -> JSON.parse(Exposed.text())


NS_IMETHODIMP HttpObserver::Observe(nsISupports* aSubject,
                                    const char* aTopic,
                                    const char16_t*) {
  MOZ_LOG(sLogger, LogLevel::Info, ("Observe(%p, %s)", aSubject, aTopic));
  if (strcmp(aTopic, "app-startup") == 0) {
    return NS_OK;
  }
  if (strcmp(aTopic, NS_HTTP_ON_OPENING_REQUEST_TOPIC) == 0) {
    /**
     * The X-Berytus-Channel-Id header is a reserved HTTP header used
     * in the Berytus framework. It must not be set by client code,
     * just like the Origin header must not be set by client code.
     * The web application must not trust any JWE packet in request
     * bodies where there is no corresponding X-Berytus-Channel-Id header.
     * Therefore, if client code sets the header, we abort the request.
     *
     * Moreover, the current Berytus implementation relies on the
     * X-Berytus-Channel-Id as a signal to skip web extension interception
     * of request bodies.
     */
    nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(httpChannel, NS_ERROR_FAILURE);
    nsAutoCString existingBerytusHeader;
    nsresult rv = httpChannel->GetRequestHeader(BERYTUS_HTTP_HEADER_CHANNEL_ID,
                                       existingBerytusHeader);
    if (NS_FAILED(rv)) {
      return NS_OK;
    }
    MOZ_LOG(sLogger, LogLevel::Warning,
      ("Channel(id:%llu) already has the Berytus Header; aborting request.",
      httpChannel->ChannelId()));
    rv = httpChannel->Cancel(NS_ERROR_DOM_INVALID_HEADER_NAME);
    NS_ENSURE_SUCCESS(rv, rv);
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
        ("Channel(id:%llu) exsting upload stream has headers; refusing to replace upload stream.",
        httpChannel->ChannelId()));
      return NS_OK;
    }
    nsCOMPtr<nsIInputStream> body = packet->Body();
    {
      nsCOMPtr<nsICloneableInputStream> stream = do_QueryInterface(packet->Body());
      if (NS_WARN_IF(!stream)) {
        MOZ_LOG(sLogger, LogLevel::Error,
          ("Channel(id:%llu) packet body is not cloneable; refusing to replace upload stream.",
          httpChannel->ChannelId()));
        return NS_ERROR_FAILURE;
      }
    }
    MOZ_ASSERT(body);
    nsCString method;
    rv = httpChannel->GetRequestMethod(method);
    NS_ENSURE_SUCCESS(rv, rv);
    if (NS_WARN_IF(packet->ContentLength() > std::numeric_limits<int64_t>::max())) {
      MOZ_LOG(sLogger, LogLevel::Info,
              ("Packet content length %llu exceeds int64_t max; refusing to replace upload stream.",
               packet->ContentLength()));
      return NS_ERROR_FAILURE;
    }
    nsAutoCString contentLengthStr;
    contentLengthStr.AppendInt(packet->ContentLength());
    MOZ_LOG(sLogger, LogLevel::Info,
        ("Adding Content-Length: %s to Channel(id=%llu)",
          contentLengthStr.get(), httpChannel->ChannelId()));
    rv = httpChannel->SetRequestHeader("Content-Length"_ns,
                                       contentLengthStr,
                                       false);
    NS_ENSURE_SUCCESS(rv, rv);
    MOZ_LOG(sLogger,
            LogLevel::Info,
            ("Replacing UploadStream of Channel(id:%llu) with method=%s, content-length=%lld, content-type=%s",
              httpChannel->ChannelId(), method.get(),
              packet->ContentLength(), packet->ContentType().get()));
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
    nsLoadFlags loadFlags;
    rv = httpChannel->GetLoadFlags(&loadFlags);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = httpChannel->SetLoadFlags(loadFlags |
                                   nsIChannel::LOAD_BYPASS_SERVICE_WORKER);
    NS_ENSURE_SUCCESS(rv, rv);
    return NS_OK;
  }
  if (strcmp(aTopic, NS_HTTP_ON_STOP_REQUEST_TOPIC) == 0) {
    MOZ_LOG(sLogger, LogLevel::Info, ("In NS_HTTP_ON_STOP_REQUEST_TOPIC"));
    nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(httpChannel, NS_ERROR_FAILURE);
    uint64_t channelId = httpChannel->ChannelId();
    ReleaseUnmasked(channelId);
    return NS_OK;
  }
  if (strcmp(aTopic, NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC) == 0) {
    MOZ_LOG(sLogger, LogLevel::Info, ("In NS_HTTP_ON_EXAMINE_RESPONSE_TOPIC"));
    // here, we need to extract the packets and replace
    // the stream with a stream that has the packets masked.
    // first, get content-type. If it's
    // (1). Multipart
    // (2). JWE
    // we process it, otherwise we do nothing.
    nsCOMPtr<nsIHttpChannel> httpChannel = do_QueryInterface(aSubject);
    NS_ENSURE_TRUE(httpChannel, NS_ERROR_INVALID_ARG);

    nsAutoCString existingBerytusHeader;
    nsresult rv = httpChannel->GetResponseHeader(
      BERYTUS_HTTP_HEADER_CHANNEL_ID,
      existingBerytusHeader);
    if (NS_FAILED(rv)) {
      // no berytus header, do nothing.
      return NS_OK;
    }
    // TODO(berytus): We need to check the channel id if it's valid.
    nsCString mimeType;
    nsCString mixedCaseMimeType;
    rv = GetMimeType(httpChannel, mimeType, mixedCaseMimeType);
    if (NS_FAILED(rv)) {
      MOZ_LOG(sLogger,
              LogLevel::Warning,
              ("Content-Type Response Header is invald or not found"));
      return NS_OK;
    }
    if (IsFormDataMimeType(mimeType) || IsUrlDataMimeType(mimeType)) {
      // 1. extract form data
      // 2. extract packets
      // 3. set fields with packets to CONCEALED
      // 4. extract bytes from formdata
      // Update response body, ensure content-length is updated
      // ensure boundary is not modified.
    }


  }
  MOZ_ASSERT_UNREACHABLE("Unexpected topic");
  return NS_ERROR_FAILURE;
}




}