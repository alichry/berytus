/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusKeyAgreementParameters.h"
#include <cstdint>
#include "BerytusCryptoWebAppActor.h"
#include "ErrorList.h"
#include "js/experimental/TypedData.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Base64.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/HoldDropJSObjects.h"
#include "mozilla/Logging.h"
#include "mozilla/LoggingCore.h"
#include "mozilla/RefPtr.h"
#include "mozilla/dom/BerytusKeyAgreementParametersBinding.h"
#include "mozilla/dom/BerytusSecretManagerActor.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/CryptoBuffer.h"
#include "mozilla/dom/RootedDictionary.h"
#include "mozilla/dom/TypedArray.h"
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDebug.h"
#include "nsIGlobalObject.h"
#include "nsIRandomGenerator.h"
#include "nsISupports.h"
#include "nsString.h"
#include "nsStringFwd.h"
#include "mozilla/Assertions.h"
#include "Crypto.h"
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/dom/Document.h"
#include "nsIX509Cert.h"
#include "nsTArray.h"
#include "nsTDependentSubstring.h"
#include "secoidt.h"

namespace mozilla::dom {

#define BERYTUS_KA_VERSION "0.3"
#define BERYTUS_KA_HKDF_SALT_LEN 32
#define BERYTUS_KA_HKDF_INFO "berytus03 derived"

static mozilla::LazyLogModule sLogger("berytus_ke");

using SupportsToDictionary = BerytusKeyAgreementParameters::SupportsToDictionary;
using Session = BerytusKeyAgreementParameters::Session;
using Fingerprint = BerytusKeyAgreementParameters::Session::Fingerprint;
using Authentication = BerytusKeyAgreementParameters::Authentication;
using Exchange = BerytusKeyAgreementParameters::Exchange;
using Derivation = BerytusKeyAgreementParameters::Derivation;
using Generation = BerytusKeyAgreementParameters::Generation;

void RandomBytes(Span<uint8_t>& aDest, ErrorResult& aRv) {
  nsCOMPtr<nsIRandomGenerator> randomGenerator =
    do_GetService("@mozilla.org/security/random-generator;1");
  if (NS_WARN_IF(!randomGenerator)) {
    aRv.Throw(NS_ERROR_DOM_OPERATION_ERR);
    return;
  }
  if (aDest.Length() == 0) {
    NS_WARNING("Buffer length is 0, cannot fill random bytes.");
    return;
  }
  if (aDest.Length() > 65536) {
    aRv.ThrowQuotaExceededError(
      "RandomBytes can only generate maximum 65536 bytes");
    return;
  }
  nsresult rv = randomGenerator->GenerateRandomBytesInto(aDest.Elements(),
                                                         aDest.Length());
  if (NS_FAILED(rv)) {
    aRv.Throw(NS_ERROR_DOM_OPERATION_ERR);
    return;
  }
}

void Hash(const CryptoBuffer& aData, CryptoBuffer& aResult, ErrorResult& aRv) {
  SECOidTag hashOID(SEC_OID_SHA256);
  uint32_t hashLen = HASH_ResultLenByOidTag(hashOID);
  if (!aResult.SetLength(hashLen, fallible)) {
    aRv.ThrowTypeError("Out of memory");
    return;
  }

  // Compute the hash
  nsresult rv = MapSECStatus(PK11_HashBuf(hashOID, aResult.Elements(),
                                          aData.Elements(), aData.Length()));
  if (NS_FAILED(rv)) {
    aRv.ThrowTypeError("Out of memory");
    return;
  }
}

/**
 * Note(berytus): implementation might leave aJson modified
 *  even when aRv.Failed().
 */
template <typename T>
void ToCanonicalJSON(const T& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const nsString& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const nsLiteralString& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const uint8_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const int8_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const uint16_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const int16_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const uint32_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const int32_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const long& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const int64_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const uint64_t& aValue, nsString& aJson, ErrorResult& aRv);
template<>
void ToCanonicalJSON(const CryptoBuffer& aValue, nsString& aJson, ErrorResult& aRv);
template<typename U>
void ToCanonicalJSON(const Span<U>& aValue, nsString&  aJson, ErrorResult& aRv);

template<>
void ToCanonicalJSON(const nsString& aValue, nsString& aJson, ErrorResult& aRv) {
  if (NS_WARN_IF(!aJson.Append(u"\""_ns, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return;
  }
  nsTDependentSubstring<char16_t> toAppend;
  for (size_t i = 0; i < aValue.Length(); i++) {
    const auto& ch = aValue.CharAt(i);
    switch (ch) {
      case u'"': {
        char16_t token[] = u"\\\"";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'/': {
        char16_t token[] = u"\\/";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\\': {
        char16_t token[] = u"\\\\";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\b': {
        char16_t token[] = u"\\b";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\f': {
        char16_t token[] = u"\\f";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\n': {
        char16_t token[] = u"\\n";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\r': {
        char16_t token[] = u"\\r";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      case u'\t': {
        char16_t token[] = u"\\t";
        toAppend.Rebind(token, (sizeof(token) - sizeof(char16_t)) / sizeof(char16_t));
        break;
      }
      default:
        // check if ch is a control character
        // see https://en.wikipedia.org/wiki/Unicode_control_characters
        if (ch <= '\u001F' || ch == '\u007F') {
          aJson.AppendPrintf("\\u%04x", ch);
          // TODO(berytus): Adopt a fallible approach for the above
        } else {
          toAppend.Rebind(&ch, 1);
        }
    }
    if (toAppend.Length() > 0 &&
        NS_WARN_IF(!aJson.Append(toAppend.Data(), toAppend.Length(), fallible))) {
      aRv.ThrowTypeError("Out of memory");
      return;
    }
  }
  if (NS_WARN_IF(!aJson.Append(u"\""_ns, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return;
  }
}
template<>
void ToCanonicalJSON(const nsLiteralString& aValue, nsString& aJson, ErrorResult& aRv) {
  return ToCanonicalJSON(static_cast<const nsString&>(aValue), aJson, aRv);
}

template<>
void ToCanonicalJSON(const uint8_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%hhu", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const int8_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%hhd", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const uint16_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%hu", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const int16_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%hd", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const uint32_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%u", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const int32_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%d", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const long& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%ld", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const int64_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%lld", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}
template<>
void ToCanonicalJSON(const uint64_t& aValue, nsString& aJson, ErrorResult& aRv) {
  aJson.AppendPrintf("%llu", aValue);
  // TODO(berytus): Adopt a fallible approach for the above
}

template<>
void ToCanonicalJSON(const CryptoBuffer& aValue, nsString& aJson, ErrorResult& aRv) {
  Span<const uint8_t> view(aValue);
  // TODO(berytus): Here
}

template<typename U>
void ToCanonicalJSON(const Span<U>& aValue, nsString&  aJson, ErrorResult& aRv) {
  JSONArrayWriter writer(aJson, aRv);
  NS_ENSURE_TRUE_VOID(writer.Begin());
  for (const auto& element : aValue) {
    NS_ENSURE_TRUE_VOID(writer.Value(element));
  }
  NS_ENSURE_TRUE_VOID(writer.End());
}

JSONStructWriter::JSONStructWriter(nsString& aJson,
                                   ErrorResult& aRv) : mJson(aJson),
                                                       mRv(aRv) {}
JSONStructWriter::~JSONStructWriter() {}

JSONObjectWriter::JSONObjectWriter(nsString& aJson,
                                   ErrorResult& aRv) : JSONStructWriter(aJson, aRv),
                                                       mEmpty(true) {}
JSONObjectWriter::~JSONObjectWriter() {}

bool JSONObjectWriter::Begin() {
  MOZ_ASSERT(!mRv.Failed());
  if (NS_WARN_IF(!mJson.Append(u"{"_ns, fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  return true;
}

bool JSONObjectWriter::End() {
  MOZ_ASSERT(!mRv.Failed());
  if (NS_WARN_IF(!mJson.Append(u"}"_ns, fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  return true;
}

bool JSONObjectWriter::Key(const nsLiteralString& aKey) {
  MOZ_ASSERT(!mRv.Failed());
  if (!mEmpty &&
      NS_WARN_IF(!mJson.Append(u',', fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  ToCanonicalJSON(aKey, mJson, mRv);
  NS_ENSURE_TRUE(!mRv.Failed(), false);
  if (NS_WARN_IF(!mJson.Append(u':', fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  mEmpty = false;
  return true;
}

template <typename T>
bool JSONObjectWriter::Value(const T& aValue) {
  MOZ_ASSERT(!mRv.Failed());
  ToCanonicalJSON(aValue, mJson, mRv);
  return !mRv.Failed();
}

JSONArrayWriter::JSONArrayWriter(
    nsString& aJson,
    ErrorResult& aRv) : JSONStructWriter(aJson, aRv),
                        mEmpty(true) {}
JSONArrayWriter::~JSONArrayWriter() {}

bool JSONArrayWriter::Begin() {
  MOZ_ASSERT(!mRv.Failed());
  if (NS_WARN_IF(!mJson.Append(u"["_ns, fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  return true;
}

bool JSONArrayWriter::End() {
  MOZ_ASSERT(!mRv.Failed());
  if (NS_WARN_IF(!mJson.Append(u"]"_ns, fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  return true;
}

template <typename T>
bool JSONArrayWriter::Value(const T& aValue) {
  MOZ_ASSERT(!mRv.Failed());
  if (!mEmpty &&
      NS_WARN_IF(!mJson.Append(u',', fallible))) {
    mRv.ThrowTypeError("Out of memory");
    return false;
  }
  ToCanonicalJSON(aValue, mJson, mRv);
  NS_ENSURE_TRUE(!mRv.Failed(), false);
  mEmpty = false;
  return true;
}


// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusKeyAgreementParameters, mGlobal, mAuthentication, mSession, mExchange, mDerivation, mGeneration)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusKeyAgreementParameters)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusKeyAgreementParameters)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusKeyAgreementParameters)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusKeyAgreementParameters::BerytusKeyAgreementParameters(
  nsIGlobalObject* aGlobal,
  RefPtr<Session>& aSession,
  RefPtr<Authentication>& aAuthentication,
  RefPtr<Exchange>& aExchange,
  RefPtr<Derivation>& aDerivation,
  RefPtr<Generation>& aGeneration
) : mGlobal(aGlobal),
    mSession(aSession),
    mAuthentication(aAuthentication),
    mExchange(aExchange),
    mDerivation(aDerivation),
    mGeneration(aGeneration) {}

BerytusKeyAgreementParameters::~BerytusKeyAgreementParameters() {}

nsIGlobalObject* BerytusKeyAgreementParameters::GetParentObject() const { return mGlobal; }

JSObject*
BerytusKeyAgreementParameters::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusKeyAgreementParameters_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusKeyAgreementParameters> BerytusKeyAgreementParameters::Create(
  const RefPtr<BerytusChannel>& aChannel,
  const nsAString& aExchangeWebApp,
  const nsAString& aExchangeScm,
  nsTArray<nsString>&& aCiphertextUrls,
  ErrorResult& aRv) {
  MOZ_ASSERT(aChannel);
  nsIGlobalObject* global = aChannel->GetParentObject();
  MOZ_ASSERT(global);
  RefPtr<const BerytusWebAppActor> appActor = aChannel->GetWebAppActor();
  MOZ_ASSERT(appActor);
  if (appActor->Type() != CryptoActor) {
    aRv.ThrowTypeError("Channel is not configured with a crypto web app actor.");
    return nullptr;
  }
  RefPtr<const BerytusCryptoWebAppActor> appCryptoActor =
    static_cast<const BerytusCryptoWebAppActor*>(appActor.get());
  RefPtr<const BerytusSecretManagerActor> scmCryptoActor =
    aChannel->GetSecretManagerActor();

  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  RefPtr<Session> session = Session::Create(aChannel, std::move(aCiphertextUrls), aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  nsString appPubEd25519, scmPubEd25519;
  appCryptoActor->GetEd25519Key(appPubEd25519);
  scmCryptoActor->GetEd25519Key(scmPubEd25519);
  RefPtr<Authentication> authentication = new Authentication(global,
                                                   appPubEd25519,
                                                   scmPubEd25519);
  RefPtr<Exchange> exchange = new Exchange(global, aExchangeWebApp, aExchangeScm);
  RefPtr<Derivation> derivation = Derivation::Create(global, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  RefPtr<Generation> generation = new Generation(global);

  return do_AddRef(new BerytusKeyAgreementParameters(
      global,
      session,
      authentication,
      exchange,
      derivation,
      generation));
}

RefPtr<const Session> BerytusKeyAgreementParameters::GetSession() const {
  MOZ_ASSERT(mSession);
  return mSession;
}
const RefPtr<Session>& BerytusKeyAgreementParameters::GetSession() {
  MOZ_ASSERT(mSession);
  return mSession;
}
RefPtr<const Authentication> BerytusKeyAgreementParameters::GetAuthentication() const {
  MOZ_ASSERT(mAuthentication);
  return mAuthentication;
}
const RefPtr<Authentication>& BerytusKeyAgreementParameters::GetAuthentication() {
  MOZ_ASSERT(mAuthentication);
  return mAuthentication;
}
RefPtr<const Exchange> BerytusKeyAgreementParameters::GetExchange() const {
  MOZ_ASSERT(mExchange);
  return mExchange;
}
const RefPtr<Exchange>& BerytusKeyAgreementParameters::GetExchange() {
  MOZ_ASSERT(mExchange);
  return mExchange;
}
RefPtr<const Derivation> BerytusKeyAgreementParameters::GetDerivation() const {
  MOZ_ASSERT(mDerivation);
  return mDerivation;
}
const RefPtr<Derivation>& BerytusKeyAgreementParameters::GetDerivation() {
  MOZ_ASSERT(mDerivation);
  return mDerivation;
}
RefPtr<const Generation> BerytusKeyAgreementParameters::GetGeneration() const {
  MOZ_ASSERT(mGeneration);
  return mGeneration;
}
const RefPtr<Generation>& BerytusKeyAgreementParameters::GetGeneration() {
  MOZ_ASSERT(mGeneration);
  return mGeneration;
}

void BerytusKeyAgreementParameters::GetSession(
    JSContext* aCx,
    JS::MutableHandle<JS::Value> aValue,
    ErrorResult& aRv) {
  return mSession->ToDictionary(aCx, aValue, aRv);
}
void BerytusKeyAgreementParameters::GetAuthentication(
  JSContext* aCx,
  JS::MutableHandle<JS::Value> aValue,
  ErrorResult& aRv) {
return mAuthentication->ToDictionary(aCx, aValue, aRv);
}
void BerytusKeyAgreementParameters::GetExchange(
    JSContext* aCx,
    JS::MutableHandle<JS::Value> aValue,
    ErrorResult& aRv) {
  return mExchange->ToDictionary(aCx, aValue, aRv);
}
void BerytusKeyAgreementParameters::GetDerivation(
    JSContext* aCx,
    JS::MutableHandle<JS::Value> aValue,
    ErrorResult& aRv) {
  return mDerivation->ToDictionary(aCx, aValue, aRv);
}

void BerytusKeyAgreementParameters::GetGeneration(
    JSContext* aCx,
    JS::MutableHandle<JS::Value> aValue,
    ErrorResult& aRv) {
  return mGeneration->ToDictionary(aCx, aValue, aRv);
}

void BerytusKeyAgreementParameters::ToCanonicalJSON(nsString& aJson, ErrorResult& aRv) const {
  JSONObjectWriter writer(aJson, aRv);

  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"authentication"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(mAuthentication));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"derivation"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(mDerivation));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"exchange"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(mExchange));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"generation"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(mGeneration));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"session"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(mSession));

  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_CYCLE_COLLECTION_WITH_JS_MEMBERS(SupportsToDictionary, (mGlobal), (mCachedDictionary))
NS_IMPL_CYCLE_COLLECTING_ADDREF(SupportsToDictionary)
NS_IMPL_CYCLE_COLLECTING_RELEASE(SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(SupportsToDictionary)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

SupportsToDictionary::SupportsToDictionary(
    nsIGlobalObject* aGlobal,
    HoldDropJSObjectsCaller aHoldDropCaller) : mGlobal(aGlobal),
                                               mCachedDictionary(nullptr),
                                               mHoldDropCaller(aHoldDropCaller) {
  if (mHoldDropCaller == HoldDropJSObjectsCaller::Implicit) {
    mozilla::HoldJSObjects(this);
  }
}

SupportsToDictionary::~SupportsToDictionary() {
  if (mHoldDropCaller == HoldDropJSObjectsCaller::Implicit) {
    mozilla::DropJSObjects(this);
  }
}

void SupportsToDictionary::ClearCachedDictionary() {
  mCachedDictionary = nullptr;
}

void SupportsToDictionary::ToDictionary(JSContext* aCx,
                                        JS::MutableHandle<JS::Value> aValue,
                                        ErrorResult& aRv) {
  if (!mCachedDictionary) {
    CacheDictionary(aCx, aRv);
    NS_ENSURE_TRUE_VOID(!aRv.Failed());
    MOZ_ASSERT(mCachedDictionary);
  }
  aValue.setObjectOrNull(mCachedDictionary);
}

NS_IMPL_ADDREF_INHERITED(Fingerprint, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Fingerprint, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN(Fingerprint)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

Fingerprint::Fingerprint(
    nsIGlobalObject* aGlobal,
    CryptoBuffer&& aSalt,
    CryptoBuffer&& aDigest) : SupportsToDictionary(aGlobal),
                              mSalt(std::move(aSalt)),
                              mDigest(std::move(aDigest)) {}

Fingerprint::~Fingerprint() {}

const nsLiteralString& Fingerprint::GetHash() const {
  return mHash;
}
const nsLiteralString& Fingerprint::GetVersion() const {
  return mVersion;
}
const CryptoBuffer& Fingerprint::GetSalt() const {
  return mSalt;
}
const CryptoBuffer& Fingerprint::GetDigest() const {
  return mDigest;
}

void Fingerprint::CacheDictionary(JSContext* aCx, ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyExchangeSessionFingerprint> dict(aCx);
  dict.mVersion.Assign(mVersion);
  dict.mHash.Assign(mHash);
  JS::Rooted<JSObject*> saltAbObj(aCx, mSalt.ToArrayBuffer(aCx, aRv));
  NS_ENSURE_TRUE_VOID(!aRv.Failed());
  MOZ_ASSERT(saltAbObj);
  JS::Rooted<JSObject*> digestAbObj(aCx, mDigest.ToArrayBuffer(aCx, aRv));
  NS_ENSURE_TRUE_VOID(!aRv.Failed());
  MOZ_ASSERT(digestAbObj);
  if (NS_WARN_IF(!dict.mSalt.SetAsArrayBuffer().Init(saltAbObj))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  if (NS_WARN_IF(!dict.mValue.SetAsArrayBuffer().Init(digestAbObj))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

already_AddRefed<Fingerprint> Fingerprint::Create(
    const RefPtr<const BerytusChannel>& aChannel,
    const std::time_t& aTimestamp,
    ErrorResult& aRv) {
  nsCOMPtr<nsIGlobalObject> global = aChannel->GetParentObject();
  MOZ_ASSERT(global);
  nsPIDOMWindowInner* inner = global->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  Document* doc = inner->GetDoc();
  MOZ_ASSERT(doc);
  nsCOMPtr<nsIChannel> nsCh = doc->GetChannel();
  nsITransportSecurityInfo* securityInfo;
  nsresult rv = nsCh->GetSecurityInfo(&securityInfo);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  nsIX509Cert* x509Cert;
  rv = securityInfo->GetServerCert(&x509Cert);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  CryptoBuffer toDigest, salt, digest;
  // salt:
  if (NS_WARN_IF(!salt.SetLength(32, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  Span<uint8_t> saltSlice(salt.Elements(), salt.Length());
  RandomBytes(saltSlice, aRv);
  if (NS_WARN_IF(!toDigest.AppendElements(salt, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  // id:
  nsString channelId;
  aChannel->GetID(channelId);
  auto utf8Id = NS_ConvertUTF16toUTF8(channelId);
  if (NS_WARN_IF(!toDigest.AppendElements(utf8Id.BeginReading(), utf8Id.Length(), fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  // timestamp:
  if (NS_WARN_IF(!toDigest.AppendElements(&aTimestamp, sizeof(aTimestamp), fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  // origin:
  nsIURI* uri = doc->GetDocumentURI();
  if (NS_WARN_IF(!uri)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  nsCString scheme, hostport, origin;
  uri->GetScheme(scheme);
  uri->GetAsciiHostPort(hostport);
  origin.Append(scheme);
  origin.Append("://");
  origin.Append(hostport);
  if (NS_WARN_IF(!toDigest.AppendElements(origin.BeginReading(), origin.Length(), fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  // cert:
  nsTArray<uint8_t> der;
  rv = x509Cert->GetRawDER(der);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  if (NS_WARN_IF(!toDigest.AppendElements(der, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  if (MOZ_LOG_TEST(sLogger, LogLevel::Info)) {
    nsCString base64;
    rv = Base64Encode(reinterpret_cast<char*>(toDigest.Elements()),
    toDigest.Length(),
    base64);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      NS_WARNING("Cannot print Berytus fingerprint! Base64 encoding error");
    } else {
      MOZ_LOG(sLogger, LogLevel::Info,
        ("GenerateFingerprint(): Digesting -> %.*s\n",
        base64.Length(), base64.Data()));
    }
  }
  Hash(toDigest, digest, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  return do_AddRef(new Fingerprint(global,
                                         std::move(salt),
                                         std::move(digest)));
}

template<>
void ToCanonicalJSON(const RefPtr<Fingerprint>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);
  NS_ENSURE_TRUE_VOID(\
    writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"hash"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetHash()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"salt"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetSalt()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"value"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetDigest()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"version"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetVersion()));

  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_CYCLE_COLLECTION_INHERITED(Session, SupportsToDictionary, mFingerprint)
NS_IMPL_ADDREF_INHERITED(Session, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Session, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(Session)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

Session::Session(
    nsIGlobalObject* aGlobal,
    const nsAString& aId,
    const std::time_t& aTimestamp,
    const RefPtr<Fingerprint>& aFingerprint,
    nsTArray<nsString>&& aCiphertextUrls) : SupportsToDictionary(aGlobal, HoldDropJSObjectsCaller::Explicit),
                                             mId(aId),
                                             mTimestamp(aTimestamp),
                                             mFingerprint(aFingerprint),
                                             mCiphertextUrls(std::move(aCiphertextUrls)) {
  mozilla::HoldJSObjects(this);
}

Session::~Session() {
  mozilla::DropJSObjects(this);
}

already_AddRefed<Session> Session::Create(
    const RefPtr<const BerytusChannel>& aChannel,
    nsTArray<nsString>&& aCiphertextUrls,
    ErrorResult& aRv) {
  MOZ_ASSERT(aChannel);
  nsString sessionId;
  aChannel->GetID(sessionId);
  std::time_t timestamp = std::time(nullptr);
  nsIGlobalObject* global = aChannel->GetParentObject();
  MOZ_ASSERT(global);
  RefPtr<Fingerprint> fingerprint = Fingerprint::Create(aChannel,
                                                        timestamp,
                                                        aRv);
  return do_AddRef(new Session(global,
                               sessionId,
                               timestamp,
                               fingerprint,
                               std::move(aCiphertextUrls)));
}

const nsString& Session::GetID() const {
  return mId;
}
const std::time_t& Session::GetTimestamp() const {
  return mTimestamp;
}
const RefPtr<Fingerprint>& Session::GetFingerprint() const {
  return mFingerprint;
}
Span<const nsString> Session::GetCiphertextUrls() const {
  return Span<const nsString>(mCiphertextUrls);
}

void Session::CacheDictionary(JSContext* aCx,
                     ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyExchangeSession> dict(aCx);
  dict.mId.Assign(mId);
  dict.mTimestamp = mTimestamp;
  JS::Rooted<JS::Value> fingerprint(aCx);
  mFingerprint->ToDictionary(aCx, &fingerprint, aRv);
  NS_ENSURE_TRUE_VOID(!aRv.Failed());
  if (NS_WARN_IF(!dict.mFingerprint.Init(aCx, fingerprint))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

template<>
void ToCanonicalJSON(const RefPtr<Session>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);

  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"ciphertextUrls"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetCiphertextUrls()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"fingerprint"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetFingerprint()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"id"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetID()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"timestamp"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetTimestamp()));

  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_ADDREF_INHERITED(Authentication, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Authentication, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN(Authentication)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

Authentication::Authentication(
    nsIGlobalObject* aGlobal,
    const nsAString& aWebApp,
    const nsAString& aScm) : SupportsToDictionary(aGlobal),
                            mWebApp(aWebApp),
                            mScm(aScm) {}
Authentication::~Authentication() {}

const nsLiteralString& Authentication::GetName() const {
  return mName;
}
const nsString& Authentication::GetWebApp() const {
  return mWebApp;
}
const nsString& Authentication::GetScm() const {
  return mScm;
}
void Authentication::CacheDictionary(JSContext* aCx,
                                     ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyExchangeAuthentication> dict(aCx);
  dict.mName.Assign(mName);
  if (mScm.Length() > 0) {
    dict.mPublic.mScm.Assign(mScm);
  }
  if (mWebApp.Length() > 0) {
    dict.mPublic.mWebApp.Assign(mWebApp);
  }

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

template<>
void ToCanonicalJSON(const RefPtr<Authentication>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);

  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"name"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetName()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"public"_ns));
  {
    JSONObjectWriter pubWriter(aJson, aRv);
    NS_ENSURE_TRUE_VOID(pubWriter.Begin());
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Key(u"scm"_ns));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Value(aValue->GetScm()));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Key(u"webApp"_ns));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Value(aValue->GetWebApp()));
    NS_ENSURE_TRUE_VOID(pubWriter.End());
  }
  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_ADDREF_INHERITED(Exchange, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Exchange, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN(Exchange)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

Exchange::Exchange(nsIGlobalObject* aGlobal,
                   const nsAString& aWebApp,
                   const nsAString& aScm) : SupportsToDictionary(aGlobal),
                                            mWebApp(aWebApp),
                                            mScm(aScm) {}

Exchange::~Exchange() {}
const nsLiteralString& Exchange::GetName() const {
  return mName;
}
const nsString& Exchange::GetWebApp() const {
  return mWebApp;
}
const nsString& Exchange::GetScm() const {
  return mScm;
}
void Exchange::CacheDictionary(JSContext* aCx,
                               ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyExchangeParams> dict(aCx);
  dict.mName.Assign(mName);
  if (mScm.Length() > 0) {
    dict.mPublic.mScm.Assign(mScm);
  }
  if (mWebApp.Length() > 0) {
    dict.mPublic.mWebApp.Assign(mWebApp);
  }

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

template<>
void ToCanonicalJSON(const RefPtr<Exchange>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);
  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"name"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetName()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"public"_ns));
  {
    JSONObjectWriter pubWriter(aJson, aRv);
    NS_ENSURE_TRUE_VOID(pubWriter.Begin());
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Key(u"scm"_ns));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Value(aValue->GetScm()));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Key(u"webApp"_ns));
    NS_ENSURE_TRUE_VOID(\
      pubWriter.Value(aValue->GetWebApp()));
    NS_ENSURE_TRUE_VOID(pubWriter.End());
  }
  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_ADDREF_INHERITED(Derivation, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Derivation, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN(Derivation)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

CryptoBuffer Derivation::mInfo;

already_AddRefed<Derivation> Derivation::Create(
  nsIGlobalObject* aGlobal,
  ErrorResult& aRv) {
  if (mInfo.Length() == 0) {
    if (NS_WARN_IF(!mInfo.AppendElements(BERYTUS_KA_HKDF_INFO, sizeof(BERYTUS_KA_HKDF_INFO), fallible))) {
      aRv.ThrowTypeError("Out of memory");
      return nullptr;
    }
  }
  CryptoBuffer salt;
  if (NS_WARN_IF(!salt.SetLength(BERYTUS_KA_HKDF_SALT_LEN, fallible))) {
    aRv.ThrowTypeError("Out of memory");
    return nullptr;
  }
  Span<uint8_t> slice(salt.Elements(), salt.Length());
  RandomBytes(slice, aRv);
  NS_ENSURE_TRUE(!aRv.Failed(), nullptr);
  return do_AddRef(new Derivation(aGlobal, std::move(salt)));
}

Derivation::Derivation(nsIGlobalObject* aGlobal,
                       CryptoBuffer&& aSalt) : SupportsToDictionary(aGlobal),
                                               mSalt(std::move(aSalt)) {}
Derivation::~Derivation() {}
const nsLiteralString& Derivation::GetName() const {
  return mName;
}
const nsLiteralString& Derivation::GetHash() const {
  return mHash;
}
const CryptoBuffer& Derivation::GetSalt() const {
  return mSalt;
}
const CryptoBuffer& Derivation::GetInfo() const {
  MOZ_ASSERT(mInfo.Length() > 0);
  return mInfo;
}
void Derivation::CacheDictionary(JSContext* aCx,
                                 ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyDerivationParams> dict(aCx);
  dict.mName.Assign(mName);
  dict.mHash.Assign(mHash);
  JS::Rooted<JSObject*> saltAbObj(aCx, mSalt.ToArrayBuffer(aCx, aRv));
  NS_ENSURE_TRUE_VOID(!aRv.Failed());
  MOZ_ASSERT(saltAbObj);
  if (NS_WARN_IF(!dict.mSalt.SetAsArrayBuffer().Init(saltAbObj))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  MOZ_ASSERT(mInfo.Length() > 0);
  JS::Rooted<JSObject*> infoAbObj(aCx, mInfo.ToArrayBuffer(aCx, aRv));
  NS_ENSURE_TRUE_VOID(!aRv.Failed());
  MOZ_ASSERT(infoAbObj);
  if (NS_WARN_IF(!dict.mInfo.SetAsArrayBuffer().Init(infoAbObj))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

template<>
void ToCanonicalJSON(const RefPtr<Derivation>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);
  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"hash"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetHash()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"info"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetInfo()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"name"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetName()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"salt"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetSalt()));

  NS_ENSURE_TRUE_VOID(writer.End());
}

NS_IMPL_ADDREF_INHERITED(Generation, SupportsToDictionary)
NS_IMPL_RELEASE_INHERITED(Generation, SupportsToDictionary)
NS_INTERFACE_MAP_BEGIN(Generation)
NS_INTERFACE_MAP_END_INHERITING(SupportsToDictionary)

Generation::Generation(
    nsIGlobalObject* aGlobal) : SupportsToDictionary(aGlobal) {}
Generation::~Generation() {}
const nsLiteralString& Generation::GetName() {
  return mName;
}
const uint16_t& Generation::GetLength() {
  return mLength;
}
void Generation::CacheDictionary(JSContext* aCx,
                                 ErrorResult& aRv) {
  JSAutoRealm ar(aCx, mGlobal->GetGlobalJSObject());

  RootedDictionary<BerytusKeyGenParams> dict(aCx);
  dict.mName.Assign(mName);
  dict.mLength = mLength;

  JS::Rooted<JS::Value> val(aCx);
  if (NS_WARN_IF(!dict.ToObjectInternal(aCx, &val))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return;
  }
  mCachedDictionary = val.toObjectOrNull();
}

template<>
void ToCanonicalJSON(const RefPtr<Generation>& aValue, nsString& aJson, ErrorResult& aRv) {
  JSONObjectWriter writer(aJson, aRv);
  NS_ENSURE_TRUE_VOID(writer.Begin());

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"length"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetLength()));

  NS_ENSURE_TRUE_VOID(\
    writer.Key(u"name"_ns));
  NS_ENSURE_TRUE_VOID(\
    writer.Value(aValue->GetName()));

  NS_ENSURE_TRUE_VOID(writer.End());
}

} // namespace mozilla::dom
