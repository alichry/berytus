/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BERYTUS_AGENTPROXY_H_
#define BERYTUS_AGENTPROXY_H_

#include "nsISupports.h"
#include "nsCycleCollectionParticipant.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/TypedArray.h" // ArrayBuffer
#include "mozilla/Variant.h"
#include "mozilla/dom/DOMException.h" // for Failure's Exception
#include "mozilla/Logging.h"

using mozilla::LogLevel;

namespace mozilla::berytus {

template <typename RV>
class IJSWord {
public:
  static bool ToJSVal(JSContext* aCx,
                      const RV& aValue,
                      JS::MutableHandle<JS::Value> aRv);
  static bool FromJSVal(JSContext* aCx,
                        JS::Handle<JS::Value> aValue,
                        RV& aRv);
};

using ArrayBuffer = mozilla::dom::ArrayBuffer;
using ArrayBufferView = mozilla::dom::ArrayBufferView;

template <typename... T>
class SafeVariant {
public:
  SafeVariant() = delete;
};

bool JSValIsNumber(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NumberFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv);
bool NumberToJSVal(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv);
struct DocumentMetadata : IJSWord<DocumentMetadata> {
  double mId;
  DocumentMetadata() = default;
  DocumentMetadata(double&& aId) : mId(std::move(aId)) {}
  DocumentMetadata(DocumentMetadata&& aOther) : mId(std::move(aOther.mId))  {}
  
