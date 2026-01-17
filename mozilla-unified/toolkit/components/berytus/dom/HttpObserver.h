/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 2; -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_BERYTUS_HTTPOBSERVER_H
#define MOZILLA_BERYTUS_HTTPOBSERVER_H

#include <cstdint>
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsHashtablesFwd.h"
#include "nsIInputStream.h"
#include "nsISupports.h"
#include "nsIUploadChannel2.h"
#include "nsIObserver.h"
#include "nsString.h"
#include "nsRefPtrHashtable.h"


class nsIHttpChannel;

namespace mozilla {
namespace berytus {

#define BERYTUS_HTTP_HEADER_CHANNEL_ID "X-Berytus-Channel-Id"_ns

class UnmaskPacket final : public nsISupports {
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS_FINAL
  NS_DECL_CYCLE_COLLECTION_CLASS(UnmaskPacket)

  UnmaskPacket(const nsACString& aBerytusChannelId,
               uint64_t aHttpChannelId,
               const nsACString& aContentType,
               uint64_t aContentLength,
               nsIInputStream* aBody);

  const nsCString& BerytusChannelId() const;
  uint64_t HttpChannelId() const;
  nsresult SetHttpChannelId(uint64_t aChannelId);
  const nsCString& ContentType() const;
  uint64_t ContentLength() const;
  nsIInputStream* Body() const;
private:
  ~UnmaskPacket();
  nsCString mBerytusChannelId;
  uint64_t mHttpChannelId;
  nsCString mContentType;
  uint64_t mContentLength;
  nsCOMPtr<nsIInputStream> mBody;
};

class HttpObserver final : public nsIObserver {
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(HttpObserver)

  NS_DECL_NSIOBSERVER

  static HttpObserver& Get();
  static already_AddRefed<HttpObserver> GetSingleton();

  void HoldUnmasked(RefPtr<UnmaskPacket>& aPacket);
private:
  HttpObserver();
  ~HttpObserver();

  nsRefPtrHashtable<nsUint64HashKey, UnmaskPacket> mPackets;

  void ReleaseUnmasked(uint64_t aChannelId);

  static void LogUploadBody(nsCOMPtr<nsIHttpChannel>& aChannel, nsCOMPtr<nsIUploadChannel2>& aUpload);

  /**
   * Here, we check with the child process if the
   * the channel id is associated with a BerytusEncryptedPacket.
   * To achieve this in a best manner, the content process should
   * set some sort of flag on the channel, marking it as such.
   * From there, we use the channel id
   */
  bool ShouldUnmask(uint64_t aChannelId, nsIUploadChannel2* aUploadChannel);
  void RetrieveUnmaskedPacket(uint64_t aChannelId);
  void ReplaceRequestBody(nsIUploadChannel2* aUploadChannel,
                          const nsACString& aContentType,
                          const int64_t& aContentLength,
                          nsIInputStream* aBody);
};
}
}
#endif