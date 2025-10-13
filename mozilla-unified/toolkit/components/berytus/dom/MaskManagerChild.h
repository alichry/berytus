/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef MOZILLA_BERYTUS_MASKMANAGERCHILD_H
#define MOZILLA_BERYTUS_MASKMANAGERCHILD_H


#include "mozilla/berytus/PMaskManagerChild.h"
#include "mozilla/ipc/ProtocolUtils.h" // ipc::IPCResult

namespace mozilla {
namespace berytus {

class MaskManagerChild final : public PMaskManagerChild {
public:
  NS_INLINE_DECL_REFCOUNTING(MaskManagerChild)
  MaskManagerChild();
protected:
  ~MaskManagerChild();

};

}
}

#endif