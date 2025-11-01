/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/berytus/MaskManagerChild.h"
#include "mozilla/berytus/UnmaskerChild.h"

namespace mozilla {
namespace berytus {

MaskManagerChild::MaskManagerChild() {}

MaskManagerChild::~MaskManagerChild() {}

already_AddRefed<UnmaskerChild> MaskManagerChild::CreateUnmasker() {
  RefPtr<UnmaskerChild> unmasker = new UnmaskerChild();
  NS_ENSURE_TRUE(SendPUnmaskerConstructor(unmasker), nullptr);
  return unmasker.forget();
}

// already_AddRefed<PUnmaskerChild> MaskManagerChild::AllocPUnmaskerChild() {
// 	RefPtr<PUnmaskerChild> actor = new UnmaskerChild();
// 	return actor.forget();
// }
}
}