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
bool JSValIsNumber(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NumberFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv);
bool NumberToJSVal(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv);
struct DocumentMetadata : IJSWord<DocumentMetadata> {
  double mId;
  
  DocumentMetadata() = default;
  DocumentMetadata(double&& aId) : mId(std::move(aId)) {}
  DocumentMetadata(DocumentMetadata&& aOther) : mId(std::move(aOther.mId)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct PreliminaryRequestContext : IJSWord<PreliminaryRequestContext> {
  DocumentMetadata mDocument;
  
  PreliminaryRequestContext() = default;
  PreliminaryRequestContext(DocumentMetadata&& aDocument) : mDocument(std::move(aDocument)) {}
  PreliminaryRequestContext(PreliminaryRequestContext&& aOther) : mDocument(std::move(aOther.mDocument)) {}
  
  
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
  CryptoActor(CryptoActor&& aOther) : mEd25519Key(std::move(aOther.mEd25519Key)) {}
  
  
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
  UriParams(UriParams&& aOther) : mUri(std::move(aOther.mUri)), mScheme(std::move(aOther.mScheme)), mHostname(std::move(aOther.mHostname)), mPort(std::move(aOther.mPort)), mPath(std::move(aOther.mPath)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv);
  static bool ToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct OriginActor : IJSWord<OriginActor> {
  UriParams mOriginalUri;
  UriParams mCurrentUri;
  
  OriginActor() = default;
  OriginActor(UriParams&& aOriginalUri, UriParams&& aCurrentUri) : mOriginalUri(std::move(aOriginalUri)), mCurrentUri(std::move(aCurrentUri)) {}
  OriginActor(OriginActor&& aOther) : mOriginalUri(std::move(aOther.mOriginalUri)), mCurrentUri(std::move(aOther.mCurrentUri)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv);
  static bool ToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_CryptoActor__OriginActor_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_CryptoActor__OriginActor_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<CryptoActor, OriginActor>** aRv);
bool Variant_CryptoActor__OriginActor_ToJSVal(JSContext* aCx, const Variant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetSigningKeyArgs : IJSWord<GetSigningKeyArgs> {
  
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  GetSigningKeyArgs() = default;
  GetSigningKeyArgs(Variant<CryptoActor, OriginActor>*&& aWebAppActor) : mWebAppActor(std::move(aWebAppActor)) {}
  GetSigningKeyArgs(GetSigningKeyArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor)) {}
  
  
  ~GetSigningKeyArgs() {
    delete mWebAppActor;
  }
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
  PartialAccountIdentity(PartialAccountIdentity&& aOther) : mFieldId(std::move(aOther.mFieldId)), mFieldValue(std::move(aOther.mFieldValue)) {}
  
  
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
  AccountConstraints(AccountConstraints&& aOther) : mCategory(std::move(aOther.mCategory)), mSchemaVersion(std::move(aOther.mSchemaVersion)), mIdentity(std::move(aOther.mIdentity)) {}
  
  
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
  ChannelConstraints(ChannelConstraints&& aOther) : mSecretManagerPublicKey(std::move(aOther.mSecretManagerPublicKey)), mEnableEndToEndEncryption(std::move(aOther.mEnableEndToEndEncryption)), mAccount(std::move(aOther.mAccount)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv);
  static bool ToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GetCredentialsMetadataArgs : IJSWord<GetCredentialsMetadataArgs> {
  ChannelConstraints mChannelConstraints;
  AccountConstraints mAccountConstraints;
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  GetCredentialsMetadataArgs() = default;
  GetCredentialsMetadataArgs(Variant<CryptoActor, OriginActor>*&& aWebAppActor, ChannelConstraints&& aChannelConstraints, AccountConstraints&& aAccountConstraints) : mWebAppActor(std::move(aWebAppActor)), mChannelConstraints(std::move(aChannelConstraints)), mAccountConstraints(std::move(aAccountConstraints)) {}
  GetCredentialsMetadataArgs(GetCredentialsMetadataArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor)), mChannelConstraints(std::move(aOther.mChannelConstraints)), mAccountConstraints(std::move(aOther.mAccountConstraints)) {}
  
  
  ~GetCredentialsMetadataArgs() {
    delete mWebAppActor;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ManagerGetCredentialsMetadataResult = MozPromise<double, Failure, true>;
struct ChannelMetadata : IJSWord<ChannelMetadata> {
  nsString mId;
  ChannelConstraints mConstraints;
  CryptoActor mScmActor;
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  ChannelMetadata() = default;
  ChannelMetadata(nsString&& aId, ChannelConstraints&& aConstraints, Variant<CryptoActor, OriginActor>*&& aWebAppActor, CryptoActor&& aScmActor) : mId(std::move(aId)), mConstraints(std::move(aConstraints)), mWebAppActor(std::move(aWebAppActor)), mScmActor(std::move(aScmActor)) {}
  ChannelMetadata(ChannelMetadata&& aOther) : mId(std::move(aOther.mId)), mConstraints(std::move(aOther.mConstraints)), mWebAppActor(std::move(aOther.mWebAppActor)), mScmActor(std::move(aOther.mScmActor)) {}
  
  
  ~ChannelMetadata() {
    delete mWebAppActor;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RequestContext : IJSWord<RequestContext> {
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  
  RequestContext() = default;
  RequestContext(ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContext(RequestContext&& aOther) : mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct InitialKeyExchangeParametersDraft : IJSWord<InitialKeyExchangeParametersDraft> {
  nsString mChannelId;
  nsString mWebAppX25519Key;
  
  InitialKeyExchangeParametersDraft() = default;
  InitialKeyExchangeParametersDraft(nsString&& aChannelId, nsString&& aWebAppX25519Key) : mChannelId(std::move(aChannelId)), mWebAppX25519Key(std::move(aWebAppX25519Key)) {}
  InitialKeyExchangeParametersDraft(InitialKeyExchangeParametersDraft&& aOther) : mChannelId(std::move(aOther.mChannelId)), mWebAppX25519Key(std::move(aOther.mWebAppX25519Key)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv);
  static bool ToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GenerateKeyExchangeParametersArgs : IJSWord<GenerateKeyExchangeParametersArgs> {
  InitialKeyExchangeParametersDraft mParamsDraft;
  
  GenerateKeyExchangeParametersArgs() = default;
  GenerateKeyExchangeParametersArgs(InitialKeyExchangeParametersDraft&& aParamsDraft) : mParamsDraft(std::move(aParamsDraft)) {}
  GenerateKeyExchangeParametersArgs(GenerateKeyExchangeParametersArgs&& aOther) : mParamsDraft(std::move(aOther.mParamsDraft)) {}
  
  
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
  PartialKeyExchangeParametersFromScm(PartialKeyExchangeParametersFromScm&& aOther) : mScmX25519Key(std::move(aOther.mScmX25519Key)), mHkdfHash(std::move(aOther.mHkdfHash)), mHkdfSalt(std::move(aOther.mHkdfSalt)), mHkdfInfo(std::move(aOther.mHkdfInfo)), mAesKeyLength(std::move(aOther.mAesKeyLength)) {}
  
  
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
  KeyExchangeParameters(KeyExchangeParameters&& aOther) : mPacket(std::move(aOther.mPacket)), mChannelId(std::move(aOther.mChannelId)), mWebAppX25519Key(std::move(aOther.mWebAppX25519Key)), mScmX25519Key(std::move(aOther.mScmX25519Key)), mHkdfHash(std::move(aOther.mHkdfHash)), mHkdfSalt(std::move(aOther.mHkdfSalt)), mHkdfInfo(std::move(aOther.mHkdfInfo)), mAesKeyLength(std::move(aOther.mAesKeyLength)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct EnableEndToEndEncryptionArgs : IJSWord<EnableEndToEndEncryptionArgs> {
  KeyExchangeParameters mParams;
  ArrayBuffer mWebAppPacketSignature;
  
  EnableEndToEndEncryptionArgs() = default;
  EnableEndToEndEncryptionArgs(KeyExchangeParameters&& aParams, ArrayBuffer&& aWebAppPacketSignature) : mParams(std::move(aParams)), mWebAppPacketSignature(std::move(aWebAppPacketSignature)) {}
  EnableEndToEndEncryptionArgs(EnableEndToEndEncryptionArgs&& aOther) : mParams(std::move(aOther.mParams)), mWebAppPacketSignature(std::move(aOther.mWebAppPacketSignature)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ChannelEnableEndToEndEncryptionResult = MozPromise<ArrayBuffer, Failure, true>;
using ChannelCloseChannelResult = MozPromise<void*, Failure, true>;
struct ELoginUserIntent {
  uint8_t mVal;
  ELoginUserIntent() : mVal(0) {}
  ELoginUserIntent(uint8_t aVal) : mVal(aVal) {};
  static ELoginUserIntent PendingDeclaration();
  static ELoginUserIntent Authenticate();
  static ELoginUserIntent Register();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, ELoginUserIntent& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv);
  static bool ToJSVal(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv);
};

bool JSValIsVariant_nsString_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString>** aRv);
bool Variant_nsString_ToJSVal(JSContext* aCx, const Variant<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestedUserAttribute : IJSWord<RequestedUserAttribute> {
  bool mRequired;
  Variant<nsString>* mId = nullptr;
  RequestedUserAttribute() = default;
  RequestedUserAttribute(Variant<nsString>*&& aId, bool&& aRequired) : mId(std::move(aId)), mRequired(std::move(aRequired)) {}
  RequestedUserAttribute(RequestedUserAttribute&& aOther) : mId(std::move(aOther.mId)), mRequired(std::move(aOther.mRequired)) {}
  
  
  ~RequestedUserAttribute() {
    delete mId;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_RequestedUserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_RequestedUserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<RequestedUserAttribute>& aRv);
bool nsTArray_RequestedUserAttribute_ToJSVal(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EOperationType {
  uint8_t mVal;
  EOperationType() : mVal(0) {}
  EOperationType(uint8_t aVal) : mVal(aVal) {};
  static EOperationType PendingDeclaration();
  static EOperationType Registration();
  static EOperationType Authentication();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EOperationType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationType& aRv);
  static bool ToJSVal(JSContext* aCx, const EOperationType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct EOperationStatus {
  uint8_t mVal;
  EOperationStatus() : mVal(0) {}
  EOperationStatus(uint8_t aVal) : mVal(aVal) {};
  static EOperationStatus Pending();
  static EOperationStatus Created();
  static EOperationStatus Aborted();
  static EOperationStatus Finished();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EOperationStatus& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv);
  static bool ToJSVal(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct OperationState : IJSWord<OperationState> {
  
  
  OperationState() = default;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv);
  static bool ToJSVal(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct LoginOperationMetadata : IJSWord<LoginOperationMetadata> {
  ELoginUserIntent mIntent;
  nsTArray<RequestedUserAttribute> mRequestedUserAttributes;
  nsString mId;
  EOperationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  
  LoginOperationMetadata() = default;
  LoginOperationMetadata(ELoginUserIntent&& aIntent, nsTArray<RequestedUserAttribute>&& aRequestedUserAttributes, nsString&& aId, EOperationType&& aType, EOperationStatus&& aStatus, OperationState&& aState) : mIntent(std::move(aIntent)), mRequestedUserAttributes(std::move(aRequestedUserAttributes)), mId(std::move(aId)), mType(std::move(aType)), mStatus(std::move(aStatus)), mState(std::move(aState)) {}
  LoginOperationMetadata(LoginOperationMetadata&& aOther) : mIntent(std::move(aOther.mIntent)), mRequestedUserAttributes(std::move(aOther.mRequestedUserAttributes)), mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveOperationArgs : IJSWord<ApproveOperationArgs> {
  LoginOperationMetadata mOperation;
  
  ApproveOperationArgs() = default;
  ApproveOperationArgs(LoginOperationMetadata&& aOperation) : mOperation(std::move(aOperation)) {}
  ApproveOperationArgs(ApproveOperationArgs&& aOther) : mOperation(std::move(aOther.mOperation)) {}
  
  
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
  OperationMetadata(OperationMetadata&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState)) {}
  
  
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
  RequestContextWithOperation(RequestContextWithOperation&& aOther) : mOperation(std::move(aOther.mOperation)), mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginCloseOpeationResult = MozPromise<void*, Failure, true>;
struct EMetadataStatus {
  uint8_t mVal;
  EMetadataStatus() : mVal(0) {}
  EMetadataStatus(uint8_t aVal) : mVal(aVal) {};
  static EMetadataStatus Pending();
  static EMetadataStatus Created();
  static EMetadataStatus Retired();
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
  RecordMetadata(RecordMetadata&& aOther) : mVersion(std::move(aOther.mVersion)), mStatus(std::move(aOther.mStatus)), mCategory(std::move(aOther.mCategory)), mChangePassUrl(std::move(aOther.mChangePassUrl)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginGetRecordMetadataResult = MozPromise<RecordMetadata, Failure, true>;
struct UpdateMetadataArgs : IJSWord<UpdateMetadataArgs> {
  RecordMetadata mMetadata;
  
  UpdateMetadataArgs() = default;
  UpdateMetadataArgs(RecordMetadata&& aMetadata) : mMetadata(std::move(aMetadata)) {}
  UpdateMetadataArgs(UpdateMetadataArgs&& aOther) : mMetadata(std::move(aOther.mMetadata)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginUpdateMetadataResult = MozPromise<void*, Failure, true>;
struct ApproveTransitionToAuthOpArgs : IJSWord<ApproveTransitionToAuthOpArgs> {
  LoginOperationMetadata mNewAuthOp;
  
  ApproveTransitionToAuthOpArgs() = default;
  ApproveTransitionToAuthOpArgs(LoginOperationMetadata&& aNewAuthOp) : mNewAuthOp(std::move(aNewAuthOp)) {}
  ApproveTransitionToAuthOpArgs(ApproveTransitionToAuthOpArgs&& aOther) : mNewAuthOp(std::move(aOther.mNewAuthOp)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationApproveTransitionToAuthOpResult = MozPromise<void*, Failure, true>;
struct UserAttribute : IJSWord<UserAttribute> {
  nsString mMimeType;
  nsString mValue;
  Variant<nsString>* mId = nullptr;
  UserAttribute() = default;
  UserAttribute(Variant<nsString>*&& aId, nsString&& aMimeType, nsString&& aValue) : mId(std::move(aId)), mMimeType(std::move(aMimeType)), mValue(std::move(aValue)) {}
  UserAttribute(UserAttribute&& aOther) : mId(std::move(aOther.mId)), mMimeType(std::move(aOther.mMimeType)), mValue(std::move(aOther.mValue)) {}
  
  
  ~UserAttribute() {
    delete mId;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv);
  static bool ToJSVal(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsnsTArray_UserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_UserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv);
bool nsTArray_UserAttribute_ToJSVal(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationGetUserAttributesResult = MozPromise<nsTArray<UserAttribute>, Failure, true>;
struct BerytusFieldCategoryOptions : IJSWord<BerytusFieldCategoryOptions> {
  nsString mCategoryId;
  Maybe<double> mPosition;
  
  BerytusFieldCategoryOptions() = default;
  BerytusFieldCategoryOptions(nsString&& aCategoryId, Maybe<double>&& aPosition) : mCategoryId(std::move(aCategoryId)), mPosition(std::move(aPosition)) {}
  BerytusFieldCategoryOptions(BerytusFieldCategoryOptions&& aOther) : mCategoryId(std::move(aOther.mCategoryId)), mPosition(std::move(aOther.mPosition)) {}
  
  
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
  BerytusForeignIdentityFieldOptions(BerytusForeignIdentityFieldOptions&& aOther) : mPrivate(std::move(aOther.mPrivate)), mKind(std::move(aOther.mKind)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusForeignIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsNull(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NullFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
bool NullToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsArrayBufferView(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ArrayBufferViewFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBufferView& aRv);
bool ArrayBufferViewToJSVal(JSContext* aCx, const ArrayBufferView& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_ArrayBuffer__ArrayBufferView_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_ArrayBuffer__ArrayBufferView_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<ArrayBuffer, ArrayBufferView>** aRv);
bool Variant_ArrayBuffer__ArrayBufferView_ToJSVal(JSContext* aCx, const Variant<ArrayBuffer, ArrayBufferView>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsNothing(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NothingFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
bool NothingToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_ArrayBuffer__ArrayBufferView__Nothing_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_ArrayBuffer__ArrayBufferView__Nothing_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<ArrayBuffer, ArrayBufferView, Nothing>** aRv);
bool Variant_ArrayBuffer__ArrayBufferView__Nothing_ToJSVal(JSContext* aCx, const Variant<ArrayBuffer, ArrayBufferView, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AesGcmParams : IJSWord<AesGcmParams> {
  nsString mName;
  ArrayBuffer mIv;
  Maybe<ArrayBuffer> mAdditionalData;
  double mTagLength;
  
  AesGcmParams() = default;
  AesGcmParams(nsString&& aName, ArrayBuffer&& aIv, Maybe<ArrayBuffer>&& aAdditionalData, double&& aTagLength) : mName(std::move(aName)), mIv(std::move(aIv)), mAdditionalData(std::move(aAdditionalData)), mTagLength(std::move(aTagLength)) {}
  AesGcmParams(AesGcmParams&& aOther) : mName(std::move(aOther.mName)), mIv(std::move(aOther.mIv)), mAdditionalData(std::move(aOther.mAdditionalData)), mTagLength(std::move(aOther.mTagLength)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AesGcmParams& aRv);
  static bool ToJSVal(JSContext* aCx, const AesGcmParams& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusEncryptedPacket : IJSWord<BerytusEncryptedPacket> {
  AesGcmParams mParameters;
  ArrayBuffer mCiphertext;
  
  BerytusEncryptedPacket() = default;
  BerytusEncryptedPacket(AesGcmParams&& aParameters, ArrayBuffer&& aCiphertext) : mParameters(std::move(aParameters)), mCiphertext(std::move(aCiphertext)) {}
  BerytusEncryptedPacket(BerytusEncryptedPacket&& aOther) : mParameters(std::move(aOther.mParameters)), mCiphertext(std::move(aOther.mCiphertext)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusEncryptedPacket& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusEncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_Nothing__nsString__BerytusEncryptedPacket_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_Nothing__nsString__BerytusEncryptedPacket_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<Nothing, nsString, BerytusEncryptedPacket>** aRv);
bool Variant_Nothing__nsString__BerytusEncryptedPacket_ToJSVal(JSContext* aCx, const Variant<Nothing, nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusForeignIdentityField : IJSWord<BerytusForeignIdentityField> {
  nsString mType;
  BerytusForeignIdentityFieldOptions mOptions;
  nsString mId;
  Variant<Nothing, nsString, BerytusEncryptedPacket>* mValue = nullptr;
  BerytusForeignIdentityField() = default;
  BerytusForeignIdentityField(nsString&& aType, BerytusForeignIdentityFieldOptions&& aOptions, Variant<Nothing, nsString, BerytusEncryptedPacket>*&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusForeignIdentityField(BerytusForeignIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  ~BerytusForeignIdentityField() {
    delete mValue;
  }
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
  BerytusIdentityFieldOptions(BerytusIdentityFieldOptions&& aOther) : mHumanReadable(std::move(aOther.mHumanReadable)), mPrivate(std::move(aOther.mPrivate)), mMaxLength(std::move(aOther.mMaxLength)), mAllowedCharacters(std::move(aOther.mAllowedCharacters)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusIdentityField : IJSWord<BerytusIdentityField> {
  nsString mType;
  BerytusIdentityFieldOptions mOptions;
  nsString mId;
  Variant<Nothing, nsString, BerytusEncryptedPacket>* mValue = nullptr;
  BerytusIdentityField() = default;
  BerytusIdentityField(nsString&& aType, BerytusIdentityFieldOptions&& aOptions, Variant<Nothing, nsString, BerytusEncryptedPacket>*&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusIdentityField(BerytusIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  ~BerytusIdentityField() {
    delete mValue;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusIdentityField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusKeyFieldOptions : IJSWord<BerytusKeyFieldOptions> {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  
  BerytusKeyFieldOptions() = default;
  BerytusKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusKeyFieldOptions(BerytusKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_ArrayBuffer__BerytusEncryptedPacket_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_ArrayBuffer__BerytusEncryptedPacket_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<ArrayBuffer, BerytusEncryptedPacket>** aRv);
bool Variant_ArrayBuffer__BerytusEncryptedPacket_ToJSVal(JSContext* aCx, const Variant<ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusKeyFieldValue : IJSWord<BerytusKeyFieldValue> {
  
  Variant<ArrayBuffer, BerytusEncryptedPacket>* mPublicKey = nullptr;
  BerytusKeyFieldValue() = default;
  BerytusKeyFieldValue(Variant<ArrayBuffer, BerytusEncryptedPacket>*&& aPublicKey) : mPublicKey(std::move(aPublicKey)) {}
  BerytusKeyFieldValue(BerytusKeyFieldValue&& aOther) : mPublicKey(std::move(aOther.mPublicKey)) {}
  
  
  ~BerytusKeyFieldValue() {
    delete mPublicKey;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusKeyField : IJSWord<BerytusKeyField> {
  nsString mType;
  BerytusKeyFieldOptions mOptions;
  BerytusKeyFieldValue mValue;
  nsString mId;
  
  BerytusKeyField() = default;
  BerytusKeyField(nsString&& aType, BerytusKeyFieldOptions&& aOptions, BerytusKeyFieldValue&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusKeyField(BerytusKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusPasswordFieldOptions : IJSWord<BerytusPasswordFieldOptions> {
  Maybe<nsString> mPasswordRules;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  
  BerytusPasswordFieldOptions() = default;
  BerytusPasswordFieldOptions(Maybe<nsString>&& aPasswordRules, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mPasswordRules(std::move(aPasswordRules)), mCategory(std::move(aCategory)) {}
  BerytusPasswordFieldOptions(BerytusPasswordFieldOptions&& aOther) : mPasswordRules(std::move(aOther.mPasswordRules)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusPasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusPasswordField : IJSWord<BerytusPasswordField> {
  nsString mType;
  BerytusPasswordFieldOptions mOptions;
  nsString mId;
  Variant<Nothing, nsString, BerytusEncryptedPacket>* mValue = nullptr;
  BerytusPasswordField() = default;
  BerytusPasswordField(nsString&& aType, BerytusPasswordFieldOptions&& aOptions, Variant<Nothing, nsString, BerytusEncryptedPacket>*&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusPasswordField(BerytusPasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  ~BerytusPasswordField() {
    delete mValue;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusPasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecurePasswordFieldOptions : IJSWord<BerytusSecurePasswordFieldOptions> {
  nsString mIdentityFieldId;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  
  BerytusSecurePasswordFieldOptions() = default;
  BerytusSecurePasswordFieldOptions(nsString&& aIdentityFieldId, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mIdentityFieldId(std::move(aIdentityFieldId)), mCategory(std::move(aCategory)) {}
  BerytusSecurePasswordFieldOptions(BerytusSecurePasswordFieldOptions&& aOther) : mIdentityFieldId(std::move(aOther.mIdentityFieldId)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecurePasswordFieldValue : IJSWord<BerytusSecurePasswordFieldValue> {
  
  Variant<ArrayBuffer, BerytusEncryptedPacket>* mSalt = nullptr;
  Variant<ArrayBuffer, BerytusEncryptedPacket>* mVerifier = nullptr;
  BerytusSecurePasswordFieldValue() = default;
  BerytusSecurePasswordFieldValue(Variant<ArrayBuffer, BerytusEncryptedPacket>*&& aSalt, Variant<ArrayBuffer, BerytusEncryptedPacket>*&& aVerifier) : mSalt(std::move(aSalt)), mVerifier(std::move(aVerifier)) {}
  BerytusSecurePasswordFieldValue(BerytusSecurePasswordFieldValue&& aOther) : mSalt(std::move(aOther.mSalt)), mVerifier(std::move(aOther.mVerifier)) {}
  
  
  ~BerytusSecurePasswordFieldValue() {
    delete mSalt;
delete mVerifier;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSecurePasswordField : IJSWord<BerytusSecurePasswordField> {
  nsString mType;
  BerytusSecurePasswordFieldOptions mOptions;
  BerytusSecurePasswordFieldValue mValue;
  nsString mId;
  
  BerytusSecurePasswordField() = default;
  BerytusSecurePasswordField(nsString&& aType, BerytusSecurePasswordFieldOptions&& aOptions, BerytusSecurePasswordFieldValue&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSecurePasswordField(BerytusSecurePasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSecurePasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSharedKeyFieldOptions : IJSWord<BerytusSharedKeyFieldOptions> {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  
  BerytusSharedKeyFieldOptions() = default;
  BerytusSharedKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusSharedKeyFieldOptions(BerytusSharedKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldOptions& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSharedKeyFieldValue : IJSWord<BerytusSharedKeyFieldValue> {
  
  Variant<ArrayBuffer, BerytusEncryptedPacket>* mPrivateKey = nullptr;
  BerytusSharedKeyFieldValue() = default;
  BerytusSharedKeyFieldValue(Variant<ArrayBuffer, BerytusEncryptedPacket>*&& aPrivateKey) : mPrivateKey(std::move(aPrivateKey)) {}
  BerytusSharedKeyFieldValue(BerytusSharedKeyFieldValue&& aOther) : mPrivateKey(std::move(aOther.mPrivateKey)) {}
  
  
  ~BerytusSharedKeyFieldValue() {
    delete mPrivateKey;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldValue& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct BerytusSharedKeyField : IJSWord<BerytusSharedKeyField> {
  nsString mType;
  BerytusSharedKeyFieldOptions mOptions;
  BerytusSharedKeyFieldValue mValue;
  nsString mId;
  
  BerytusSharedKeyField() = default;
  BerytusSharedKeyField(nsString&& aType, BerytusSharedKeyFieldOptions&& aOptions, BerytusSharedKeyFieldValue&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSharedKeyField(BerytusSharedKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyField& aRv);
  static bool ToJSVal(JSContext* aCx, const BerytusSharedKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_BerytusForeignIdentityField__BerytusIdentityField__BerytusKeyField__BerytusPasswordField__BerytusSecurePasswordField__BerytusSharedKeyField_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_BerytusForeignIdentityField__BerytusIdentityField__BerytusKeyField__BerytusPasswordField__BerytusSecurePasswordField__BerytusSharedKeyField_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>** aRv);
bool Variant_BerytusForeignIdentityField__BerytusIdentityField__BerytusKeyField__BerytusPasswordField__BerytusSecurePasswordField__BerytusSharedKeyField_ToJSVal(JSContext* aCx, const Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AddFieldArgs : IJSWord<AddFieldArgs> {
  
  Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>* mField = nullptr;
  AddFieldArgs() = default;
  AddFieldArgs(Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>*&& aField) : mField(std::move(aField)) {}
  AddFieldArgs(AddFieldArgs&& aOther) : mField(std::move(aOther.mField)) {}
  
  
  ~AddFieldArgs() {
    delete mField;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationAddFieldResult = MozPromise<Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>*, Failure, true>;
struct EFieldType {
  uint8_t mVal;
  EFieldType() : mVal(0) {}
  EFieldType(uint8_t aVal) : mVal(aVal) {};
  static EFieldType Identity();
  static EFieldType ForeignIdentity();
  static EFieldType Password();
  static EFieldType SecurePassword();
  static EFieldType Key();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EFieldType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EFieldType& aRv);
  static bool ToJSVal(JSContext* aCx, const EFieldType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct BaseFieldMetadata : IJSWord<BaseFieldMetadata> {
  EFieldType mFieldType;
  nsString mFieldId;
  Maybe<nsString> mDescription;
  
  BaseFieldMetadata() = default;
  BaseFieldMetadata(EFieldType&& aFieldType, nsString&& aFieldId, Maybe<nsString>&& aDescription) : mFieldType(std::move(aFieldType)), mFieldId(std::move(aFieldId)), mDescription(std::move(aDescription)) {}
  BaseFieldMetadata(BaseFieldMetadata&& aOther) : mFieldType(std::move(aOther.mFieldType)), mFieldId(std::move(aOther.mFieldId)), mDescription(std::move(aOther.mDescription)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BaseFieldMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const BaseFieldMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct FieldValueRejectionReason : IJSWord<FieldValueRejectionReason> {
  nsString mCode;
  
  FieldValueRejectionReason() = default;
  FieldValueRejectionReason(nsString&& aCode) : mCode(std::move(aCode)) {}
  FieldValueRejectionReason(FieldValueRejectionReason&& aOther) : mCode(std::move(aOther.mCode)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsMaybe_ArrayBuffer_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_ArrayBuffer_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<ArrayBuffer>& aRv);
bool Maybe_ArrayBuffer_ToJSVal(JSContext* aCx, const Maybe<ArrayBuffer>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EncryptedPacket : IJSWord<EncryptedPacket> {
  AesGcmParams mParameters;
  ArrayBuffer mCiphertext;
  
  EncryptedPacket() = default;
  EncryptedPacket(AesGcmParams&& aParameters, ArrayBuffer&& aCiphertext) : mParameters(std::move(aParameters)), mCiphertext(std::move(aCiphertext)) {}
  EncryptedPacket(EncryptedPacket&& aOther) : mParameters(std::move(aOther.mParameters)), mCiphertext(std::move(aOther.mCiphertext)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EncryptedPacket& aRv);
  static bool ToJSVal(JSContext* aCx, const EncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_nsString__ArrayBuffer__EncryptedPacket__Nothing_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer__EncryptedPacket__Nothing_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer, EncryptedPacket, Nothing>** aRv);
bool Variant_nsString__ArrayBuffer__EncryptedPacket__Nothing_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer, EncryptedPacket, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct RejectFieldValueArgs : IJSWord<RejectFieldValueArgs> {
  BaseFieldMetadata mField;
  FieldValueRejectionReason mReason;
  Variant<nsString, ArrayBuffer, EncryptedPacket, Nothing>* mOptionalNewValue = nullptr;
  RejectFieldValueArgs() = default;
  RejectFieldValueArgs(BaseFieldMetadata&& aField, FieldValueRejectionReason&& aReason, Variant<nsString, ArrayBuffer, EncryptedPacket, Nothing>*&& aOptionalNewValue) : mField(std::move(aField)), mReason(std::move(aReason)), mOptionalNewValue(std::move(aOptionalNewValue)) {}
  RejectFieldValueArgs(RejectFieldValueArgs&& aOther) : mField(std::move(aOther.mField)), mReason(std::move(aOther.mReason)), mOptionalNewValue(std::move(aOther.mOptionalNewValue)) {}
  
  
  ~RejectFieldValueArgs() {
    delete mOptionalNewValue;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_nsString__ArrayBuffer__EncryptedPacket_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer__EncryptedPacket_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer, EncryptedPacket>** aRv);
bool Variant_nsString__ArrayBuffer__EncryptedPacket_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer, EncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationRejectFieldValueResult = MozPromise<Variant<nsString, ArrayBuffer, EncryptedPacket>*, Failure, true>;
struct EChallengeType {
  uint8_t mVal;
  EChallengeType() : mVal(0) {}
  EChallengeType(uint8_t aVal) : mVal(aVal) {};
  static EChallengeType Identification();
  static EChallengeType DigitalSignature();
  static EChallengeType Password();
  static EChallengeType SecureRemotePassword();
  static EChallengeType ForeignIdentityOtp();
  void ToString(nsString& aRetVal) const;
  static bool FromString(const nsString& aVal, EChallengeType& aRetVal);
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeType& aRv);
  static bool ToJSVal(JSContext* aCx, const EChallengeType& aValue, JS::MutableHandle<JS::Value> aRv);
};

struct ChallengeParameters : IJSWord<ChallengeParameters> {
  
  
  ChallengeParameters() = default;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct EChallengeStatus {
  uint8_t mVal;
  EChallengeStatus() : mVal(0) {}
  EChallengeStatus(uint8_t aVal) : mVal(aVal) {};
  static EChallengeStatus Invalid();
  static EChallengeStatus Pending();
  static EChallengeStatus Active();
  static EChallengeStatus Aborted();
  static EChallengeStatus Sealed();
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
  ChallengeMetadata(ChallengeMetadata&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters)), mStatus(std::move(aOther.mStatus)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveChallengeRequestArgs : IJSWord<ApproveChallengeRequestArgs> {
  ChallengeMetadata mChallenge;
  
  ApproveChallengeRequestArgs() = default;
  ApproveChallengeRequestArgs(ChallengeMetadata&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  ApproveChallengeRequestArgs(ApproveChallengeRequestArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationApproveChallengeRequestResult = MozPromise<void*, Failure, true>;
struct ChallengeAbortionReason : IJSWord<ChallengeAbortionReason> {
  nsString mCode;
  
  ChallengeAbortionReason() = default;
  ChallengeAbortionReason(nsString&& aCode) : mCode(std::move(aCode)) {}
  ChallengeAbortionReason(ChallengeAbortionReason&& aOther) : mCode(std::move(aOther.mCode)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeAbortionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct AbortChallengeArgs : IJSWord<AbortChallengeArgs> {
  ChallengeMetadata mChallenge;
  ChallengeAbortionReason mReason;
  
  AbortChallengeArgs() = default;
  AbortChallengeArgs(ChallengeMetadata&& aChallenge, ChallengeAbortionReason&& aReason) : mChallenge(std::move(aChallenge)), mReason(std::move(aReason)) {}
  AbortChallengeArgs(AbortChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)), mReason(std::move(aOther.mReason)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationAbortChallengeResult = MozPromise<void*, Failure, true>;
struct CloseChallengeArgs : IJSWord<CloseChallengeArgs> {
  ChallengeMetadata mChallenge;
  
  CloseChallengeArgs() = default;
  CloseChallengeArgs(ChallengeMetadata&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  CloseChallengeArgs(CloseChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationCloseChallengeResult = MozPromise<void*, Failure, true>;
struct ChallengePayload : IJSWord<ChallengePayload> {
  
  
  ChallengePayload() = default;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengePayload& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengePayload& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ChallengeMessage : IJSWord<ChallengeMessage> {
  nsString mName;
  ChallengePayload mPayload;
  
  ChallengeMessage() = default;
  ChallengeMessage(nsString&& aName, ChallengePayload&& aPayload) : mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  ChallengeMessage(ChallengeMessage&& aOther) : mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RespondToChallengeMessageArgs : IJSWord<RespondToChallengeMessageArgs> {
  ChallengeMetadata mChallenge;
  ChallengeMessage mChallengeMessage;
  
  RespondToChallengeMessageArgs() = default;
  RespondToChallengeMessageArgs(ChallengeMetadata&& aChallenge, ChallengeMessage&& aChallengeMessage) : mChallenge(std::move(aChallenge)), mChallengeMessage(std::move(aChallengeMessage)) {}
  RespondToChallengeMessageArgs(RespondToChallengeMessageArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)), mChallengeMessage(std::move(aOther.mChallengeMessage)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RespondToChallengeMessageArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const RespondToChallengeMessageArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ChallengeMessageResponse : IJSWord<ChallengeMessageResponse> {
  ChallengePayload mPayload;
  
  ChallengeMessageResponse() = default;
  ChallengeMessageResponse(ChallengePayload&& aPayload) : mPayload(std::move(aPayload)) {}
  ChallengeMessageResponse(ChallengeMessageResponse&& aOther) : mPayload(std::move(aOther.mPayload)) {}
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessageResponse& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationRespondToChallengeMessageResult = MozPromise<ChallengeMessageResponse, Failure, true>;

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
  RefPtr<AccountCreationAddFieldResult> AccountCreation_AddField(RequestContextWithOperation& aContext, AddFieldArgs& aArgs) const;
  RefPtr<AccountCreationRejectFieldValueResult> AccountCreation_RejectFieldValue(RequestContextWithOperation& aContext, RejectFieldValueArgs& aArgs) const;
  RefPtr<AccountAuthenticationApproveChallengeRequestResult> AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs) const;
  RefPtr<AccountAuthenticationAbortChallengeResult> AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs) const;
  RefPtr<AccountAuthenticationCloseChallengeResult> AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs) const;
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, RespondToChallengeMessageArgs& aArgs) const;

};
}  // namespace mozilla::berytus

#endif