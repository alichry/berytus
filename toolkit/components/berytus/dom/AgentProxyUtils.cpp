#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusCryptoWebAppActor.h"
#include "mozilla/dom/BerytusAnonymousWebAppActor.h"

namespace mozilla::berytus {

nsresult Utils_PreliminaryRequestContext(nsIGlobalObject* aGlobal,
                                         PreliminaryRequestContext& aOut) {
  nsPIDOMWindowInner* inner = aGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    return NS_ERROR_FAILURE;
  }
  if (NS_WARN_IF(inner->WindowID() > std::numeric_limits<double>::max())) {
    return NS_ERROR_FAILURE;
  }
  aOut.mDocument.mId = static_cast<double>(inner->WindowID());
  return NS_OK;
}

void Utils_nsURIToUriParams(nsIURI* aSrcURI, UriParams& aOut) {
  nsCString utf8Str;

  aSrcURI->GetHost(utf8Str);
  CopyUTF8toUTF16(utf8Str, aOut.mHostname);
  aSrcURI->GetScheme(utf8Str);
  CopyUTF8toUTF16(utf8Str, aOut.mScheme);
  aSrcURI->GetFilePath(utf8Str);
  CopyUTF8toUTF16(utf8Str, aOut.mPath);
  int32_t port = 0;
  aSrcURI->GetPort(&port);
  aOut.mPort = static_cast<double>(port);
  aSrcURI->GetSpec(utf8Str);
  CopyUTF8toUTF16(utf8Str, aOut.mUri);

}

Variant<berytus::CryptoActor, berytus::OriginActor>*
Utils_WebAppActorToVariant(const RefPtr<dom::BerytusWebAppActor> aActor) {
  if (aActor->Type() == dom::BerytusWebAppActorType::CryptoActor) {
    CryptoActor sActor = CryptoActor();
    static_cast<dom::BerytusCryptoWebAppActor*>(aActor.get())
      ->GetEd25519Key(sActor.mEd25519Key);
    return new Variant<CryptoActor, berytus::OriginActor>(std::move(sActor));
  }
  OriginActor sActor = OriginActor();
  auto* anonActor = static_cast<dom::BerytusAnonymousWebAppActor*>(aActor.get());
  nsIURI* originalURI = anonActor->GetOriginalURI();
  nsIURI* currentURI = anonActor->GetOriginalURI();
  Utils_nsURIToUriParams(originalURI, sActor.mOriginalUri);
  Utils_nsURIToUriParams(currentURI, sActor.mCurrentUri);
  return new Variant<berytus::CryptoActor, berytus::OriginActor>(std::move(sActor));
}

};