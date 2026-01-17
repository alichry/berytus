/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "mozilla/berytus/UnmaskerChild.h"

namespace mozilla {
namespace berytus {

namespace loggers::UnmaskerChild {
  static mozilla::LazyLogModule sLogger("berytus_mask_ipc");
}


UnmaskerChild::UnmaskerChild() {
  MOZ_LOG(loggers::UnmaskerChild::sLogger, LogLevel::Debug, ("UnmaskerChild()"));
}
UnmaskerChild::~UnmaskerChild() {
  MOZ_LOG(loggers::UnmaskerChild::sLogger, LogLevel::Debug, ("~UnmaskerChild()"));
}

ipc::IPCResult UnmaskerChild::Recv__delete__(const ipc::Shmem& aShmem) {
  return IPC_OK();
}

}
}