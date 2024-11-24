/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BERYTUS_AGENTPROXYUTILS_H_
#define BERYTUS_AGENTPROXYUTILS_H_

#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusWebAppActor.h"

namespace mozilla::berytus {

nsresult Utils_PreliminaryRequestContext(nsIGlobalObject* aGlobal,
                                         PreliminaryRequestContext& aOut);
void
Utils_nsURIToUriParams(nsIURI* aSrcURI, UriParams& aOut);

Variant<berytus::CryptoActor, berytus::OriginActor>*
Utils_WebAppActorToVariant(const RefPtr<mozilla::dom::BerytusWebAppActor> aActor);



}

#endif