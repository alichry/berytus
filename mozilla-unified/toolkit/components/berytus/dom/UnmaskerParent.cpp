/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "mozilla/berytus/UnmaskerParent.h"
#include "mozilla/berytus/HttpObserver.h"
#include "mozilla/ipc/IPCStreamUtils.h"
#include "mozilla/ipc/IPCCore.h" // void_t

namespace mozilla {
namespace berytus {

namespace loggers::UnmaskerParent {
  static mozilla::LazyLogModule sLogger("berytus_mask_ipc");
}

UnmaskerParent::UnmaskerParent() {
  MOZ_LOG(loggers::UnmaskerParent::sLogger, LogLevel::Debug, ("UnmaskerParent()"));
}
UnmaskerParent::~UnmaskerParent() {
  MOZ_LOG(loggers::UnmaskerParent::sLogger, LogLevel::Debug, ("~UnmaskerParent()"));
}

ipc::IPCResult UnmaskerParent::RecvUnmaskInChannel(
    const nsACString& aBerytusChannelId, const uint64_t& aHttpChannelId,
    const IPCStream& aUnmaskedStream, const uint64_t& aLength,
    const nsACString& aContentType, UnmaskInChannelResolver&& aResolver) {
  MOZ_LOG(loggers::UnmaskerParent::sLogger,
          LogLevel::Info,
          ("RecvUnmaskInChannel(berytusChannelId=%.*s, httpChannelId=%llu, length=%llu, content-type=%.*s)", (int) aBerytusChannelId.Length(), aBerytusChannelId.BeginReading(), aHttpChannelId, aLength, (int) aContentType.Length(), aContentType.BeginReading()));
  RefPtr<HttpObserver> httpObs = HttpObserver::GetSingleton();
  nsCOMPtr<nsIInputStream> body = DeserializeIPCStream(aUnmaskedStream.stream());
  if (NS_WARN_IF(!body)) {
    return IPC_FAIL(this, "Failed to deserialize IPCStream");
  }
  RefPtr<UnmaskPacket> packet =
      new UnmaskPacket(aBerytusChannelId,
                       aHttpChannelId,
                       aContentType,
                       aLength,
                       body);
  httpObs->HoldUnmasked(packet);
  aResolver(void_t{});
  return IPC_OK();
}

}
}