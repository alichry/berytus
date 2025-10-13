/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "mozilla/berytus/UnmaskerParent.h"
#include "mozilla/berytus/HttpObserver.h"
#include "mozilla/ipc/IPCStreamUtils.h"

namespace mozilla {
namespace berytus {

UnmaskerParent::UnmaskerParent() {}
UnmaskerParent::~UnmaskerParent() {}

ipc::IPCResult UnmaskerParent::RecvUnmaskInChannel(
    const uint64_t& aChannelId, const IPCStream& aUnmaskedStream,
    const uint64_t& aLength, const nsACString& aContentType) {
  RefPtr<HttpObserver> httpObs = HttpObserver::GetSingleton();
  nsCOMPtr<nsIInputStream> body = DeserializeIPCStream(aUnmaskedStream.stream());
  if (NS_WARN_IF(!body)) {
    return IPC_FAIL(this, "Failed to deserialize IPCStream");
  }
  RefPtr<HttpObserver::UnmaskPacket> packet =
      new HttpObserver::UnmaskPacket(aChannelId,
                                     aContentType,
                                     // TODO(berytus): Change to uint64_t if we're sure we can infer len at some point
                                     int64_t(aLength),
                                     body);
  httpObs->HoldUnmasked(packet);
  return IPC_OK();
}

}
}