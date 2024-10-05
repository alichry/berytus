/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusKeyAgreementParameters.h"
#include "mozilla/dom/BerytusKeyAgreementParametersBinding.h"
#include "mozilla/Base64.h"
#include "nsString.h"
#include "nsStringFwd.h"
#include "nsTextFormatter.h"
#include "mozilla/Assertions.h"
#include "js/ArrayBuffer.h"  // JS::NewArrayBufferWithContents

namespace mozilla::dom {


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_0(BerytusKeyAgreementParameters)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusKeyAgreementParameters)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusKeyAgreementParameters)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusKeyAgreementParameters)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusKeyAgreementParameters::BerytusKeyAgreementParameters(
  nsIGlobalObject* aGlobal,
  const nsAString& aSessionId,
  const nsAString& aScmX25519Key,
  const nsAString& aWebAppX25519Key,
  const nsAString& aHkdfHash,
  const nsTArray<uint8_t>& aHkdfSalt,
  const nsTArray<uint8_t>& aHkdfInfo,
  const uint16_t& aAesKeyLength
) : mGlobal(aGlobal),
  mSessionId(aSessionId),
  mScmX25519Key(aScmX25519Key),
  mWebAppX25519Key(aWebAppX25519Key),
  mHkdfHash(aHkdfHash),
  mHkdfSalt(nsTArray<uint8_t>(aHkdfSalt.Length())),
  mHkdfInfo(nsTArray<uint8_t>(aHkdfInfo.Length())),
  mAesKeyLength(aAesKeyLength)
{
  MOZ_ASSERT(false);
  // AuthRealmUtils::CopyByteArray(aHkdfInfo, mHkdfInfo);
  // AuthRealmUtils::CopyByteArray(aHkdfSalt, mHkdfSalt);
    // Add |MOZ_COUNT_CTOR(BerytusKeyAgreementParameters);| for a non-refcounted object.
}

BerytusKeyAgreementParameters::~BerytusKeyAgreementParameters()
{
    // Add |MOZ_COUNT_DTOR(BerytusKeyAgreementParameters);| for a non-refcounted object.
}

nsIGlobalObject* BerytusKeyAgreementParameters::GetParentObject() const { return mGlobal; }

JSObject*
BerytusKeyAgreementParameters::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyAgreementParameters_Binding::Wrap(aCx, this, aGivenProto);
}

void BerytusKeyAgreementParameters::GetSessionId(nsString& aRetVal) const
{
  aRetVal.Assign(mSessionId);
}

void BerytusKeyAgreementParameters::GetScmX25519Key(nsString& aRetVal) const
{
  aRetVal.Assign(mScmX25519Key);
}

void BerytusKeyAgreementParameters::GetWebAppX25519Key(nsString& aRetVal) const
{
  aRetVal.Assign(mWebAppX25519Key);
}

void BerytusKeyAgreementParameters::GetHkdfHash(nsString& aRetVal) const
{
  aRetVal.Assign(mHkdfHash);
}

void BerytusKeyAgreementParameters::GetHkdfSalt(JSContext* aCx, JS::MutableHandle<JSObject*> aRetVal)
{
  JSObject* arrayBuffer = JS::NewArrayBufferWithContents(
    aCx, mHkdfSalt.Length(), mHkdfSalt.Elements(),
    JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory
  ); // BRTTODO: Check for memory stuff
  aRetVal.set(arrayBuffer);
}

nsresult BerytusKeyAgreementParameters::GetHkdfSaltAsBase64(nsACString& aRetVal) const
{
  return mozilla::Base64Encode(reinterpret_cast<const char*>(mHkdfSalt.Elements()), mHkdfSalt.Length(), aRetVal);
}

void BerytusKeyAgreementParameters::GetHkdfInfo(JSContext* aCx, JS::MutableHandle<JSObject*> aRetVal)
{

  JSObject* arrayBuffer = JS::NewArrayBufferWithContents(
    aCx, mHkdfInfo.Length(), mHkdfInfo.Elements(),
    JS::NewArrayBufferOutOfMemory::CallerMustFreeMemory
  ); // BRTTODO: Check for memory stuff
  aRetVal.set(arrayBuffer);
}

[[nodiscard]] nsresult BerytusKeyAgreementParameters::GetHkdfInfoAsBase64(nsACString& aRetVal) const
{
  return mozilla::Base64Encode(reinterpret_cast<const char*>(mHkdfInfo.Elements()), mHkdfInfo.Length(), aRetVal);
}

uint16_t BerytusKeyAgreementParameters::AesKeyLength() const
{
  return mAesKeyLength;
}

void BerytusKeyAgreementParameters::ToJSON(
  nsString& aRetVal,
  ErrorResult& aRv
) const
{
  nsresult res;
  nsCString saltBase64;
  nsCString infoBase64;
  res = GetHkdfSaltAsBase64(saltBase64);
  if (NS_FAILED(res)) {
    aRv.ThrowOperationError("Probably OOM when base64-encoding the salt");
    return;
  }
  res = GetHkdfInfoAsBase64(infoBase64);
  if (NS_FAILED(res)) {
    aRv.ThrowOperationError("Probably OOM when base64-encoding the salt");
    return;
  }
  const char16_t* kJSONFmt =
    u"{\n"
    u"  \"sessionId\": %s,\n"
    u"  \"webAppX25519Key\": %s,\n"
    u"  \"scmX25519Key\": %s,\n"
    u"  \"hkdfHash\": %s,\n"
    u"  \"hkdfSalt\": %s,\n"
    u"  \"hkdfInfo\": %s,\n"
    u"  \"aesKeyLength\": %hu\n"
    u"}";

  nsTextFormatter::ssprintf(
    aRetVal,
    kJSONFmt,
    NS_ConvertUTF16toUTF8(mSessionId).get(),
    NS_ConvertUTF16toUTF8(mWebAppX25519Key).get(),
    NS_ConvertUTF16toUTF8(mScmX25519Key).get(),
    NS_ConvertUTF16toUTF8(mHkdfHash).get(),
    saltBase64.get(),
    infoBase64.get(),
    mAesKeyLength
  );
}

} // namespace mozilla::dom
