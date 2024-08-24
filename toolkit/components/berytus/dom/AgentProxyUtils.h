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