  ~DocumentMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct PreliminaryRequestContext : IJSWord<PreliminaryRequestContext> {
  DocumentMetadata mDocument;
  PreliminaryRequestContext() = default;
  PreliminaryRequestContext(DocumentMetadata&& aDocument) : mDocument(std::move(aDocument)) {}
  PreliminaryRequestContext(PreliminaryRequestContext&& aOther) : mDocument(std::move(aOther.mDocument))  {}
  
  ~PreliminaryRequestContext() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv);
  static bool ToJSVal(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsString(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool StringFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv);
bool StringToJSVal(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv);
struct CryptoActor : IJSWord<CryptoActor> {
  nsString mEd25519Key;
  CryptoActor() = default;
  CryptoActor(nsString&& aEd25519Key) : mEd25519Key(std::move(aEd25519Key)) {}
  CryptoActor(CryptoActor&& aOther) : mEd25519Key(std::move(aOther.mEd25519Key))  {}
  
  ~CryptoActor() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CryptoActor& aRv);
  static bool ToJSVal(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct UriParams : IJSWord<UriParams> {
  nsString mUri;
  nsString mScheme;
  nsString mHostname;
  double mPort;
  nsString mPath;
  UriParams() = default;
  UriParams(nsString&& aUri, nsString&& aScheme, nsString&& aHostname, double&& aPort, nsString&& aPath) : mUri(std::move(aUri)), mScheme(std::move(aScheme)), mHostname(std::move(aHostname)), mPort(std::move(aPort)), mPath(std::move(aPath)) {}
  UriParams(UriParams&& aOther) : mUri(std::move(aOther.mUri)), mScheme(std::move(aOther.mScheme)), mHostname(std::move(aOther.mHostname)), mPort(std::move(aOther.mPort)), mPath(std::move(aOther.mPath))  {}
  
  ~UriParams() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv);
  static bool ToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct OriginActor : IJSWord<OriginActor> {
  UriParams mOriginalUri;
  UriParams mCurrentUri;
  OriginActor() = default;
  OriginActor(UriParams&& aOriginalUri, UriParams&& aCurrentUri) : mOriginalUri(std::move(aOriginalUri)), mCurrentUri(std::move(aCurrentUri)) {}
  OriginActor(OriginActor&& aOther) : mOriginalUri(std::move(aOther.mOriginalUri)), mCurrentUri(std::move(aOther.mCurrentUri))  {}
  
  ~OriginActor() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv);
  static bool ToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<CryptoActor, OriginActor> : IJSWord<SafeVariant<CryptoActor, OriginActor>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<CryptoActor, OriginActor>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<CryptoActor, OriginActor>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<CryptoActor, OriginActor> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<CryptoActor, OriginActor>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<CryptoActor, OriginActor>* mVariant;
};
struct GetSigningKeyArgs : IJSWord<GetSigningKeyArgs> {
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  GetSigningKeyArgs() = default;
  GetSigningKeyArgs(SafeVariant<CryptoActor, OriginActor>&& aWebAppActor) : mWebAppActor(std::move(aWebAppActor)) {}
  GetSigningKeyArgs(GetSigningKeyArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor))  {}
  
  ~GetSigningKeyArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetSigningKeyArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE nsCString("An error has occurred"_ns)
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME nsCString("An error has occurred"_ns)
struct Failure {
  RefPtr<mozilla::dom::Exception> mException;

  Failure() : Failure(NS_ERROR_FAILURE) {}
  Failure(nsresult res) : mException(new mozilla::dom::Exception(BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE, NS_ERROR_FAILURE, BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME)) {}

  ErrorResult ToErrorResult() const;
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Failure& aRv);
};
using ManagerGetSigningKeyResult = MozPromise<nsString, Failure, true>;
bool JSValIsnsTArray_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<nsString>& aRv);
bool nsTArray_nsString_ToJSVal(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_nsTArray_nsString__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsTArray_nsString__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<nsString>>& aRv);
bool Maybe_nsTArray_nsString__ToJSVal(JSContext* aCx, const Maybe<nsTArray<nsString>>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsBool(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool BoolFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool BoolToJSVal(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsString>& aRv);
bool Maybe_nsString_ToJSVal(JSContext* aCx, const Maybe<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_double_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_double_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<double>& aRv);
bool Maybe_double_ToJSVal(JSContext* aCx, const Maybe<double>& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialAccountIdentity : IJSWord<PartialAccountIdentity> {
  nsString mFieldId;
  nsString mFieldValue;
  PartialAccountIdentity() = default;
  PartialAccountIdentity(nsString&& aFieldId, nsString&& aFieldValue) : mFieldId(std::move(aFieldId)), mFieldValue(std::move(aFieldValue)) {}
  PartialAccountIdentity(PartialAccountIdentity&& aOther) : mFieldId(std::move(aOther.mFieldId)), mFieldValue(std::move(aOther.mFieldValue))  {}
  
  ~PartialAccountIdentity() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialAccountIdentity& aRv);
  static bool ToJSVal(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_PartialAccountIdentity_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_PartialAccountIdentity_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<PartialAccountIdentity>& aRv);
bool nsTArray_PartialAccountIdentity_ToJSVal(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_nsTArray_PartialAccountIdentity__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsTArray_PartialAccountIdentity__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<PartialAccountIdentity>>& aRv);
bool Maybe_nsTArray_PartialAccountIdentity__ToJSVal(JSContext* aCx, const Maybe<nsTArray<PartialAccountIdentity>>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AccountConstraints : IJSWord<AccountConstraints> {
  Maybe<nsString> mCategory;
  Maybe<double> mSchemaVersion;
  Maybe<nsTArray<PartialAccountIdentity>> mIdentity;
  AccountConstraints() = default;
  AccountConstraints(Maybe<nsString>&& aCategory, Maybe<double>&& aSchemaVersion, Maybe<nsTArray<PartialAccountIdentity>>&& aIdentity) : mCategory(std::move(aCategory)), mSchemaVersion(std::move(aSchemaVersion)), mIdentity(std::move(aIdentity)) {}
  AccountConstraints(AccountConstraints&& aOther) : mCategory(std::move(aOther.mCategory)), mSchemaVersion(std::move(aOther.mSchemaVersion)), mIdentity(std::move(aOther.mIdentity))  {}
  
  ~AccountConstraints() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AccountConstraints& aRv);
  static bool ToJSVal(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsMaybe_AccountConstraints_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_AccountConstraints_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<AccountConstraints>& aRv);
bool Maybe_AccountConstraints_ToJSVal(JSContext* aCx, const Maybe<AccountConstraints>& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChannelConstraints : IJSWord<ChannelConstraints> {
  Maybe<nsTArray<nsString>> mSecretManagerPublicKey;
  bool mEnableEndToEndEncryption;
  Maybe<AccountConstraints> mAccount;
  ChannelConstraints() = default;
  ChannelConstraints(Maybe<nsTArray<nsString>>&& aSecretManagerPublicKey, bool&& aEnableEndToEndEncryption, Maybe<AccountConstraints>&& aAccount) : mSecretManagerPublicKey(std::move(aSecretManagerPublicKey)), mEnableEndToEndEncryption(std::move(aEnableEndToEndEncryption)), mAccount(std::move(aAccount)) {}
  ChannelConstraints(ChannelConstraints&& aOther) : mSecretManagerPublicKey(std::move(aOther.mSecretManagerPublicKey)), mEnableEndToEndEncryption(std::move(aOther.mEnableEndToEndEncryption)), mAccount(std::move(aOther.mAccount))  {}
  
  ~ChannelConstraints() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv);
  static bool ToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GetCredentialsMetadataArgs : IJSWord<GetCredentialsMetadataArgs> {
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  ChannelConstraints mChannelConstraints;
  AccountConstraints mAccountConstraints;
  GetCredentialsMetadataArgs() = default;
  GetCredentialsMetadataArgs(SafeVariant<CryptoActor, OriginActor>&& aWebAppActor, ChannelConstraints&& aChannelConstraints, AccountConstraints&& aAccountConstraints) : mWebAppActor(std::move(aWebAppActor)), mChannelConstraints(std::move(aChannelConstraints)), mAccountConstraints(std::move(aAccountConstraints)) {}
  GetCredentialsMetadataArgs(GetCredentialsMetadataArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor)), mChannelConstraints(std::move(aOther.mChannelConstraints)), mAccountConstraints(std::move(aOther.mAccountConstraints))  {}
  
  ~GetCredentialsMetadataArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ManagerGetCredentialsMetadataResult = MozPromise<double, Failure, true>;
struct ChannelMetadata : IJSWord<ChannelMetadata> {
  nsString mId;
  ChannelConstraints mConstraints;
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  CryptoActor mScmActor;
  ChannelMetadata() = default;
  ChannelMetadata(nsString&& aId, ChannelConstraints&& aConstraints, SafeVariant<CryptoActor, OriginActor>&& aWebAppActor, CryptoActor&& aScmActor) : mId(std::move(aId)), mConstraints(std::move(aConstraints)), mWebAppActor(std::move(aWebAppActor)), mScmActor(std::move(aScmActor)) {}
  ChannelMetadata(ChannelMetadata&& aOther) : mId(std::move(aOther.mId)), mConstraints(std::move(aOther.mConstraints)), mWebAppActor(std::move(aOther.mWebAppActor)), mScmActor(std::move(aOther.mScmActor))  {}
  
  ~ChannelMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RequestContext : IJSWord<RequestContext> {
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContext() = default;
  RequestContext(ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContext(RequestContext&& aOther) : mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  
  ~RequestContext() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct InitialKeyExchangeParametersDraft : IJSWord<InitialKeyExchangeParametersDraft> {
  nsString mChannelId;
  nsString mWebAppX25519Key;
  InitialKeyExchangeParametersDraft() = default;
  InitialKeyExchangeParametersDraft(nsString&& aChannelId, nsString&& aWebAppX25519Key) : mChannelId(std::move(aChannelId)), mWebAppX25519Key(std::move(aWebAppX25519Key)) {}
  InitialKeyExchangeParametersDraft(InitialKeyExchangeParametersDraft&& aOther) : mChannelId(std::move(aOther.mChannelId)), mWebAppX25519Key(std::move(aOther.mWebAppX25519Key))  {}
  
  ~InitialKeyExchangeParametersDraft() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv);
  static bool ToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GenerateKeyExchangeParametersArgs : IJSWord<GenerateKeyExchangeParametersArgs> {
  InitialKeyExchangeParametersDraft mParamsDraft;
  GenerateKeyExchangeParametersArgs() = default;
  GenerateKeyExchangeParametersArgs(InitialKeyExchangeParametersDraft&& aParamsDraft) : mParamsDraft(std::move(aParamsDraft)) {}
  GenerateKeyExchangeParametersArgs(GenerateKeyExchangeParametersArgs&& aOther) : mParamsDraft(std::move(aOther.mParamsDraft))  {}
  
  ~GenerateKeyExchangeParametersArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GenerateKeyExchangeParametersArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const GenerateKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsArrayBuffer(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ArrayBufferFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBuffer& aRv);
bool ArrayBufferToJSVal(JSContext* aCx, const ArrayBuffer& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialKeyExchangeParametersFromScm : IJSWord<PartialKeyExchangeParametersFromScm> {
  nsString mScmX25519Key;
  nsString mHkdfHash;
  ArrayBuffer mHkdfSalt;
  ArrayBuffer mHkdfInfo;
  double mAesKeyLength;
  PartialKeyExchangeParametersFromScm() = default;
  PartialKeyExchangeParametersFromScm(nsString&& aScmX25519Key, nsString&& aHkdfHash, ArrayBuffer&& aHkdfSalt, ArrayBuffer&& aHkdfInfo, double&& aAesKeyLength) : mScmX25519Key(std::move(aScmX25519Key)), mHkdfHash(std::move(aHkdfHash)), mHkdfSalt(std::move(aHkdfSalt)), mHkdfInfo(std::move(aHkdfInfo)), mAesKeyLength(std::move(aAesKeyLength)) {}
  PartialKeyExchangeParametersFromScm(PartialKeyExchangeParametersFromScm&& aOther) : mScmX25519Key(std::move(aOther.mScmX25519Key)), mHkdfHash(std::move(aOther.mHkdfHash)), mHkdfSalt(std::move(aOther.mHkdfSalt)), mHkdfInfo(std::move(aOther.mHkdfInfo)), mAesKeyLength(std::move(aOther.mAesKeyLength))  {}
  
  ~PartialKeyExchangeParametersFromScm() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialKeyExchangeParametersFromScm& aRv);
  static bool ToJSVal(JSContext* aCx, const PartialKeyExchangeParametersFromScm& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ChannelGenerateKeyExchangeParametersResult = MozPromise<PartialKeyExchangeParametersFromScm, Failure, true>;
struct KeyExchangeParameters : IJSWord<KeyExchangeParameters> {
  nsString mPacket;
  nsString mChannelId;
  nsString mWebAppX25519Key;
  nsString mScmX25519Key;
  nsString mHkdfHash;
  ArrayBuffer mHkdfSalt;
  ArrayBuffer mHkdfInfo;
  double mAesKeyLength;
  KeyExchangeParameters() = default;
  KeyExchangeParameters(nsString&& aPacket, nsString&& aChannelId, nsString&& aWebAppX25519Key, nsString&& aScmX25519Key, nsString&& aHkdfHash, ArrayBuffer&& aHkdfSalt, ArrayBuffer&& aHkdfInfo, double&& aAesKeyLength) : mPacket(std::move(aPacket)), mChannelId(std::move(aChannelId)), mWebAppX25519Key(std::move(aWebAppX25519Key)), mScmX25519Key(std::move(aScmX25519Key)), mHkdfHash(std::move(aHkdfHash)), mHkdfSalt(std::move(aHkdfSalt)), mHkdfInfo(std::move(aHkdfInfo)), mAesKeyLength(std::move(aAesKeyLength)) {}
  KeyExchangeParameters(KeyExchangeParameters&& aOther) : mPacket(std::move(aOther.mPacket)), mChannelId(std::move(aOther.mChannelId)), mWebAppX25519Key(std::move(aOther.mWebAppX25519Key)), mScmX25519Key(std::move(aOther.mScmX25519Key)), mHkdfHash(std::move(aOther.mHkdfHash)), mHkdfSalt(std::move(aOther.mHkdfSalt)), mHkdfInfo(std::move(aOther.mHkdfInfo)), mAesKeyLength(std::move(aOther.mAesKeyLength))  {}
  
  ~KeyExchangeParameters() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct EnableEndToEndEncryptionArgs : IJSWord<EnableEndToEndEncryptionArgs> {
  KeyExchangeParameters mParams;
  ArrayBuffer mWebAppPacketSignature;
  EnableEndToEndEncryptionArgs() = default;
  EnableEndToEndEncryptionArgs(KeyExchangeParameters&& aParams, ArrayBuffer&& aWebAppPacketSignature) : mParams(std::move(aParams)), mWebAppPacketSignature(std::move(aWebAppPacketSignature)) {}
  EnableEndToEndEncryptionArgs(EnableEndToEndEncryptionArgs&& aOther) : mParams(std::move(aOther.mParams)), mWebAppPacketSignature(std::move(aOther.mWebAppPacketSignature))  {}
  
  ~EnableEndToEndEncryptionArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ChannelEnableEndToEndEncryptionResult = MozPromise<ArrayBuffer, Failure, true>;
using ChannelCloseChannelResult = MozPromise<void*, Failure, true>;
struct ELoginUserIntent {
  uint8_t mVal;
  ELoginUserIntent() : mVal(0) {}
  ELoginUserIntent(uint8_t aVal) : mVal(aVal) {}
  ELoginUserIntent(ELoginUserIntent&& aOther) : mVal(std::move(aOther.mVal)) {}
  static ELoginUserIntent PendingDeclaration();
  static ELoginUserIntent Authenticate();
  static ELoginUserIntent Register();
  bool IsPendingDeclaration() const;
  bool IsAuthenticate() const;
  bool IsRegister() const;
  void SetAsPendingDeclaration();
  void SetAsAuthenticate();
  void SetAsRegister();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, ELoginUserIntent& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv);
  static bool ToJSVal(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct RequestedUserAttribute : IJSWord<RequestedUserAttribute> {
  nsString mId;
  bool mRequired;
  RequestedUserAttribute() = default;
  RequestedUserAttribute(nsString&& aId, bool&& aRequired) : mId(std::move(aId)), mRequired(std::move(aRequired)) {}
  RequestedUserAttribute(RequestedUserAttribute&& aOther) : mId(std::move(aOther.mId)), mRequired(std::move(aOther.mRequired))  {}
  
  ~RequestedUserAttribute() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_RequestedUserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_RequestedUserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<RequestedUserAttribute>& aRv);
bool nsTArray_RequestedUserAttribute_ToJSVal(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EBerytusFieldType {
  uint8_t mVal;
  EBerytusFieldType() : mVal(0) {}
  EBerytusFieldType(uint8_t aVal) : mVal(aVal) {}
  EBerytusFieldType(EBerytusFieldType&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EBerytusFieldType Identity();
  static EBerytusFieldType ForeignIdentity();
  static EBerytusFieldType Password();
  static EBerytusFieldType SecurePassword();
  static EBerytusFieldType ConsumablePassword();
  static EBerytusFieldType Key();
  static EBerytusFieldType SharedKey();
  static EBerytusFieldType Custom();
  bool IsIdentity() const;
  bool IsForeignIdentity() const;
  bool IsPassword() const;
  bool IsSecurePassword() const;
  bool IsConsumablePassword() const;
  bool IsKey() const;
  bool IsSharedKey() const;
  bool IsCustom() const;
  void SetAsIdentity();
  void SetAsForeignIdentity();
  void SetAsPassword();
  void SetAsSecurePassword();
  void SetAsConsumablePassword();
  void SetAsKey();
  void SetAsSharedKey();
  void SetAsCustom();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EBerytusFieldType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EBerytusFieldType& aRv);
  static bool ToJSVal(JSContext* aCx, const EBerytusFieldType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct FieldInfo : IJSWord<FieldInfo> {
  nsString mId;
  EBerytusFieldType mType;
  FieldInfo() = default;
  FieldInfo(nsString&& aId, EBerytusFieldType&& aType) : mId(std::move(aId)), mType(std::move(aType)) {}
  FieldInfo(FieldInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType))  {}
  
  ~FieldInfo() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldInfo& aRv);
  static bool ToJSVal(JSContext* aCx, const FieldInfo& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_FieldInfo_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_FieldInfo_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<FieldInfo>& aRv);
bool nsTArray_FieldInfo_ToJSVal(JSContext* aCx, const nsTArray<FieldInfo>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EOperationType {
  uint8_t mVal;
  EOperationType() : mVal(0) {}
  EOperationType(uint8_t aVal) : mVal(aVal) {}
  EOperationType(EOperationType&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EOperationType PendingDeclaration();
  static EOperationType Registration();
  static EOperationType Authentication();
  bool IsPendingDeclaration() const;
  bool IsRegistration() const;
  bool IsAuthentication() const;
  void SetAsPendingDeclaration();
  void SetAsRegistration();
  void SetAsAuthentication();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EOperationType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationType& aRv);
  static bool ToJSVal(JSContext* aCx, const EOperationType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct EOperationStatus {
  uint8_t mVal;
  EOperationStatus() : mVal(0) {}
  EOperationStatus(uint8_t aVal) : mVal(aVal) {}
  EOperationStatus(EOperationStatus&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EOperationStatus Pending();
  static EOperationStatus Created();
  static EOperationStatus Aborted();
  static EOperationStatus Finished();
  bool IsPending() const;
  bool IsCreated() const;
  bool IsAborted() const;
  bool IsFinished() const;
  void SetAsPending();
  void SetAsCreated();
  void SetAsAborted();
  void SetAsFinished();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EOperationStatus& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv);
  static bool ToJSVal(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct OperationState : IJSWord<OperationState> {
  
  OperationState() = default;
  
  ~OperationState() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv);
  static bool ToJSVal(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct LoginOperationMetadata : IJSWord<LoginOperationMetadata> {
  ELoginUserIntent mIntent;
  nsTArray<RequestedUserAttribute> mRequestedUserAttributes;
  nsTArray<FieldInfo> mFields;
  nsString mId;
  EOperationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  LoginOperationMetadata() = default;
  LoginOperationMetadata(ELoginUserIntent&& aIntent, nsTArray<RequestedUserAttribute>&& aRequestedUserAttributes, nsTArray<FieldInfo>&& aFields, nsString&& aId, EOperationType&& aType, EOperationStatus&& aStatus, OperationState&& aState) : mIntent(std::move(aIntent)), mRequestedUserAttributes(std::move(aRequestedUserAttributes)), mFields(std::move(aFields)), mId(std::move(aId)), mType(std::move(aType)), mStatus(std::move(aStatus)), mState(std::move(aState)) {}
  LoginOperationMetadata(LoginOperationMetadata&& aOther) : mIntent(std::move(aOther.mIntent)), mRequestedUserAttributes(std::move(aOther.mRequestedUserAttributes)), mFields(std::move(aOther.mFields)), mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState))  {}
  
  ~LoginOperationMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveOperationArgs : IJSWord<ApproveOperationArgs> {
  LoginOperationMetadata mOperation;
  ApproveOperationArgs() = default;
  ApproveOperationArgs(LoginOperationMetadata&& aOperation) : mOperation(std::move(aOperation)) {}
  ApproveOperationArgs(ApproveOperationArgs&& aOther) : mOperation(std::move(aOther.mOperation))  {}
  
  ~ApproveOperationArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginApproveOperationResult = MozPromise<ELoginUserIntent, Failure, true>;
struct OperationMetadata : IJSWord<OperationMetadata> {
  nsString mId;
  EOperationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  OperationMetadata() = default;
  OperationMetadata(nsString&& aId, EOperationType&& aType, EOperationStatus&& aStatus, OperationState&& aState) : mId(std::move(aId)), mType(std::move(aType)), mStatus(std::move(aStatus)), mState(std::move(aState)) {}
  OperationMetadata(OperationMetadata&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState))  {}
  
  ~OperationMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RequestContextWithOperation : IJSWord<RequestContextWithOperation> {
  OperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContextWithOperation() = default;
  RequestContextWithOperation(OperationMetadata&& aOperation, ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mOperation(std::move(aOperation)), mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContextWithOperation(RequestContextWithOperation&& aOther) : mOperation(std::move(aOther.mOperation)), mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  
  ~RequestContextWithOperation() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginCloseOpeationResult = MozPromise<void*, Failure, true>;
struct EMetadataStatus {
  uint8_t mVal;
  EMetadataStatus() : mVal(0) {}
  EMetadataStatus(uint8_t aVal) : mVal(aVal) {}
  EMetadataStatus(EMetadataStatus&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EMetadataStatus Pending();
  static EMetadataStatus Created();
  static EMetadataStatus Retired();
  bool IsPending() const;
  bool IsCreated() const;
  bool IsRetired() const;
  void SetAsPending();
  void SetAsCreated();
  void SetAsRetired();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EMetadataStatus& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv);
  static bool ToJSVal(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct RecordMetadata : IJSWord<RecordMetadata> {
  double mVersion;
  EMetadataStatus mStatus;
  nsString mCategory;
  nsString mChangePassUrl;
  RecordMetadata() = default;
  RecordMetadata(double&& aVersion, EMetadataStatus&& aStatus, nsString&& aCategory, nsString&& aChangePassUrl) : mVersion(std::move(aVersion)), mStatus(std::move(aStatus)), mCategory(std::move(aCategory)), mChangePassUrl(std::move(aChangePassUrl)) {}
  RecordMetadata(RecordMetadata&& aOther) : mVersion(std::move(aOther.mVersion)), mStatus(std::move(aOther.mStatus)), mCategory(std::move(aOther.mCategory)), mChangePassUrl(std::move(aOther.mChangePassUrl))  {}
  
  ~RecordMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginGetRecordMetadataResult = MozPromise<RecordMetadata, Failure, true>;
struct UpdateMetadataArgs : IJSWord<UpdateMetadataArgs> {
  RecordMetadata mMetadata;
  UpdateMetadataArgs() = default;
  UpdateMetadataArgs(RecordMetadata&& aMetadata) : mMetadata(std::move(aMetadata)) {}
  UpdateMetadataArgs(UpdateMetadataArgs&& aOther) : mMetadata(std::move(aOther.mMetadata))  {}
  
  ~UpdateMetadataArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginUpdateMetadataResult = MozPromise<void*, Failure, true>;
struct ApproveTransitionToAuthOpArgs : IJSWord<ApproveTransitionToAuthOpArgs> {
  LoginOperationMetadata mNewAuthOp;
  ApproveTransitionToAuthOpArgs() = default;
  ApproveTransitionToAuthOpArgs(LoginOperationMetadata&& aNewAuthOp) : mNewAuthOp(std::move(aNewAuthOp)) {}
  ApproveTransitionToAuthOpArgs(ApproveTransitionToAuthOpArgs&& aOther) : mNewAuthOp(std::move(aOther.mNewAuthOp))  {}
  
  ~ApproveTransitionToAuthOpArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationApproveTransitionToAuthOpResult = MozPromise<void*, Failure, true>;
bool JSValIsArrayBufferView(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ArrayBufferViewFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBufferView& aRv);
bool ArrayBufferViewToJSVal(JSContext* aCx, const ArrayBufferView& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<ArrayBuffer, ArrayBufferView> : IJSWord<SafeVariant<ArrayBuffer, ArrayBufferView>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<ArrayBuffer, ArrayBufferView>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<ArrayBuffer, ArrayBufferView>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<ArrayBuffer, ArrayBufferView> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<ArrayBuffer, ArrayBufferView>* mVariant;
};
bool JSValIsNothing(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NothingFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
bool NothingToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<ArrayBuffer, ArrayBufferView, Nothing> : IJSWord<SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<ArrayBuffer, ArrayBufferView, Nothing>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<ArrayBuffer, ArrayBufferView, Nothing> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<ArrayBuffer, ArrayBufferView, Nothing>* mVariant;
};
struct AesGcmParams : IJSWord<AesGcmParams> {
  SafeVariant<ArrayBuffer, ArrayBufferView> mIv;
  SafeVariant<ArrayBuffer, ArrayBufferView, Nothing> mAdditionalData;
  Maybe<double> mTagLength;
  nsString mName;
  AesGcmParams() = default;
  AesGcmParams(SafeVariant<ArrayBuffer, ArrayBufferView>&& aIv, SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>&& aAdditionalData, Maybe<double>&& aTagLength, nsString&& aName) : mIv(std::move(aIv)), mAdditionalData(std::move(aAdditionalData)), mTagLength(std::move(aTagLength)), mName(std::move(aName)) {}
  AesGcmParams(AesGcmParams&& aOther) : mIv(std::move(aOther.mIv)), mAdditionalData(std::move(aOther.mAdditionalData)), mTagLength(std::move(aOther.mTagLength)), mName(std::move(aOther.mName))  {}
  
  ~AesGcmParams() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AesGcmParams& aRv);
  static bool ToJSVal(JSContext* aCx, const AesGcmParams& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusEncryptedPacket : IJSWord<BerytusEncryptedPacket> {
  AesGcmParams mParameters;
  ArrayBuffer mCiphertext;
  BerytusEncryptedPacket() = default;
  BerytusEncryptedPacket(AesGcmParams&& aParameters, ArrayBuffer&& aCiphertext) : mParameters(std::move(aParameters)), mCiphertext(std::move(aCiphertext)) {}
  BerytusEncryptedPacket(BerytusEncryptedPacket&& aOther) : mParameters(std::move(aOther.mParameters)), mCiphertext(std::move(aOther.mCiphertext))  {}
  
  ~BerytusEncryptedPacket() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusEncryptedPacket& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusEncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> : IJSWord<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>* mVariant;
};
struct UserAttribute : IJSWord<UserAttribute> {
  nsString mId;
  Maybe<nsString> mInfo;
  Maybe<nsString> mMimeType;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mValue;
  UserAttribute() = default;
  UserAttribute(nsString&& aId, Maybe<nsString>&& aInfo, Maybe<nsString>&& aMimeType, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aValue) : mId(std::move(aId)), mInfo(std::move(aInfo)), mMimeType(std::move(aMimeType)), mValue(std::move(aValue)) {}
  UserAttribute(UserAttribute&& aOther) : mId(std::move(aOther.mId)), mInfo(std::move(aOther.mInfo)), mMimeType(std::move(aOther.mMimeType)), mValue(std::move(aOther.mValue))  {}
  
  ~UserAttribute() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv);
  static bool ToJSVal(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_UserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_UserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv);
bool nsTArray_UserAttribute_ToJSVal(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationGetUserAttributesResult = MozPromise<nsTArray<UserAttribute>, Failure, true>;
struct UpdateUserAttributesArgs : IJSWord<UpdateUserAttributesArgs> {
  nsTArray<UserAttribute> mUserAttributes;
  UpdateUserAttributesArgs() = default;
  UpdateUserAttributesArgs(nsTArray<UserAttribute>&& aUserAttributes) : mUserAttributes(std::move(aUserAttributes)) {}
  UpdateUserAttributesArgs(UpdateUserAttributesArgs&& aOther) : mUserAttributes(std::move(aOther.mUserAttributes))  {}
  
  ~UpdateUserAttributesArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateUserAttributesArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const UpdateUserAttributesArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationUpdateUserAttributesResult = MozPromise<void*, Failure, true>;
struct RequestContextWithLoginOperation : IJSWord<RequestContextWithLoginOperation> {
  LoginOperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContextWithLoginOperation() = default;
  RequestContextWithLoginOperation(LoginOperationMetadata&& aOperation, ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mOperation(std::move(aOperation)), mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContextWithLoginOperation(RequestContextWithLoginOperation&& aOther) : mOperation(std::move(aOther.mOperation)), mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  
  ~RequestContextWithLoginOperation() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithLoginOperation& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContextWithLoginOperation& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusFieldCategoryOptions : IJSWord<BerytusFieldCategoryOptions> {
  nsString mCategoryId;
  Maybe<double> mPosition;
  BerytusFieldCategoryOptions() = default;
  BerytusFieldCategoryOptions(nsString&& aCategoryId, Maybe<double>&& aPosition) : mCategoryId(std::move(aCategoryId)), mPosition(std::move(aPosition)) {}
  BerytusFieldCategoryOptions(BerytusFieldCategoryOptions&& aOther) : mCategoryId(std::move(aOther.mCategoryId)), mPosition(std::move(aOther.mPosition))  {}
  
  ~BerytusFieldCategoryOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusFieldCategoryOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusFieldCategoryOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsMaybe_BerytusFieldCategoryOptions_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_BerytusFieldCategoryOptions_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<BerytusFieldCategoryOptions>& aRv);
bool Maybe_BerytusFieldCategoryOptions_ToJSVal(JSContext* aCx, const Maybe<BerytusFieldCategoryOptions>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusForeignIdentityFieldOptions : IJSWord<BerytusForeignIdentityFieldOptions> {
  bool mPrivate;
  nsString mKind;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusForeignIdentityFieldOptions() = default;
  BerytusForeignIdentityFieldOptions(bool&& aPrivate, nsString&& aKind, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mPrivate(std::move(aPrivate)), mKind(std::move(aKind)), mCategory(std::move(aCategory)) {}
  BerytusForeignIdentityFieldOptions(BerytusForeignIdentityFieldOptions&& aOther) : mPrivate(std::move(aOther.mPrivate)), mKind(std::move(aOther.mKind)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusForeignIdentityFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusForeignIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct JSNull {};
bool JSValIsNull(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NullFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, JSNull& aRv);
bool NullToJSVal(JSContext* aCx, const JSNull& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<JSNull, nsString, BerytusEncryptedPacket> : IJSWord<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<JSNull, nsString, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket>* mVariant;
};
struct BerytusForeignIdentityField : IJSWord<BerytusForeignIdentityField> {
  nsString mType;
  BerytusForeignIdentityFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusForeignIdentityField() = default;
  BerytusForeignIdentityField(nsString&& aType, BerytusForeignIdentityFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusForeignIdentityField(BerytusForeignIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusForeignIdentityField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusForeignIdentityField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusIdentityFieldOptions : IJSWord<BerytusIdentityFieldOptions> {
  bool mHumanReadable;
  bool mPrivate;
  double mMaxLength;
  Maybe<nsString> mAllowedCharacters;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusIdentityFieldOptions() = default;
  BerytusIdentityFieldOptions(bool&& aHumanReadable, bool&& aPrivate, double&& aMaxLength, Maybe<nsString>&& aAllowedCharacters, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mHumanReadable(std::move(aHumanReadable)), mPrivate(std::move(aPrivate)), mMaxLength(std::move(aMaxLength)), mAllowedCharacters(std::move(aAllowedCharacters)), mCategory(std::move(aCategory)) {}
  BerytusIdentityFieldOptions(BerytusIdentityFieldOptions&& aOther) : mHumanReadable(std::move(aOther.mHumanReadable)), mPrivate(std::move(aOther.mPrivate)), mMaxLength(std::move(aOther.mMaxLength)), mAllowedCharacters(std::move(aOther.mAllowedCharacters)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusIdentityFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusIdentityField : IJSWord<BerytusIdentityField> {
  nsString mType;
  BerytusIdentityFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusIdentityField() = default;
  BerytusIdentityField(nsString&& aType, BerytusIdentityFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusIdentityField(BerytusIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusIdentityField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusIdentityField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusKeyFieldOptions : IJSWord<BerytusKeyFieldOptions> {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusKeyFieldOptions() = default;
  BerytusKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusKeyFieldOptions(BerytusKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusKeyFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<ArrayBuffer, BerytusEncryptedPacket> : IJSWord<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<ArrayBuffer, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<ArrayBuffer, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<ArrayBuffer, BerytusEncryptedPacket>* mVariant;
};
struct BerytusKeyFieldValue : IJSWord<BerytusKeyFieldValue> {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mPublicKey;
  BerytusKeyFieldValue() = default;
  BerytusKeyFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aPublicKey) : mPublicKey(std::move(aPublicKey)) {}
  BerytusKeyFieldValue(BerytusKeyFieldValue&& aOther) : mPublicKey(std::move(aOther.mPublicKey))  {}
  
  ~BerytusKeyFieldValue() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<JSNull, BerytusKeyFieldValue> : IJSWord<SafeVariant<JSNull, BerytusKeyFieldValue>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<JSNull, BerytusKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<JSNull, BerytusKeyFieldValue>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<JSNull, BerytusKeyFieldValue> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusKeyFieldValue>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<JSNull, BerytusKeyFieldValue>* mVariant;
};
struct BerytusKeyField : IJSWord<BerytusKeyField> {
  nsString mType;
  BerytusKeyFieldOptions mOptions;
  SafeVariant<JSNull, BerytusKeyFieldValue> mValue;
  nsString mId;
  BerytusKeyField() = default;
  BerytusKeyField(nsString&& aType, BerytusKeyFieldOptions&& aOptions, SafeVariant<JSNull, BerytusKeyFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusKeyField(BerytusKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusKeyField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusPasswordFieldOptions : IJSWord<BerytusPasswordFieldOptions> {
  Maybe<nsString> mPasswordRules;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusPasswordFieldOptions() = default;
  BerytusPasswordFieldOptions(Maybe<nsString>&& aPasswordRules, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mPasswordRules(std::move(aPasswordRules)), mCategory(std::move(aCategory)) {}
  BerytusPasswordFieldOptions(BerytusPasswordFieldOptions&& aOther) : mPasswordRules(std::move(aOther.mPasswordRules)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusPasswordFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusPasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusPasswordField : IJSWord<BerytusPasswordField> {
  nsString mType;
  BerytusPasswordFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusPasswordField() = default;
  BerytusPasswordField(nsString&& aType, BerytusPasswordFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusPasswordField(BerytusPasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusPasswordField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusPasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecurePasswordFieldOptions : IJSWord<BerytusSecurePasswordFieldOptions> {
  nsString mIdentityFieldId;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusSecurePasswordFieldOptions() = default;
  BerytusSecurePasswordFieldOptions(nsString&& aIdentityFieldId, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mIdentityFieldId(std::move(aIdentityFieldId)), mCategory(std::move(aCategory)) {}
  BerytusSecurePasswordFieldOptions(BerytusSecurePasswordFieldOptions&& aOther) : mIdentityFieldId(std::move(aOther.mIdentityFieldId)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusSecurePasswordFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecurePasswordFieldValue : IJSWord<BerytusSecurePasswordFieldValue> {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mSalt;
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mVerifier;
  BerytusSecurePasswordFieldValue() = default;
  BerytusSecurePasswordFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aSalt, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aVerifier) : mSalt(std::move(aSalt)), mVerifier(std::move(aVerifier)) {}
  BerytusSecurePasswordFieldValue(BerytusSecurePasswordFieldValue&& aOther) : mSalt(std::move(aOther.mSalt)), mVerifier(std::move(aOther.mVerifier))  {}
  
  ~BerytusSecurePasswordFieldValue() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<JSNull, BerytusSecurePasswordFieldValue> : IJSWord<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<JSNull, BerytusSecurePasswordFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<JSNull, BerytusSecurePasswordFieldValue>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<JSNull, BerytusSecurePasswordFieldValue> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<JSNull, BerytusSecurePasswordFieldValue>* mVariant;
};
struct BerytusSecurePasswordField : IJSWord<BerytusSecurePasswordField> {
  nsString mType;
  BerytusSecurePasswordFieldOptions mOptions;
  SafeVariant<JSNull, BerytusSecurePasswordFieldValue> mValue;
  nsString mId;
  BerytusSecurePasswordField() = default;
  BerytusSecurePasswordField(nsString&& aType, BerytusSecurePasswordFieldOptions&& aOptions, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSecurePasswordField(BerytusSecurePasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusSecurePasswordField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSharedKeyFieldOptions : IJSWord<BerytusSharedKeyFieldOptions> {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusSharedKeyFieldOptions() = default;
  BerytusSharedKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusSharedKeyFieldOptions(BerytusSharedKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory))  {}
  
  ~BerytusSharedKeyFieldOptions() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSharedKeyFieldValue : IJSWord<BerytusSharedKeyFieldValue> {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mPrivateKey;
  BerytusSharedKeyFieldValue() = default;
  BerytusSharedKeyFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aPrivateKey) : mPrivateKey(std::move(aPrivateKey)) {}
  BerytusSharedKeyFieldValue(BerytusSharedKeyFieldValue&& aOther) : mPrivateKey(std::move(aOther.mPrivateKey))  {}
  
  ~BerytusSharedKeyFieldValue() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<JSNull, BerytusSharedKeyFieldValue> : IJSWord<SafeVariant<JSNull, BerytusSharedKeyFieldValue>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<JSNull, BerytusSharedKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<JSNull, BerytusSharedKeyFieldValue>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<JSNull, BerytusSharedKeyFieldValue> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<JSNull, BerytusSharedKeyFieldValue>* mVariant;
};
struct BerytusSharedKeyField : IJSWord<BerytusSharedKeyField> {
  nsString mType;
  BerytusSharedKeyFieldOptions mOptions;
  SafeVariant<JSNull, BerytusSharedKeyFieldValue> mValue;
  nsString mId;
  BerytusSharedKeyField() = default;
  BerytusSharedKeyField(nsString&& aType, BerytusSharedKeyFieldOptions&& aOptions, SafeVariant<JSNull, BerytusSharedKeyFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSharedKeyField(BerytusSharedKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  
  ~BerytusSharedKeyField() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField> : IJSWord<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>* mVariant;
};
struct AddFieldArgs : IJSWord<AddFieldArgs> {
  SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField> mField;
  AddFieldArgs() = default;
  AddFieldArgs(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>&& aField) : mField(std::move(aField)) {}
  AddFieldArgs(AddFieldArgs&& aOther) : mField(std::move(aOther.mField))  {}
  
  ~AddFieldArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> : IJSWord<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>* mVariant;
};
using AccountCreationAddFieldResult = MozPromise<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>, Failure, true>;
struct FieldValueRejectionReason : IJSWord<FieldValueRejectionReason> {
  nsString mCode;
  FieldValueRejectionReason() = default;
  FieldValueRejectionReason(nsString&& aCode) : mCode(std::move(aCode)) {}
  FieldValueRejectionReason(FieldValueRejectionReason&& aOther) : mCode(std::move(aOther.mCode))  {}
  
  ~FieldValueRejectionReason() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing> : IJSWord<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>* mVariant;
};
struct RejectFieldValueArgs : IJSWord<RejectFieldValueArgs> {
  nsString mFieldId;
  FieldValueRejectionReason mReason;
  SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing> mOptionalNewValue;
  RejectFieldValueArgs() = default;
  RejectFieldValueArgs(nsString&& aFieldId, FieldValueRejectionReason&& aReason, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>&& aOptionalNewValue) : mFieldId(std::move(aFieldId)), mReason(std::move(aReason)), mOptionalNewValue(std::move(aOptionalNewValue)) {}
  RejectFieldValueArgs(RejectFieldValueArgs&& aOther) : mFieldId(std::move(aOther.mFieldId)), mReason(std::move(aOther.mReason)), mOptionalNewValue(std::move(aOther.mOptionalNewValue))  {}
  
  ~RejectFieldValueArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationRejectFieldValueResult = MozPromise<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>, Failure, true>;
struct EChallengeType {
  uint8_t mVal;
  EChallengeType() : mVal(0) {}
  EChallengeType(uint8_t aVal) : mVal(aVal) {}
  EChallengeType(EChallengeType&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EChallengeType Identification();
  static EChallengeType DigitalSignature();
  static EChallengeType Password();
  static EChallengeType SecureRemotePassword();
  static EChallengeType ForeignIdentityOtp();
  bool IsIdentification() const;
  bool IsDigitalSignature() const;
  bool IsPassword() const;
  bool IsSecureRemotePassword() const;
  bool IsForeignIdentityOtp() const;
  void SetAsIdentification();
  void SetAsDigitalSignature();
  void SetAsPassword();
  void SetAsSecureRemotePassword();
  void SetAsForeignIdentityOtp();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EChallengeType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeType& aRv);
  static bool ToJSVal(JSContext* aCx, const EChallengeType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct ChallengeParameters : IJSWord<ChallengeParameters> {
  
  ChallengeParameters() = default;
  
  ~ChallengeParameters() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct EChallengeStatus {
  uint8_t mVal;
  EChallengeStatus() : mVal(0) {}
  EChallengeStatus(uint8_t aVal) : mVal(aVal) {}
  EChallengeStatus(EChallengeStatus&& aOther) : mVal(std::move(aOther.mVal)) {}
  static EChallengeStatus Invalid();
  static EChallengeStatus Pending();
  static EChallengeStatus Active();
  static EChallengeStatus Aborted();
  static EChallengeStatus Sealed();
  bool IsInvalid() const;
  bool IsPending() const;
  bool IsActive() const;
  bool IsAborted() const;
  bool IsSealed() const;
  void SetAsInvalid();
  void SetAsPending();
  void SetAsActive();
  void SetAsAborted();
  void SetAsSealed();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EChallengeStatus& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeStatus& aRv);
  static bool ToJSVal(JSContext* aCx, const EChallengeStatus& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct ChallengeMetadata : IJSWord<ChallengeMetadata> {
  nsString mId;
  EChallengeType mType;
  ChallengeParameters mParameters;
  EChallengeStatus mStatus;
  ChallengeMetadata() = default;
  ChallengeMetadata(nsString&& aId, EChallengeType&& aType, ChallengeParameters&& aParameters, EChallengeStatus&& aStatus) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)), mStatus(std::move(aStatus)) {}
  ChallengeMetadata(ChallengeMetadata&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters)), mStatus(std::move(aOther.mStatus))  {}
  
  ~ChallengeMetadata() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveChallengeRequestArgs : IJSWord<ApproveChallengeRequestArgs> {
  ChallengeMetadata mChallenge;
  ApproveChallengeRequestArgs() = default;
  ApproveChallengeRequestArgs(ChallengeMetadata&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  ApproveChallengeRequestArgs(ApproveChallengeRequestArgs&& aOther) : mChallenge(std::move(aOther.mChallenge))  {}
  
  ~ApproveChallengeRequestArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationApproveChallengeRequestResult = MozPromise<void*, Failure, true>;
struct ChallengeAbortionReason : IJSWord<ChallengeAbortionReason> {
  nsString mCode;
  ChallengeAbortionReason() = default;
  ChallengeAbortionReason(nsString&& aCode) : mCode(std::move(aCode)) {}
  ChallengeAbortionReason(ChallengeAbortionReason&& aOther) : mCode(std::move(aOther.mCode))  {}
  
  ~ChallengeAbortionReason() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeAbortionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct AbortChallengeArgs : IJSWord<AbortChallengeArgs> {
  ChallengeMetadata mChallenge;
  ChallengeAbortionReason mReason;
  AbortChallengeArgs() = default;
  AbortChallengeArgs(ChallengeMetadata&& aChallenge, ChallengeAbortionReason&& aReason) : mChallenge(std::move(aChallenge)), mReason(std::move(aReason)) {}
  AbortChallengeArgs(AbortChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)), mReason(std::move(aOther.mReason))  {}
  
  ~AbortChallengeArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationAbortChallengeResult = MozPromise<void*, Failure, true>;
struct CloseChallengeArgs : IJSWord<CloseChallengeArgs> {
  ChallengeMetadata mChallenge;
  CloseChallengeArgs() = default;
  CloseChallengeArgs(ChallengeMetadata&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  CloseChallengeArgs(CloseChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge))  {}
  
  ~CloseChallengeArgs() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationCloseChallengeResult = MozPromise<void*, Failure, true>;
struct BerytusIdentificationChallengeInfo : IJSWord<BerytusIdentificationChallengeInfo> {
  nsString mId;
  nsString mType;
  JSNull mParameters;
  BerytusIdentificationChallengeInfo() = default;
  BerytusIdentificationChallengeInfo(nsString&& aId, nsString&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusIdentificationChallengeInfo(BerytusIdentificationChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  
  ~BerytusIdentificationChallengeInfo() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentificationChallengeInfo& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusIdentificationChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendGetIdentityFieldsMessage : IJSWord<BerytusSendGetIdentityFieldsMessage> {
  BerytusIdentificationChallengeInfo mChallenge;
  nsString mName;
  nsTArray<nsString> mPayload;
  BerytusSendGetIdentityFieldsMessage() = default;
  BerytusSendGetIdentityFieldsMessage(BerytusIdentificationChallengeInfo&& aChallenge, nsString&& aName, nsTArray<nsString>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendGetIdentityFieldsMessage(BerytusSendGetIdentityFieldsMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendGetIdentityFieldsMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetIdentityFieldsMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendGetIdentityFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusPasswordChallengeInfo : IJSWord<BerytusPasswordChallengeInfo> {
  nsString mId;
  nsString mType;
  JSNull mParameters;
  BerytusPasswordChallengeInfo() = default;
  BerytusPasswordChallengeInfo(nsString&& aId, nsString&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusPasswordChallengeInfo(BerytusPasswordChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  
  ~BerytusPasswordChallengeInfo() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordChallengeInfo& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusPasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendGetPasswordFieldsMessage : IJSWord<BerytusSendGetPasswordFieldsMessage> {
  BerytusPasswordChallengeInfo mChallenge;
  nsString mName;
  nsTArray<nsString> mPayload;
  BerytusSendGetPasswordFieldsMessage() = default;
  BerytusSendGetPasswordFieldsMessage(BerytusPasswordChallengeInfo&& aChallenge, nsString&& aName, nsTArray<nsString>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendGetPasswordFieldsMessage(BerytusSendGetPasswordFieldsMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendGetPasswordFieldsMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetPasswordFieldsMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendGetPasswordFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusDigitalSignatureChallengeInfo : IJSWord<BerytusDigitalSignatureChallengeInfo> {
  nsString mId;
  nsString mType;
  JSNull mParameters;
  BerytusDigitalSignatureChallengeInfo() = default;
  BerytusDigitalSignatureChallengeInfo(nsString&& aId, nsString&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusDigitalSignatureChallengeInfo(BerytusDigitalSignatureChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  
  ~BerytusDigitalSignatureChallengeInfo() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusDigitalSignatureChallengeInfo& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusDigitalSignatureChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendSelectKeyMessage : IJSWord<BerytusSendSelectKeyMessage> {
  BerytusDigitalSignatureChallengeInfo mChallenge;
  nsString mName;
  nsString mPayload;
  BerytusSendSelectKeyMessage() = default;
  BerytusSendSelectKeyMessage(BerytusDigitalSignatureChallengeInfo&& aChallenge, nsString&& aName, nsString&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSelectKeyMessage(BerytusSendSelectKeyMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendSelectKeyMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectKeyMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendSelectKeyMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendSignNonceMessage : IJSWord<BerytusSendSignNonceMessage> {
  BerytusDigitalSignatureChallengeInfo mChallenge;
  nsString mName;
  SafeVariant<ArrayBuffer, ArrayBufferView> mPayload;
  BerytusSendSignNonceMessage() = default;
  BerytusSendSignNonceMessage(BerytusDigitalSignatureChallengeInfo&& aChallenge, nsString&& aName, SafeVariant<ArrayBuffer, ArrayBufferView>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSignNonceMessage(BerytusSendSignNonceMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendSignNonceMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSignNonceMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendSignNonceMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecureRemotePasswordChallengeParameters : IJSWord<BerytusSecureRemotePasswordChallengeParameters> {
  nsString mEncoding;
  BerytusSecureRemotePasswordChallengeParameters() = default;
  BerytusSecureRemotePasswordChallengeParameters(nsString&& aEncoding) : mEncoding(std::move(aEncoding)) {}
  BerytusSecureRemotePasswordChallengeParameters(BerytusSecureRemotePasswordChallengeParameters&& aOther) : mEncoding(std::move(aOther.mEncoding))  {}
  
  ~BerytusSecureRemotePasswordChallengeParameters() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecureRemotePasswordChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecureRemotePasswordChallengeInfo : IJSWord<BerytusSecureRemotePasswordChallengeInfo> {
  nsString mId;
  nsString mType;
  BerytusSecureRemotePasswordChallengeParameters mParameters;
  BerytusSecureRemotePasswordChallengeInfo() = default;
  BerytusSecureRemotePasswordChallengeInfo(nsString&& aId, nsString&& aType, BerytusSecureRemotePasswordChallengeParameters&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusSecureRemotePasswordChallengeInfo(BerytusSecureRemotePasswordChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  
  ~BerytusSecureRemotePasswordChallengeInfo() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeInfo& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecureRemotePasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendSelectSecurePasswordMessage : IJSWord<BerytusSendSelectSecurePasswordMessage> {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  nsString mName;
  nsString mPayload;
  BerytusSendSelectSecurePasswordMessage() = default;
  BerytusSendSelectSecurePasswordMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, nsString&& aName, nsString&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSelectSecurePasswordMessage(BerytusSendSelectSecurePasswordMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendSelectSecurePasswordMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectSecurePasswordMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendSelectSecurePasswordMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendExchangePublicKeysMessage : IJSWord<BerytusSendExchangePublicKeysMessage> {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  nsString mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendExchangePublicKeysMessage() = default;
  BerytusSendExchangePublicKeysMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, nsString&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendExchangePublicKeysMessage(BerytusSendExchangePublicKeysMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendExchangePublicKeysMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendExchangePublicKeysMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendExchangePublicKeysMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendComputeClientProofMessage : IJSWord<BerytusSendComputeClientProofMessage> {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  nsString mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendComputeClientProofMessage() = default;
  BerytusSendComputeClientProofMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, nsString&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendComputeClientProofMessage(BerytusSendComputeClientProofMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendComputeClientProofMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendComputeClientProofMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendComputeClientProofMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSendVerifyServerProofMessage : IJSWord<BerytusSendVerifyServerProofMessage> {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  nsString mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendVerifyServerProofMessage() = default;
  BerytusSendVerifyServerProofMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, nsString&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendVerifyServerProofMessage(BerytusSendVerifyServerProofMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  
  ~BerytusSendVerifyServerProofMessage() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendVerifyServerProofMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSendVerifyServerProofMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage> : IJSWord<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>* mVariant;
};
template <typename K, typename V,
          typename = std::enable_if_t<std::is_base_of<IJSWord<K>, K>::value>,
          typename = std::enable_if_t<std::is_base_of<IJSWord<V>, V>::value>>
class RecordWord : public dom::Record<K, V>,
               public IJSWord<RecordWord<K, V>> {
  RecordWord() = default;
  RecordWord(RecordWord<K, V>&& aOther) : dom::Record<K, V>(std::move(aOther)) {}
  ~RecordWord() {}

  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext *aCx, const JS::Handle<JS::Value> aValue, RecordWord<K, V>& aRv);
  static bool ToJSVal(JSContext* aCx, const RecordWord<K, V>& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<RecordWord<nsString, nsString>> : IJSWord<SafeVariant<RecordWord<nsString, nsString>>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<RecordWord<nsString, nsString>>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<RecordWord<nsString, nsString>>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<RecordWord<nsString, nsString>> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<RecordWord<nsString, nsString>>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<RecordWord<nsString, nsString>>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<RecordWord<nsString, nsString>>* mVariant;
};
struct BerytusChallengeGetIdentityFieldsMessageResponse : IJSWord<BerytusChallengeGetIdentityFieldsMessageResponse> {
  SafeVariant<RecordWord<nsString, nsString>> mResponse;
  BerytusChallengeGetIdentityFieldsMessageResponse() = default;
  BerytusChallengeGetIdentityFieldsMessageResponse(SafeVariant<RecordWord<nsString, nsString>>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeGetIdentityFieldsMessageResponse(BerytusChallengeGetIdentityFieldsMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeGetIdentityFieldsMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetIdentityFieldsMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeGetIdentityFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusChallengeGetPasswordFieldsMessageResponse : IJSWord<BerytusChallengeGetPasswordFieldsMessageResponse> {
  SafeVariant<RecordWord<nsString, nsString>> mResponse;
  BerytusChallengeGetPasswordFieldsMessageResponse() = default;
  BerytusChallengeGetPasswordFieldsMessageResponse(SafeVariant<RecordWord<nsString, nsString>>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeGetPasswordFieldsMessageResponse(BerytusChallengeGetPasswordFieldsMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeGetPasswordFieldsMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetPasswordFieldsMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeGetPasswordFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusChallengeSelectKeyMessageResponse : IJSWord<BerytusChallengeSelectKeyMessageResponse> {
  BerytusKeyFieldValue mResponse;
  BerytusChallengeSelectKeyMessageResponse() = default;
  BerytusChallengeSelectKeyMessageResponse(BerytusKeyFieldValue&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSelectKeyMessageResponse(BerytusChallengeSelectKeyMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeSelectKeyMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectKeyMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeSelectKeyMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusChallengeSignNonceMessageResponse : IJSWord<BerytusChallengeSignNonceMessageResponse> {
  ArrayBuffer mResponse;
  BerytusChallengeSignNonceMessageResponse() = default;
  BerytusChallengeSignNonceMessageResponse(ArrayBuffer&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSignNonceMessageResponse(BerytusChallengeSignNonceMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeSignNonceMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSignNonceMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeSignNonceMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<nsString, BerytusEncryptedPacket> : IJSWord<SafeVariant<nsString, BerytusEncryptedPacket>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<nsString, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<nsString, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket>* mVariant;
};
struct BerytusChallengeSelectSecurePasswordMessageResponse : IJSWord<BerytusChallengeSelectSecurePasswordMessageResponse> {
  SafeVariant<nsString, BerytusEncryptedPacket> mResponse;
  BerytusChallengeSelectSecurePasswordMessageResponse() = default;
  BerytusChallengeSelectSecurePasswordMessageResponse(SafeVariant<nsString, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSelectSecurePasswordMessageResponse(BerytusChallengeSelectSecurePasswordMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeSelectSecurePasswordMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectSecurePasswordMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeSelectSecurePasswordMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> : IJSWord<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<nsString, ArrayBuffer, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<nsString, ArrayBuffer, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<nsString, ArrayBuffer, BerytusEncryptedPacket>* mVariant;
};
struct BerytusChallengeExchangePublicKeysMessageResponse : IJSWord<BerytusChallengeExchangePublicKeysMessageResponse> {
  SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> mResponse;
  BerytusChallengeExchangePublicKeysMessageResponse() = default;
  BerytusChallengeExchangePublicKeysMessageResponse(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeExchangePublicKeysMessageResponse(BerytusChallengeExchangePublicKeysMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeExchangePublicKeysMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeExchangePublicKeysMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeExchangePublicKeysMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusChallengeComputeClientProofMessageResponse : IJSWord<BerytusChallengeComputeClientProofMessageResponse> {
  SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> mResponse;
  BerytusChallengeComputeClientProofMessageResponse() = default;
  BerytusChallengeComputeClientProofMessageResponse(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeComputeClientProofMessageResponse(BerytusChallengeComputeClientProofMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeComputeClientProofMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeComputeClientProofMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeComputeClientProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusChallengeVerifyServerProofMessageResponse : IJSWord<BerytusChallengeVerifyServerProofMessageResponse> {
  Nothing mResponse;
  BerytusChallengeVerifyServerProofMessageResponse() = default;
  BerytusChallengeVerifyServerProofMessageResponse(Nothing&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeVerifyServerProofMessageResponse(BerytusChallengeVerifyServerProofMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  ~BerytusChallengeVerifyServerProofMessageResponse() {}
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeVerifyServerProofMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusChallengeVerifyServerProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
template<>
class SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse> : IJSWord<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>> {
public:
  SafeVariant() : mVariant(nullptr) {};
  SafeVariant(SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  };
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse> const* InternalValue() const { return mVariant; }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>& aRv);
  static bool ToJSVal(JSContext* aCx, const SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>& aValue, JS::MutableHandle<JS::Value> aRv);
protected:
  mozilla::Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>* mVariant;
};
using AccountAuthenticationRespondToChallengeMessageResult = MozPromise<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>, Failure, true>;

class AgentProxy final : public nsISupports {
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(AgentProxy)

public:
  AgentProxy(nsIGlobalObject* aGlobal, const nsAString& aManagerId);
  void Disable();
  bool IsDisabled() const;

protected:
  ~AgentProxy();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsString mManagerId;
  bool mDisabled;

  template <typename W1, typename W2,
            typename = std::enable_if_t<std::is_base_of<IJSWord<W1>, W1>::value>,
            typename = std::enable_if_t<std::is_base_of<IJSWord<W2>, W2>::value>>
  already_AddRefed<dom::Promise> CallSendQuery(JSContext *aCx,
                                               const nsAString & aGroup,
                                               const nsAString &aMethod,
                                               const W1& aReqCx,
                                               const W2* aReqArgs,
                                               ErrorResult& aRv) const;
public:
  RefPtr<ManagerGetSigningKeyResult> Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs) const;
  RefPtr<ManagerGetCredentialsMetadataResult> Manager_GetCredentialsMetadata(PreliminaryRequestContext& aContext, GetCredentialsMetadataArgs& aArgs) const;
  RefPtr<ChannelGenerateKeyExchangeParametersResult> Channel_GenerateKeyExchangeParameters(RequestContext& aContext, GenerateKeyExchangeParametersArgs& aArgs) const;
  RefPtr<ChannelEnableEndToEndEncryptionResult> Channel_EnableEndToEndEncryption(RequestContext& aContext, EnableEndToEndEncryptionArgs& aArgs) const;
  RefPtr<ChannelCloseChannelResult> Channel_CloseChannel(RequestContext& aContext) const;
  RefPtr<LoginApproveOperationResult> Login_ApproveOperation(RequestContext& aContext, ApproveOperationArgs& aArgs) const;
  RefPtr<LoginCloseOpeationResult> Login_CloseOpeation(RequestContextWithOperation& aContext) const;
  RefPtr<LoginGetRecordMetadataResult> Login_GetRecordMetadata(RequestContextWithOperation& aContext) const;
  RefPtr<LoginUpdateMetadataResult> Login_UpdateMetadata(RequestContextWithOperation& aContext, UpdateMetadataArgs& aArgs) const;
  RefPtr<AccountCreationApproveTransitionToAuthOpResult> AccountCreation_ApproveTransitionToAuthOp(RequestContextWithOperation& aContext, ApproveTransitionToAuthOpArgs& aArgs) const;
  RefPtr<AccountCreationGetUserAttributesResult> AccountCreation_GetUserAttributes(RequestContextWithOperation& aContext) const;
  RefPtr<AccountCreationUpdateUserAttributesResult> AccountCreation_UpdateUserAttributes(RequestContextWithOperation& aContext, UpdateUserAttributesArgs& aArgs) const;
  RefPtr<AccountCreationAddFieldResult> AccountCreation_AddField(RequestContextWithLoginOperation& aContext, AddFieldArgs& aArgs) const;
  RefPtr<AccountCreationRejectFieldValueResult> AccountCreation_RejectFieldValue(RequestContextWithLoginOperation& aContext, RejectFieldValueArgs& aArgs) const;
  RefPtr<AccountAuthenticationApproveChallengeRequestResult> AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs) const;
  RefPtr<AccountAuthenticationAbortChallengeResult> AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs) const;
  RefPtr<AccountAuthenticationCloseChallengeResult> AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs) const;
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aArgs) const;

};
}  // namespace mozilla::berytus

#endif