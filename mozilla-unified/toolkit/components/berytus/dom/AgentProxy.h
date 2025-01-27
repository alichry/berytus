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
#include "mozilla/dom/Record.h"
#include "mozilla/dom/PromiseNativeHandler.h"

using mozilla::LogLevel;

namespace mozilla::berytus {

using ArrayBuffer = mozilla::dom::ArrayBuffer;
using ArrayBufferView = mozilla::dom::ArrayBufferView;
template <typename K, typename V>
using Record = mozilla::dom::Record<K, V>;

template <typename... T>
class SafeVariant {
public:
  SafeVariant() = delete;
};

class StaticStringBase {
public:
  virtual const nsLiteralString& GetString() const = 0;
  operator nsLiteralString const&() const {
    return GetString();
  }
protected:
  virtual ~StaticStringBase() {}
};

template <typename T>
bool JSValIs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  static_assert(false, "No JSValIs specialisation was found!");
  return false;
}

template <typename T>
bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, T& aRv) {
  static_assert(false, "No FromJSVal specialisation was found!");
  return false;
}

template <typename T>
bool ToJSVal(JSContext* aCx, const T& aValue, JS::MutableHandle<JS::Value> aRv) {
  static_assert(false, "No ToJSVal specialisation was found!");
  return false;
}

template<>
bool JSValIs<double>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<double>(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv);
template<>
bool ToJSVal<double>(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<nsString>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<nsString>(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv);
template<>
bool ToJSVal<nsString>(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv);
struct UriParams {
  nsString mUri;
  nsString mScheme;
  nsString mHostname;
  double mPort;
  nsString mPath;
  UriParams() = default;
  UriParams(nsString&& aUri, nsString&& aScheme, nsString&& aHostname, double&& aPort, nsString&& aPath) : mUri(std::move(aUri)), mScheme(std::move(aScheme)), mHostname(std::move(aHostname)), mPort(std::move(aPort)), mPath(std::move(aPath)) {}
  UriParams(UriParams&& aOther) : mUri(std::move(aOther.mUri)), mScheme(std::move(aOther.mScheme)), mHostname(std::move(aOther.mHostname)), mPort(std::move(aOther.mPort)), mPath(std::move(aOther.mPath))  {}
  UriParams& operator=(UriParams&& aOther) {
    mUri = std::move(aOther.mUri);
  mScheme = std::move(aOther.mScheme);
  mHostname = std::move(aOther.mHostname);
  mPort = std::move(aOther.mPort);
  mPath = std::move(aOther.mPath);
    return *this;
  }
  
  ~UriParams() {}
};
template<>
bool JSValIs<UriParams>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<UriParams>(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv);
template<>
bool ToJSVal<UriParams>(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv);
struct DocumentMetadata {
  double mId;
  UriParams mUri;
  DocumentMetadata() = default;
  DocumentMetadata(double&& aId, UriParams&& aUri) : mId(std::move(aId)), mUri(std::move(aUri)) {}
  DocumentMetadata(DocumentMetadata&& aOther) : mId(std::move(aOther.mId)), mUri(std::move(aOther.mUri))  {}
  DocumentMetadata& operator=(DocumentMetadata&& aOther) {
    mId = std::move(aOther.mId);
  mUri = std::move(aOther.mUri);
    return *this;
  }
  
  ~DocumentMetadata() {}
};
template<>
bool JSValIs<DocumentMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<DocumentMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv);
template<>
bool ToJSVal<DocumentMetadata>(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct PreliminaryRequestContext {
  DocumentMetadata mDocument;
  PreliminaryRequestContext() = default;
  PreliminaryRequestContext(DocumentMetadata&& aDocument) : mDocument(std::move(aDocument)) {}
  PreliminaryRequestContext(PreliminaryRequestContext&& aOther) : mDocument(std::move(aOther.mDocument))  {}
  PreliminaryRequestContext& operator=(PreliminaryRequestContext&& aOther) {
    mDocument = std::move(aOther.mDocument);
    return *this;
  }
  
  ~PreliminaryRequestContext() {}
};
template<>
bool JSValIs<PreliminaryRequestContext>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<PreliminaryRequestContext>(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv);
template<>
bool ToJSVal<PreliminaryRequestContext>(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
struct CryptoActor {
  nsString mEd25519Key;
  CryptoActor() = default;
  CryptoActor(nsString&& aEd25519Key) : mEd25519Key(std::move(aEd25519Key)) {}
  CryptoActor(CryptoActor&& aOther) : mEd25519Key(std::move(aOther.mEd25519Key))  {}
  CryptoActor& operator=(CryptoActor&& aOther) {
    mEd25519Key = std::move(aOther.mEd25519Key);
    return *this;
  }
  
  ~CryptoActor() {}
};
template<>
bool JSValIs<CryptoActor>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<CryptoActor>(JSContext* aCx, JS::Handle<JS::Value> aValue, CryptoActor& aRv);
template<>
bool ToJSVal<CryptoActor>(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv);
struct OriginActor {
  UriParams mOriginalUri;
  UriParams mCurrentUri;
  OriginActor() = default;
  OriginActor(UriParams&& aOriginalUri, UriParams&& aCurrentUri) : mOriginalUri(std::move(aOriginalUri)), mCurrentUri(std::move(aCurrentUri)) {}
  OriginActor(OriginActor&& aOther) : mOriginalUri(std::move(aOther.mOriginalUri)), mCurrentUri(std::move(aOther.mCurrentUri))  {}
  OriginActor& operator=(OriginActor&& aOther) {
    mOriginalUri = std::move(aOther.mOriginalUri);
  mCurrentUri = std::move(aOther.mCurrentUri);
    return *this;
  }
  
  ~OriginActor() {}
};
template<>
bool JSValIs<OriginActor>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<OriginActor>(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv);
template<>
bool ToJSVal<OriginActor>(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<CryptoActor, OriginActor> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<CryptoActor, OriginActor>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<CryptoActor, OriginActor>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<CryptoActor, OriginActor>* mVariant;
};
template<>
bool JSValIs<SafeVariant<CryptoActor, OriginActor>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<CryptoActor, OriginActor>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<CryptoActor, OriginActor>& aRv);
template<>
bool ToJSVal<SafeVariant<CryptoActor, OriginActor>>(JSContext* aCx, const SafeVariant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetSigningKeyArgs {
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  GetSigningKeyArgs() = default;
  GetSigningKeyArgs(SafeVariant<CryptoActor, OriginActor>&& aWebAppActor) : mWebAppActor(std::move(aWebAppActor)) {}
  GetSigningKeyArgs(GetSigningKeyArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor))  {}
  GetSigningKeyArgs& operator=(GetSigningKeyArgs&& aOther) {
    mWebAppActor = std::move(aOther.mWebAppActor);
    return *this;
  }
  
  ~GetSigningKeyArgs() {}
};
template<>
bool JSValIs<GetSigningKeyArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<GetSigningKeyArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, GetSigningKeyArgs& aRv);
template<>
bool ToJSVal<GetSigningKeyArgs>(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv);
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE nsCString("An error has occurred"_ns)
#define BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME nsCString("An error has occurred"_ns)
struct Failure {
  RefPtr<mozilla::dom::Exception> mException;

  Failure() : Failure(NS_ERROR_FAILURE) {}
  Failure(nsresult res) : mException(new mozilla::dom::Exception(BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE, NS_ERROR_FAILURE, BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME)) {}

  ErrorResult ToErrorResult() const;
};
template<>
bool FromJSVal<Failure>(JSContext* aCx, JS::Handle<JS::Value> aValue, Failure& aRv);
using ManagerGetSigningKeyResult = MozPromise<nsString, Failure, true>;
template<>
bool JSValIs<nsTArray<nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<nsTArray<nsString>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<nsString>& aRv);
template<>
bool ToJSVal<nsTArray<nsString>>(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<nsTArray<nsString>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<nsTArray<nsString>>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<nsString>>& aRv);
template<>
bool ToJSVal<Maybe<nsTArray<nsString>>>(JSContext* aCx, const Maybe<nsTArray<nsString>>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<bool>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<bool>(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool ToJSVal<bool>(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<nsString>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsString>& aRv);
template<>
bool ToJSVal<Maybe<nsString>>(JSContext* aCx, const Maybe<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<double>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<double>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<double>& aRv);
template<>
bool ToJSVal<Maybe<double>>(JSContext* aCx, const Maybe<double>& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialAccountIdentity {
  nsString mFieldId;
  nsString mFieldValue;
  PartialAccountIdentity() = default;
  PartialAccountIdentity(nsString&& aFieldId, nsString&& aFieldValue) : mFieldId(std::move(aFieldId)), mFieldValue(std::move(aFieldValue)) {}
  PartialAccountIdentity(PartialAccountIdentity&& aOther) : mFieldId(std::move(aOther.mFieldId)), mFieldValue(std::move(aOther.mFieldValue))  {}
  PartialAccountIdentity& operator=(PartialAccountIdentity&& aOther) {
    mFieldId = std::move(aOther.mFieldId);
  mFieldValue = std::move(aOther.mFieldValue);
    return *this;
  }
  
  ~PartialAccountIdentity() {}
};
template<>
bool JSValIs<PartialAccountIdentity>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<PartialAccountIdentity>(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialAccountIdentity& aRv);
template<>
bool ToJSVal<PartialAccountIdentity>(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<nsTArray<PartialAccountIdentity>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<nsTArray<PartialAccountIdentity>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<PartialAccountIdentity>& aRv);
template<>
bool ToJSVal<nsTArray<PartialAccountIdentity>>(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<PartialAccountIdentity>>& aRv);
template<>
bool ToJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext* aCx, const Maybe<nsTArray<PartialAccountIdentity>>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AccountConstraints {
  Maybe<nsString> mCategory;
  Maybe<double> mSchemaVersion;
  Maybe<nsTArray<PartialAccountIdentity>> mIdentity;
  AccountConstraints() = default;
  AccountConstraints(Maybe<nsString>&& aCategory, Maybe<double>&& aSchemaVersion, Maybe<nsTArray<PartialAccountIdentity>>&& aIdentity) : mCategory(std::move(aCategory)), mSchemaVersion(std::move(aSchemaVersion)), mIdentity(std::move(aIdentity)) {}
  AccountConstraints(AccountConstraints&& aOther) : mCategory(std::move(aOther.mCategory)), mSchemaVersion(std::move(aOther.mSchemaVersion)), mIdentity(std::move(aOther.mIdentity))  {}
  AccountConstraints& operator=(AccountConstraints&& aOther) {
    mCategory = std::move(aOther.mCategory);
  mSchemaVersion = std::move(aOther.mSchemaVersion);
  mIdentity = std::move(aOther.mIdentity);
    return *this;
  }
  
  ~AccountConstraints() {}
};
template<>
bool JSValIs<AccountConstraints>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<AccountConstraints>(JSContext* aCx, JS::Handle<JS::Value> aValue, AccountConstraints& aRv);
template<>
bool ToJSVal<AccountConstraints>(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<AccountConstraints>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<AccountConstraints>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<AccountConstraints>& aRv);
template<>
bool ToJSVal<Maybe<AccountConstraints>>(JSContext* aCx, const Maybe<AccountConstraints>& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChannelConstraints {
  Maybe<nsTArray<nsString>> mSecretManagerPublicKey;
  bool mEnableEndToEndEncryption;
  Maybe<AccountConstraints> mAccount;
  ChannelConstraints() = default;
  ChannelConstraints(Maybe<nsTArray<nsString>>&& aSecretManagerPublicKey, bool&& aEnableEndToEndEncryption, Maybe<AccountConstraints>&& aAccount) : mSecretManagerPublicKey(std::move(aSecretManagerPublicKey)), mEnableEndToEndEncryption(std::move(aEnableEndToEndEncryption)), mAccount(std::move(aAccount)) {}
  ChannelConstraints(ChannelConstraints&& aOther) : mSecretManagerPublicKey(std::move(aOther.mSecretManagerPublicKey)), mEnableEndToEndEncryption(std::move(aOther.mEnableEndToEndEncryption)), mAccount(std::move(aOther.mAccount))  {}
  ChannelConstraints& operator=(ChannelConstraints&& aOther) {
    mSecretManagerPublicKey = std::move(aOther.mSecretManagerPublicKey);
  mEnableEndToEndEncryption = std::move(aOther.mEnableEndToEndEncryption);
  mAccount = std::move(aOther.mAccount);
    return *this;
  }
  
  ~ChannelConstraints() {}
};
template<>
bool JSValIs<ChannelConstraints>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ChannelConstraints>(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv);
template<>
bool ToJSVal<ChannelConstraints>(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetCredentialsMetadataArgs {
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  ChannelConstraints mChannelConstraints;
  AccountConstraints mAccountConstraints;
  GetCredentialsMetadataArgs() = default;
  GetCredentialsMetadataArgs(SafeVariant<CryptoActor, OriginActor>&& aWebAppActor, ChannelConstraints&& aChannelConstraints, AccountConstraints&& aAccountConstraints) : mWebAppActor(std::move(aWebAppActor)), mChannelConstraints(std::move(aChannelConstraints)), mAccountConstraints(std::move(aAccountConstraints)) {}
  GetCredentialsMetadataArgs(GetCredentialsMetadataArgs&& aOther) : mWebAppActor(std::move(aOther.mWebAppActor)), mChannelConstraints(std::move(aOther.mChannelConstraints)), mAccountConstraints(std::move(aOther.mAccountConstraints))  {}
  GetCredentialsMetadataArgs& operator=(GetCredentialsMetadataArgs&& aOther) {
    mWebAppActor = std::move(aOther.mWebAppActor);
  mChannelConstraints = std::move(aOther.mChannelConstraints);
  mAccountConstraints = std::move(aOther.mAccountConstraints);
    return *this;
  }
  
  ~GetCredentialsMetadataArgs() {}
};
template<>
bool JSValIs<GetCredentialsMetadataArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<GetCredentialsMetadataArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv);
template<>
bool ToJSVal<GetCredentialsMetadataArgs>(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using ManagerGetCredentialsMetadataResult = MozPromise<double, Failure, true>;
struct ChannelMetadata {
  nsString mId;
  ChannelConstraints mConstraints;
  SafeVariant<CryptoActor, OriginActor> mWebAppActor;
  CryptoActor mScmActor;
  ChannelMetadata() = default;
  ChannelMetadata(nsString&& aId, ChannelConstraints&& aConstraints, SafeVariant<CryptoActor, OriginActor>&& aWebAppActor, CryptoActor&& aScmActor) : mId(std::move(aId)), mConstraints(std::move(aConstraints)), mWebAppActor(std::move(aWebAppActor)), mScmActor(std::move(aScmActor)) {}
  ChannelMetadata(ChannelMetadata&& aOther) : mId(std::move(aOther.mId)), mConstraints(std::move(aOther.mConstraints)), mWebAppActor(std::move(aOther.mWebAppActor)), mScmActor(std::move(aOther.mScmActor))  {}
  ChannelMetadata& operator=(ChannelMetadata&& aOther) {
    mId = std::move(aOther.mId);
  mConstraints = std::move(aOther.mConstraints);
  mWebAppActor = std::move(aOther.mWebAppActor);
  mScmActor = std::move(aOther.mScmActor);
    return *this;
  }
  
  ~ChannelMetadata() {}
};
template<>
bool JSValIs<ChannelMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ChannelMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv);
template<>
bool ToJSVal<ChannelMetadata>(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestContext {
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContext() = default;
  RequestContext(ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContext(RequestContext&& aOther) : mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  RequestContext& operator=(RequestContext&& aOther) {
    mChannel = std::move(aOther.mChannel);
  mDocument = std::move(aOther.mDocument);
    return *this;
  }
  
  ~RequestContext() {}
};
template<>
bool JSValIs<RequestContext>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RequestContext>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv);
template<>
bool ToJSVal<RequestContext>(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
struct InitialKeyExchangeParametersDraft {
  nsString mChannelId;
  nsString mWebAppX25519Key;
  InitialKeyExchangeParametersDraft() = default;
  InitialKeyExchangeParametersDraft(nsString&& aChannelId, nsString&& aWebAppX25519Key) : mChannelId(std::move(aChannelId)), mWebAppX25519Key(std::move(aWebAppX25519Key)) {}
  InitialKeyExchangeParametersDraft(InitialKeyExchangeParametersDraft&& aOther) : mChannelId(std::move(aOther.mChannelId)), mWebAppX25519Key(std::move(aOther.mWebAppX25519Key))  {}
  InitialKeyExchangeParametersDraft& operator=(InitialKeyExchangeParametersDraft&& aOther) {
    mChannelId = std::move(aOther.mChannelId);
  mWebAppX25519Key = std::move(aOther.mWebAppX25519Key);
    return *this;
  }
  
  ~InitialKeyExchangeParametersDraft() {}
};
template<>
bool JSValIs<InitialKeyExchangeParametersDraft>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<InitialKeyExchangeParametersDraft>(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv);
template<>
bool ToJSVal<InitialKeyExchangeParametersDraft>(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv);
struct GenerateKeyExchangeParametersArgs {
  InitialKeyExchangeParametersDraft mParamsDraft;
  GenerateKeyExchangeParametersArgs() = default;
  GenerateKeyExchangeParametersArgs(InitialKeyExchangeParametersDraft&& aParamsDraft) : mParamsDraft(std::move(aParamsDraft)) {}
  GenerateKeyExchangeParametersArgs(GenerateKeyExchangeParametersArgs&& aOther) : mParamsDraft(std::move(aOther.mParamsDraft))  {}
  GenerateKeyExchangeParametersArgs& operator=(GenerateKeyExchangeParametersArgs&& aOther) {
    mParamsDraft = std::move(aOther.mParamsDraft);
    return *this;
  }
  
  ~GenerateKeyExchangeParametersArgs() {}
};
template<>
bool JSValIs<GenerateKeyExchangeParametersArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<GenerateKeyExchangeParametersArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, GenerateKeyExchangeParametersArgs& aRv);
template<>
bool ToJSVal<GenerateKeyExchangeParametersArgs>(JSContext* aCx, const GenerateKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<ArrayBuffer>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ArrayBuffer>(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBuffer& aRv);
template<>
bool ToJSVal<ArrayBuffer>(JSContext* aCx, const ArrayBuffer& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialKeyExchangeParametersFromScm {
  nsString mScmX25519Key;
  nsString mHkdfHash;
  ArrayBuffer mHkdfSalt;
  ArrayBuffer mHkdfInfo;
  double mAesKeyLength;
  PartialKeyExchangeParametersFromScm() = default;
  PartialKeyExchangeParametersFromScm(nsString&& aScmX25519Key, nsString&& aHkdfHash, ArrayBuffer&& aHkdfSalt, ArrayBuffer&& aHkdfInfo, double&& aAesKeyLength) : mScmX25519Key(std::move(aScmX25519Key)), mHkdfHash(std::move(aHkdfHash)), mHkdfSalt(std::move(aHkdfSalt)), mHkdfInfo(std::move(aHkdfInfo)), mAesKeyLength(std::move(aAesKeyLength)) {}
  PartialKeyExchangeParametersFromScm(PartialKeyExchangeParametersFromScm&& aOther) : mScmX25519Key(std::move(aOther.mScmX25519Key)), mHkdfHash(std::move(aOther.mHkdfHash)), mHkdfSalt(std::move(aOther.mHkdfSalt)), mHkdfInfo(std::move(aOther.mHkdfInfo)), mAesKeyLength(std::move(aOther.mAesKeyLength))  {}
  
  
  ~PartialKeyExchangeParametersFromScm() {}
};
template<>
bool JSValIs<PartialKeyExchangeParametersFromScm>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<PartialKeyExchangeParametersFromScm>(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialKeyExchangeParametersFromScm& aRv);
template<>
bool ToJSVal<PartialKeyExchangeParametersFromScm>(JSContext* aCx, const PartialKeyExchangeParametersFromScm& aValue, JS::MutableHandle<JS::Value> aRv);
using ChannelGenerateKeyExchangeParametersResult = MozPromise<PartialKeyExchangeParametersFromScm, Failure, true>;
struct KeyExchangeParameters {
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
};
template<>
bool JSValIs<KeyExchangeParameters>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<KeyExchangeParameters>(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv);
template<>
bool ToJSVal<KeyExchangeParameters>(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
struct EnableEndToEndEncryptionArgs {
  KeyExchangeParameters mParams;
  ArrayBuffer mWebAppPacketSignature;
  EnableEndToEndEncryptionArgs() = default;
  EnableEndToEndEncryptionArgs(KeyExchangeParameters&& aParams, ArrayBuffer&& aWebAppPacketSignature) : mParams(std::move(aParams)), mWebAppPacketSignature(std::move(aWebAppPacketSignature)) {}
  EnableEndToEndEncryptionArgs(EnableEndToEndEncryptionArgs&& aOther) : mParams(std::move(aOther.mParams)), mWebAppPacketSignature(std::move(aOther.mWebAppPacketSignature))  {}
  
  
  ~EnableEndToEndEncryptionArgs() {}
};
template<>
bool JSValIs<EnableEndToEndEncryptionArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<EnableEndToEndEncryptionArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv);
template<>
bool ToJSVal<EnableEndToEndEncryptionArgs>(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using ChannelEnableEndToEndEncryptionResult = MozPromise<ArrayBuffer, Failure, true>;
using ChannelCloseChannelResult = MozPromise<void*, Failure, true>;
struct ELoginUserIntent {
  uint8_t mVal;
  ELoginUserIntent() : mVal(0) {}
  ELoginUserIntent(uint8_t aVal) : mVal(aVal) {}
  ELoginUserIntent(ELoginUserIntent&& aOther) : mVal(std::move(aOther.mVal)) {}
  ELoginUserIntent& operator=(ELoginUserIntent&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
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
};
template<>
bool JSValIs<ELoginUserIntent>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ELoginUserIntent>(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv)
;
template<>
bool ToJSVal<ELoginUserIntent>(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv);

class StaticString0 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"name"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString0>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString0>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString0& aRv);
template<>
bool ToJSVal<StaticString0>(JSContext* aCx, const StaticString0& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_name = StaticString0;
class StaticString1 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"givenName"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString1>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString1>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString1& aRv);
template<>
bool ToJSVal<StaticString1>(JSContext* aCx, const StaticString1& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_givenName = StaticString1;
class StaticString2 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"familyName"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString2>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString2>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString2& aRv);
template<>
bool ToJSVal<StaticString2>(JSContext* aCx, const StaticString2& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_familyName = StaticString2;
class StaticString3 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"middleName"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString3>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString3>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString3& aRv);
template<>
bool ToJSVal<StaticString3>(JSContext* aCx, const StaticString3& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_middleName = StaticString3;
class StaticString4 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"nickname"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString4>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString4>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString4& aRv);
template<>
bool ToJSVal<StaticString4>(JSContext* aCx, const StaticString4& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_nickname = StaticString4;
class StaticString5 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"profile"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString5>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString5>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString5& aRv);
template<>
bool ToJSVal<StaticString5>(JSContext* aCx, const StaticString5& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_profile = StaticString5;
class StaticString6 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"picture"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString6>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString6>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString6& aRv);
template<>
bool ToJSVal<StaticString6>(JSContext* aCx, const StaticString6& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_picture = StaticString6;
class StaticString7 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"website"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString7>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString7>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString7& aRv);
template<>
bool ToJSVal<StaticString7>(JSContext* aCx, const StaticString7& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_website = StaticString7;
class StaticString8 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"gender"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString8>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString8>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString8& aRv);
template<>
bool ToJSVal<StaticString8>(JSContext* aCx, const StaticString8& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_gender = StaticString8;
class StaticString9 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"birthdate"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString9>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString9>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString9& aRv);
template<>
bool ToJSVal<StaticString9>(JSContext* aCx, const StaticString9& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_birthdate = StaticString9;
class StaticString10 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"zoneinfo"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString10>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString10>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString10& aRv);
template<>
bool ToJSVal<StaticString10>(JSContext* aCx, const StaticString10& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_zoneinfo = StaticString10;
class StaticString11 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"locale"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString11>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString11>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString11& aRv);
template<>
bool ToJSVal<StaticString11>(JSContext* aCx, const StaticString11& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_locale = StaticString11;
class StaticString12 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"address"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString12>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString12>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString12& aRv);
template<>
bool ToJSVal<StaticString12>(JSContext* aCx, const StaticString12& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_address = StaticString12;
template<>
class SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString> const* InternalValue() const { return mVariant; }
  mozilla::Variant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>* InternalValue() { return mVariant; }
  
  nsString AsString() const {
    MOZ_ASSERT(mVariant);
    return mVariant->match(
        [](StaticString0& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString1& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString2& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString3& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString4& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString5& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString6& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString7& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString8& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString9& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString10& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString11& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString12& aStr) -> nsString {
          return aStr.GetString();
        },
    [](nsString& aStr) -> nsString {
          return aStr;
        }
    );
  }
protected:
  mozilla::Variant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>* mVariant;
};
template<>
bool JSValIs<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>& aRv);
template<>
bool ToJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext* aCx, const SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestedUserAttribute {
  SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString> mId;
  bool mRequired;
  RequestedUserAttribute() = default;
  RequestedUserAttribute(SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>&& aId, bool&& aRequired) : mId(std::move(aId)), mRequired(std::move(aRequired)) {}
  RequestedUserAttribute(RequestedUserAttribute&& aOther) : mId(std::move(aOther.mId)), mRequired(std::move(aOther.mRequired))  {}
  RequestedUserAttribute& operator=(RequestedUserAttribute&& aOther) {
    mId = std::move(aOther.mId);
  mRequired = std::move(aOther.mRequired);
    return *this;
  }
  
  ~RequestedUserAttribute() {}
};
template<>
bool JSValIs<RequestedUserAttribute>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RequestedUserAttribute>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv);
template<>
bool ToJSVal<RequestedUserAttribute>(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<nsTArray<RequestedUserAttribute>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<nsTArray<RequestedUserAttribute>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<RequestedUserAttribute>& aRv);
template<>
bool ToJSVal<nsTArray<RequestedUserAttribute>>(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EBerytusFieldType {
  uint8_t mVal;
  EBerytusFieldType() : mVal(0) {}
  EBerytusFieldType(uint8_t aVal) : mVal(aVal) {}
  EBerytusFieldType(EBerytusFieldType&& aOther) : mVal(std::move(aOther.mVal)) {}
  EBerytusFieldType& operator=(EBerytusFieldType&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
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
};
template<>
bool JSValIs<EBerytusFieldType>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<EBerytusFieldType>(JSContext* aCx, JS::Handle<JS::Value> aValue, EBerytusFieldType& aRv)
;
template<>
bool ToJSVal<EBerytusFieldType>(JSContext* aCx, const EBerytusFieldType& aValue, JS::MutableHandle<JS::Value> aRv);

struct BerytusFieldCategoryOptions {
  nsString mCategoryId;
  Maybe<double> mPosition;
  BerytusFieldCategoryOptions() = default;
  BerytusFieldCategoryOptions(nsString&& aCategoryId, Maybe<double>&& aPosition) : mCategoryId(std::move(aCategoryId)), mPosition(std::move(aPosition)) {}
  BerytusFieldCategoryOptions(BerytusFieldCategoryOptions&& aOther) : mCategoryId(std::move(aOther.mCategoryId)), mPosition(std::move(aOther.mPosition))  {}
  BerytusFieldCategoryOptions& operator=(BerytusFieldCategoryOptions&& aOther) {
    mCategoryId = std::move(aOther.mCategoryId);
  mPosition = std::move(aOther.mPosition);
    return *this;
  }
  
  ~BerytusFieldCategoryOptions() {}
};
template<>
bool JSValIs<BerytusFieldCategoryOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusFieldCategoryOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusFieldCategoryOptions& aRv);
template<>
bool ToJSVal<BerytusFieldCategoryOptions>(JSContext* aCx, const BerytusFieldCategoryOptions& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Maybe<BerytusFieldCategoryOptions>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Maybe<BerytusFieldCategoryOptions>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<BerytusFieldCategoryOptions>& aRv);
template<>
bool ToJSVal<Maybe<BerytusFieldCategoryOptions>>(JSContext* aCx, const Maybe<BerytusFieldCategoryOptions>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusForeignIdentityFieldOptions {
  bool mPrivate;
  nsString mKind;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusForeignIdentityFieldOptions() = default;
  BerytusForeignIdentityFieldOptions(bool&& aPrivate, nsString&& aKind, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mPrivate(std::move(aPrivate)), mKind(std::move(aKind)), mCategory(std::move(aCategory)) {}
  BerytusForeignIdentityFieldOptions(BerytusForeignIdentityFieldOptions&& aOther) : mPrivate(std::move(aOther.mPrivate)), mKind(std::move(aOther.mKind)), mCategory(std::move(aOther.mCategory))  {}
  BerytusForeignIdentityFieldOptions& operator=(BerytusForeignIdentityFieldOptions&& aOther) {
    mPrivate = std::move(aOther.mPrivate);
  mKind = std::move(aOther.mKind);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusForeignIdentityFieldOptions() {}
};
template<>
bool JSValIs<BerytusForeignIdentityFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusForeignIdentityFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityFieldOptions& aRv);
template<>
bool ToJSVal<BerytusForeignIdentityFieldOptions>(JSContext* aCx, const BerytusForeignIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusIdentityFieldOptions {
  bool mHumanReadable;
  bool mPrivate;
  double mMaxLength;
  Maybe<nsString> mAllowedCharacters;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusIdentityFieldOptions() = default;
  BerytusIdentityFieldOptions(bool&& aHumanReadable, bool&& aPrivate, double&& aMaxLength, Maybe<nsString>&& aAllowedCharacters, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mHumanReadable(std::move(aHumanReadable)), mPrivate(std::move(aPrivate)), mMaxLength(std::move(aMaxLength)), mAllowedCharacters(std::move(aAllowedCharacters)), mCategory(std::move(aCategory)) {}
  BerytusIdentityFieldOptions(BerytusIdentityFieldOptions&& aOther) : mHumanReadable(std::move(aOther.mHumanReadable)), mPrivate(std::move(aOther.mPrivate)), mMaxLength(std::move(aOther.mMaxLength)), mAllowedCharacters(std::move(aOther.mAllowedCharacters)), mCategory(std::move(aOther.mCategory))  {}
  BerytusIdentityFieldOptions& operator=(BerytusIdentityFieldOptions&& aOther) {
    mHumanReadable = std::move(aOther.mHumanReadable);
  mPrivate = std::move(aOther.mPrivate);
  mMaxLength = std::move(aOther.mMaxLength);
  mAllowedCharacters = std::move(aOther.mAllowedCharacters);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusIdentityFieldOptions() {}
};
template<>
bool JSValIs<BerytusIdentityFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusIdentityFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityFieldOptions& aRv);
template<>
bool ToJSVal<BerytusIdentityFieldOptions>(JSContext* aCx, const BerytusIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusKeyFieldOptions {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusKeyFieldOptions() = default;
  BerytusKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusKeyFieldOptions(BerytusKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory))  {}
  BerytusKeyFieldOptions& operator=(BerytusKeyFieldOptions&& aOther) {
    mAlg = std::move(aOther.mAlg);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusKeyFieldOptions() {}
};
template<>
bool JSValIs<BerytusKeyFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusKeyFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldOptions& aRv);
template<>
bool ToJSVal<BerytusKeyFieldOptions>(JSContext* aCx, const BerytusKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusPasswordFieldOptions {
  Maybe<nsString> mPasswordRules;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusPasswordFieldOptions() = default;
  BerytusPasswordFieldOptions(Maybe<nsString>&& aPasswordRules, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mPasswordRules(std::move(aPasswordRules)), mCategory(std::move(aCategory)) {}
  BerytusPasswordFieldOptions(BerytusPasswordFieldOptions&& aOther) : mPasswordRules(std::move(aOther.mPasswordRules)), mCategory(std::move(aOther.mCategory))  {}
  BerytusPasswordFieldOptions& operator=(BerytusPasswordFieldOptions&& aOther) {
    mPasswordRules = std::move(aOther.mPasswordRules);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusPasswordFieldOptions() {}
};
template<>
bool JSValIs<BerytusPasswordFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusPasswordFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordFieldOptions& aRv);
template<>
bool ToJSVal<BerytusPasswordFieldOptions>(JSContext* aCx, const BerytusPasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSecurePasswordFieldOptions {
  nsString mIdentityFieldId;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusSecurePasswordFieldOptions() = default;
  BerytusSecurePasswordFieldOptions(nsString&& aIdentityFieldId, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mIdentityFieldId(std::move(aIdentityFieldId)), mCategory(std::move(aCategory)) {}
  BerytusSecurePasswordFieldOptions(BerytusSecurePasswordFieldOptions&& aOther) : mIdentityFieldId(std::move(aOther.mIdentityFieldId)), mCategory(std::move(aOther.mCategory))  {}
  BerytusSecurePasswordFieldOptions& operator=(BerytusSecurePasswordFieldOptions&& aOther) {
    mIdentityFieldId = std::move(aOther.mIdentityFieldId);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusSecurePasswordFieldOptions() {}
};
template<>
bool JSValIs<BerytusSecurePasswordFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSecurePasswordFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldOptions& aRv);
template<>
bool ToJSVal<BerytusSecurePasswordFieldOptions>(JSContext* aCx, const BerytusSecurePasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSharedKeyFieldOptions {
  double mAlg;
  Maybe<BerytusFieldCategoryOptions> mCategory;
  BerytusSharedKeyFieldOptions() = default;
  BerytusSharedKeyFieldOptions(double&& aAlg, Maybe<BerytusFieldCategoryOptions>&& aCategory) : mAlg(std::move(aAlg)), mCategory(std::move(aCategory)) {}
  BerytusSharedKeyFieldOptions(BerytusSharedKeyFieldOptions&& aOther) : mAlg(std::move(aOther.mAlg)), mCategory(std::move(aOther.mCategory))  {}
  BerytusSharedKeyFieldOptions& operator=(BerytusSharedKeyFieldOptions&& aOther) {
    mAlg = std::move(aOther.mAlg);
  mCategory = std::move(aOther.mCategory);
    return *this;
  }
  
  ~BerytusSharedKeyFieldOptions() {}
};
template<>
bool JSValIs<BerytusSharedKeyFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSharedKeyFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldOptions& aRv);
template<>
bool ToJSVal<BerytusSharedKeyFieldOptions>(JSContext* aCx, const BerytusSharedKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions> const* InternalValue() const { return mVariant; }
  mozilla::Variant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>* mVariant;
};
template<>
bool JSValIs<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>& aRv);
template<>
bool ToJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext* aCx, const SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>& aValue, JS::MutableHandle<JS::Value> aRv);
struct FieldInfo {
  nsString mId;
  EBerytusFieldType mType;
  SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions> mOptions;
  FieldInfo() = default;
  FieldInfo(nsString&& aId, EBerytusFieldType&& aType, SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>&& aOptions) : mId(std::move(aId)), mType(std::move(aType)), mOptions(std::move(aOptions)) {}
  FieldInfo(FieldInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions))  {}
  FieldInfo& operator=(FieldInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
    return *this;
  }
  
  ~FieldInfo() {}
};
template<>
bool JSValIs<FieldInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<FieldInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldInfo& aRv);
template<>
bool ToJSVal<FieldInfo>(JSContext* aCx, const FieldInfo& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Record<nsString, FieldInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Record<nsString, FieldInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, FieldInfo>& aRv);
template<>
bool ToJSVal<Record<nsString, FieldInfo>>(JSContext* aCx, const Record<nsString, FieldInfo>& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString13 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"Identification"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString13>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString13>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString13& aRv);
template<>
bool ToJSVal<StaticString13>(JSContext* aCx, const StaticString13& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_Identification = StaticString13;

struct JSNull {};
template<>
bool JSValIs<JSNull>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<JSNull>(JSContext* aCx, JS::Handle<JS::Value> aValue, JSNull& aRv);
template<>
bool ToJSVal<JSNull>(JSContext* aCx, const JSNull& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusIdentificationChallengeInfo {
  nsString mId;
  StaticString13 mType;
  JSNull mParameters;
  BerytusIdentificationChallengeInfo() = default;
  BerytusIdentificationChallengeInfo(nsString&& aId, StaticString13&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusIdentificationChallengeInfo(BerytusIdentificationChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  BerytusIdentificationChallengeInfo& operator=(BerytusIdentificationChallengeInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mParameters = std::move(aOther.mParameters);
    return *this;
  }
  
  ~BerytusIdentificationChallengeInfo() {}
};
template<>
bool JSValIs<BerytusIdentificationChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusIdentificationChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentificationChallengeInfo& aRv);
template<>
bool ToJSVal<BerytusIdentificationChallengeInfo>(JSContext* aCx, const BerytusIdentificationChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString14 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"Password"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString14>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString14>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString14& aRv);
template<>
bool ToJSVal<StaticString14>(JSContext* aCx, const StaticString14& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_Password = StaticString14;
struct BerytusPasswordChallengeInfo {
  nsString mId;
  StaticString14 mType;
  JSNull mParameters;
  BerytusPasswordChallengeInfo() = default;
  BerytusPasswordChallengeInfo(nsString&& aId, StaticString14&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusPasswordChallengeInfo(BerytusPasswordChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  BerytusPasswordChallengeInfo& operator=(BerytusPasswordChallengeInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mParameters = std::move(aOther.mParameters);
    return *this;
  }
  
  ~BerytusPasswordChallengeInfo() {}
};
template<>
bool JSValIs<BerytusPasswordChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusPasswordChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordChallengeInfo& aRv);
template<>
bool ToJSVal<BerytusPasswordChallengeInfo>(JSContext* aCx, const BerytusPasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString15 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"DigitalSignature"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString15>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString15>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString15& aRv);
template<>
bool ToJSVal<StaticString15>(JSContext* aCx, const StaticString15& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_DigitalSignature = StaticString15;
struct BerytusDigitalSignatureChallengeInfo {
  nsString mId;
  StaticString15 mType;
  JSNull mParameters;
  BerytusDigitalSignatureChallengeInfo() = default;
  BerytusDigitalSignatureChallengeInfo(nsString&& aId, StaticString15&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusDigitalSignatureChallengeInfo(BerytusDigitalSignatureChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  BerytusDigitalSignatureChallengeInfo& operator=(BerytusDigitalSignatureChallengeInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mParameters = std::move(aOther.mParameters);
    return *this;
  }
  
  ~BerytusDigitalSignatureChallengeInfo() {}
};
template<>
bool JSValIs<BerytusDigitalSignatureChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusDigitalSignatureChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusDigitalSignatureChallengeInfo& aRv);
template<>
bool ToJSVal<BerytusDigitalSignatureChallengeInfo>(JSContext* aCx, const BerytusDigitalSignatureChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString16 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SecureRemotePassword"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString16>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString16>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString16& aRv);
template<>
bool ToJSVal<StaticString16>(JSContext* aCx, const StaticString16& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SecureRemotePassword = StaticString16;
class StaticString17 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"None"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString17>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString17>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString17& aRv);
template<>
bool ToJSVal<StaticString17>(JSContext* aCx, const StaticString17& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_None = StaticString17;
class StaticString18 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"Hex"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString18>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString18>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString18& aRv);
template<>
bool ToJSVal<StaticString18>(JSContext* aCx, const StaticString18& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_Hex = StaticString18;
template<>
bool JSValIs<Nothing>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Nothing>(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
template<>
bool ToJSVal<Nothing>(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<StaticString17, StaticString18, Nothing> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<StaticString17, StaticString18, Nothing>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<StaticString17, StaticString18, Nothing>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<StaticString17, StaticString18, Nothing> const* InternalValue() const { return mVariant; }
  mozilla::Variant<StaticString17, StaticString18, Nothing>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<StaticString17, StaticString18, Nothing>* mVariant;
};
template<>
bool JSValIs<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString17, StaticString18, Nothing>& aRv);
template<>
bool ToJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext* aCx, const SafeVariant<StaticString17, StaticString18, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSecureRemotePasswordChallengeParameters {
  SafeVariant<StaticString17, StaticString18, Nothing> mEncoding;
  BerytusSecureRemotePasswordChallengeParameters() = default;
  BerytusSecureRemotePasswordChallengeParameters(SafeVariant<StaticString17, StaticString18, Nothing>&& aEncoding) : mEncoding(std::move(aEncoding)) {}
  BerytusSecureRemotePasswordChallengeParameters(BerytusSecureRemotePasswordChallengeParameters&& aOther) : mEncoding(std::move(aOther.mEncoding))  {}
  BerytusSecureRemotePasswordChallengeParameters& operator=(BerytusSecureRemotePasswordChallengeParameters&& aOther) {
    mEncoding = std::move(aOther.mEncoding);
    return *this;
  }
  
  ~BerytusSecureRemotePasswordChallengeParameters() {}
};
template<>
bool JSValIs<BerytusSecureRemotePasswordChallengeParameters>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSecureRemotePasswordChallengeParameters>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeParameters& aRv);
template<>
bool ToJSVal<BerytusSecureRemotePasswordChallengeParameters>(JSContext* aCx, const BerytusSecureRemotePasswordChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSecureRemotePasswordChallengeInfo {
  nsString mId;
  StaticString16 mType;
  BerytusSecureRemotePasswordChallengeParameters mParameters;
  BerytusSecureRemotePasswordChallengeInfo() = default;
  BerytusSecureRemotePasswordChallengeInfo(nsString&& aId, StaticString16&& aType, BerytusSecureRemotePasswordChallengeParameters&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusSecureRemotePasswordChallengeInfo(BerytusSecureRemotePasswordChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  BerytusSecureRemotePasswordChallengeInfo& operator=(BerytusSecureRemotePasswordChallengeInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mParameters = std::move(aOther.mParameters);
    return *this;
  }
  
  ~BerytusSecureRemotePasswordChallengeInfo() {}
};
template<>
bool JSValIs<BerytusSecureRemotePasswordChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeInfo& aRv);
template<>
bool ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(JSContext* aCx, const BerytusSecureRemotePasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString19 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"OffChannelOtp"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString19>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString19>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString19& aRv);
template<>
bool ToJSVal<StaticString19>(JSContext* aCx, const StaticString19& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_OffChannelOtp = StaticString19;
struct BerytusOffChannelOtpChallengeInfo {
  nsString mId;
  StaticString19 mType;
  JSNull mParameters;
  BerytusOffChannelOtpChallengeInfo() = default;
  BerytusOffChannelOtpChallengeInfo(nsString&& aId, StaticString19&& aType, JSNull&& aParameters) : mId(std::move(aId)), mType(std::move(aType)), mParameters(std::move(aParameters)) {}
  BerytusOffChannelOtpChallengeInfo(BerytusOffChannelOtpChallengeInfo&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mParameters(std::move(aOther.mParameters))  {}
  BerytusOffChannelOtpChallengeInfo& operator=(BerytusOffChannelOtpChallengeInfo&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mParameters = std::move(aOther.mParameters);
    return *this;
  }
  
  ~BerytusOffChannelOtpChallengeInfo() {}
};
template<>
bool JSValIs<BerytusOffChannelOtpChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusOffChannelOtpChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusOffChannelOtpChallengeInfo& aRv);
template<>
bool ToJSVal<BerytusOffChannelOtpChallengeInfo>(JSContext* aCx, const BerytusOffChannelOtpChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo> const* InternalValue() const { return mVariant; }
  mozilla::Variant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>* mVariant;
};
template<>
bool JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>& aRv);
template<>
bool ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext* aCx, const SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>& aRv);
template<>
bool ToJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext* aCx, const Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>& aValue, JS::MutableHandle<JS::Value> aRv);
struct EOperationType {
  uint8_t mVal;
  EOperationType() : mVal(0) {}
  EOperationType(uint8_t aVal) : mVal(aVal) {}
  EOperationType(EOperationType&& aOther) : mVal(std::move(aOther.mVal)) {}
  EOperationType& operator=(EOperationType&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
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
};
template<>
bool JSValIs<EOperationType>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<EOperationType>(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationType& aRv)
;
template<>
bool ToJSVal<EOperationType>(JSContext* aCx, const EOperationType& aValue, JS::MutableHandle<JS::Value> aRv);

struct EOperationStatus {
  uint8_t mVal;
  EOperationStatus() : mVal(0) {}
  EOperationStatus(uint8_t aVal) : mVal(aVal) {}
  EOperationStatus(EOperationStatus&& aOther) : mVal(std::move(aOther.mVal)) {}
  EOperationStatus& operator=(EOperationStatus&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
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
};
template<>
bool JSValIs<EOperationStatus>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<EOperationStatus>(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv)
;
template<>
bool ToJSVal<EOperationStatus>(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv);

struct OperationState {
  
  OperationState() = default;
  
  ~OperationState() {}
};
template<>
bool JSValIs<OperationState>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<OperationState>(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv);
template<>
bool ToJSVal<OperationState>(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv);
struct LoginOperationMetadata {
  ELoginUserIntent mIntent;
  nsTArray<RequestedUserAttribute> mRequestedUserAttributes;
  Record<nsString, FieldInfo> mFields;
  Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>> mChallenges;
  nsString mId;
  EOperationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  LoginOperationMetadata() = default;
  LoginOperationMetadata(ELoginUserIntent&& aIntent, nsTArray<RequestedUserAttribute>&& aRequestedUserAttributes, Record<nsString, FieldInfo>&& aFields, Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>&& aChallenges, nsString&& aId, EOperationType&& aType, EOperationStatus&& aStatus, OperationState&& aState) : mIntent(std::move(aIntent)), mRequestedUserAttributes(std::move(aRequestedUserAttributes)), mFields(std::move(aFields)), mChallenges(std::move(aChallenges)), mId(std::move(aId)), mType(std::move(aType)), mStatus(std::move(aStatus)), mState(std::move(aState)) {}
  LoginOperationMetadata(LoginOperationMetadata&& aOther) : mIntent(std::move(aOther.mIntent)), mRequestedUserAttributes(std::move(aOther.mRequestedUserAttributes)), mFields(std::move(aOther.mFields)), mChallenges(std::move(aOther.mChallenges)), mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState))  {}
  
  
  ~LoginOperationMetadata() {}
};
template<>
bool JSValIs<LoginOperationMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<LoginOperationMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv);
template<>
bool ToJSVal<LoginOperationMetadata>(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct ApproveOperationArgs {
  LoginOperationMetadata mOperation;
  ApproveOperationArgs() = default;
  ApproveOperationArgs(LoginOperationMetadata&& aOperation) : mOperation(std::move(aOperation)) {}
  ApproveOperationArgs(ApproveOperationArgs&& aOther) : mOperation(std::move(aOther.mOperation))  {}
  
  
  ~ApproveOperationArgs() {}
};
template<>
bool JSValIs<ApproveOperationArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ApproveOperationArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv);
template<>
bool ToJSVal<ApproveOperationArgs>(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using LoginApproveOperationResult = MozPromise<ELoginUserIntent, Failure, true>;
struct OperationMetadata {
  nsString mId;
  EOperationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  OperationMetadata() = default;
  OperationMetadata(nsString&& aId, EOperationType&& aType, EOperationStatus&& aStatus, OperationState&& aState) : mId(std::move(aId)), mType(std::move(aType)), mStatus(std::move(aStatus)), mState(std::move(aState)) {}
  OperationMetadata(OperationMetadata&& aOther) : mId(std::move(aOther.mId)), mType(std::move(aOther.mType)), mStatus(std::move(aOther.mStatus)), mState(std::move(aOther.mState))  {}
  OperationMetadata& operator=(OperationMetadata&& aOther) {
    mId = std::move(aOther.mId);
  mType = std::move(aOther.mType);
  mStatus = std::move(aOther.mStatus);
  mState = std::move(aOther.mState);
    return *this;
  }
  
  ~OperationMetadata() {}
};
template<>
bool JSValIs<OperationMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<OperationMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv);
template<>
bool ToJSVal<OperationMetadata>(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestContextWithOperation {
  OperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContextWithOperation() = default;
  RequestContextWithOperation(OperationMetadata&& aOperation, ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mOperation(std::move(aOperation)), mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContextWithOperation(RequestContextWithOperation&& aOther) : mOperation(std::move(aOther.mOperation)), mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  RequestContextWithOperation& operator=(RequestContextWithOperation&& aOther) {
    mOperation = std::move(aOther.mOperation);
  mChannel = std::move(aOther.mChannel);
  mDocument = std::move(aOther.mDocument);
    return *this;
  }
  
  ~RequestContextWithOperation() {}
};
template<>
bool JSValIs<RequestContextWithOperation>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RequestContextWithOperation>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv);
template<>
bool ToJSVal<RequestContextWithOperation>(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv);
using LoginCloseOperationResult = MozPromise<void*, Failure, true>;
struct EMetadataStatus {
  uint8_t mVal;
  EMetadataStatus() : mVal(0) {}
  EMetadataStatus(uint8_t aVal) : mVal(aVal) {}
  EMetadataStatus(EMetadataStatus&& aOther) : mVal(std::move(aOther.mVal)) {}
  EMetadataStatus& operator=(EMetadataStatus&& aOther) {
    mVal = std::move(aOther.mVal);
    return *this;
  }
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
};
template<>
bool JSValIs<EMetadataStatus>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<EMetadataStatus>(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv)
;
template<>
bool ToJSVal<EMetadataStatus>(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv);

struct RecordMetadata {
  double mVersion;
  EMetadataStatus mStatus;
  nsString mCategory;
  nsString mChangePassUrl;
  RecordMetadata() = default;
  RecordMetadata(double&& aVersion, EMetadataStatus&& aStatus, nsString&& aCategory, nsString&& aChangePassUrl) : mVersion(std::move(aVersion)), mStatus(std::move(aStatus)), mCategory(std::move(aCategory)), mChangePassUrl(std::move(aChangePassUrl)) {}
  RecordMetadata(RecordMetadata&& aOther) : mVersion(std::move(aOther.mVersion)), mStatus(std::move(aOther.mStatus)), mCategory(std::move(aOther.mCategory)), mChangePassUrl(std::move(aOther.mChangePassUrl))  {}
  RecordMetadata& operator=(RecordMetadata&& aOther) {
    mVersion = std::move(aOther.mVersion);
  mStatus = std::move(aOther.mStatus);
  mCategory = std::move(aOther.mCategory);
  mChangePassUrl = std::move(aOther.mChangePassUrl);
    return *this;
  }
  
  ~RecordMetadata() {}
};
template<>
bool JSValIs<RecordMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RecordMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv);
template<>
bool ToJSVal<RecordMetadata>(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
using LoginGetRecordMetadataResult = MozPromise<RecordMetadata, Failure, true>;
struct UpdateMetadataArgs {
  RecordMetadata mMetadata;
  UpdateMetadataArgs() = default;
  UpdateMetadataArgs(RecordMetadata&& aMetadata) : mMetadata(std::move(aMetadata)) {}
  UpdateMetadataArgs(UpdateMetadataArgs&& aOther) : mMetadata(std::move(aOther.mMetadata))  {}
  UpdateMetadataArgs& operator=(UpdateMetadataArgs&& aOther) {
    mMetadata = std::move(aOther.mMetadata);
    return *this;
  }
  
  ~UpdateMetadataArgs() {}
};
template<>
bool JSValIs<UpdateMetadataArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<UpdateMetadataArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv);
template<>
bool ToJSVal<UpdateMetadataArgs>(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using LoginUpdateMetadataResult = MozPromise<void*, Failure, true>;
struct ApproveTransitionToAuthOpArgs {
  LoginOperationMetadata mNewAuthOp;
  ApproveTransitionToAuthOpArgs() = default;
  ApproveTransitionToAuthOpArgs(LoginOperationMetadata&& aNewAuthOp) : mNewAuthOp(std::move(aNewAuthOp)) {}
  ApproveTransitionToAuthOpArgs(ApproveTransitionToAuthOpArgs&& aOther) : mNewAuthOp(std::move(aOther.mNewAuthOp))  {}
  
  
  ~ApproveTransitionToAuthOpArgs() {}
};
template<>
bool JSValIs<ApproveTransitionToAuthOpArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ApproveTransitionToAuthOpArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv);
template<>
bool ToJSVal<ApproveTransitionToAuthOpArgs>(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationApproveTransitionToAuthOpResult = MozPromise<void*, Failure, true>;
struct RequestContextWithLoginOperation {
  LoginOperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  RequestContextWithLoginOperation() = default;
  RequestContextWithLoginOperation(LoginOperationMetadata&& aOperation, ChannelMetadata&& aChannel, DocumentMetadata&& aDocument) : mOperation(std::move(aOperation)), mChannel(std::move(aChannel)), mDocument(std::move(aDocument)) {}
  RequestContextWithLoginOperation(RequestContextWithLoginOperation&& aOther) : mOperation(std::move(aOther.mOperation)), mChannel(std::move(aOther.mChannel)), mDocument(std::move(aOther.mDocument))  {}
  
  
  ~RequestContextWithLoginOperation() {}
};
template<>
bool JSValIs<RequestContextWithLoginOperation>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RequestContextWithLoginOperation>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithLoginOperation& aRv);
template<>
bool ToJSVal<RequestContextWithLoginOperation>(JSContext* aCx, const RequestContextWithLoginOperation& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<ArrayBufferView>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ArrayBufferView>(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBufferView& aRv);
template<>
bool ToJSVal<ArrayBufferView>(JSContext* aCx, const ArrayBufferView& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<ArrayBuffer, ArrayBufferView> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<ArrayBuffer, ArrayBufferView>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<ArrayBuffer, ArrayBufferView>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<ArrayBuffer, ArrayBufferView>* mVariant;
};
template<>
bool JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView>& aRv);
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<ArrayBuffer, ArrayBufferView, Nothing> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<ArrayBuffer, ArrayBufferView, Nothing>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<ArrayBuffer, ArrayBufferView, Nothing>* mVariant;
};
template<>
bool JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aRv);
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>>(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AesGcmParams {
  SafeVariant<ArrayBuffer, ArrayBufferView> mIv;
  SafeVariant<ArrayBuffer, ArrayBufferView, Nothing> mAdditionalData;
  Maybe<double> mTagLength;
  nsString mName;
  AesGcmParams() = default;
  AesGcmParams(SafeVariant<ArrayBuffer, ArrayBufferView>&& aIv, SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>&& aAdditionalData, Maybe<double>&& aTagLength, nsString&& aName) : mIv(std::move(aIv)), mAdditionalData(std::move(aAdditionalData)), mTagLength(std::move(aTagLength)), mName(std::move(aName)) {}
  AesGcmParams(AesGcmParams&& aOther) : mIv(std::move(aOther.mIv)), mAdditionalData(std::move(aOther.mAdditionalData)), mTagLength(std::move(aOther.mTagLength)), mName(std::move(aOther.mName))  {}
  AesGcmParams& operator=(AesGcmParams&& aOther) {
    mIv = std::move(aOther.mIv);
  mAdditionalData = std::move(aOther.mAdditionalData);
  mTagLength = std::move(aOther.mTagLength);
  mName = std::move(aOther.mName);
    return *this;
  }
  
  ~AesGcmParams() {}
};
template<>
bool JSValIs<AesGcmParams>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<AesGcmParams>(JSContext* aCx, JS::Handle<JS::Value> aValue, AesGcmParams& aRv);
template<>
bool ToJSVal<AesGcmParams>(JSContext* aCx, const AesGcmParams& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusEncryptedPacket {
  AesGcmParams mParameters;
  ArrayBuffer mCiphertext;
  BerytusEncryptedPacket() = default;
  BerytusEncryptedPacket(AesGcmParams&& aParameters, ArrayBuffer&& aCiphertext) : mParameters(std::move(aParameters)), mCiphertext(std::move(aCiphertext)) {}
  BerytusEncryptedPacket(BerytusEncryptedPacket&& aOther) : mParameters(std::move(aOther.mParameters)), mCiphertext(std::move(aOther.mCiphertext))  {}
  
  
  ~BerytusEncryptedPacket() {}
};
template<>
bool JSValIs<BerytusEncryptedPacket>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusEncryptedPacket>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusEncryptedPacket& aRv);
template<>
bool ToJSVal<BerytusEncryptedPacket>(JSContext* aCx, const BerytusEncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct UserAttribute {
  SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString> mId;
  Maybe<nsString> mInfo;
  Maybe<nsString> mMimeType;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mValue;
  UserAttribute() = default;
  UserAttribute(SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>&& aId, Maybe<nsString>&& aInfo, Maybe<nsString>&& aMimeType, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aValue) : mId(std::move(aId)), mInfo(std::move(aInfo)), mMimeType(std::move(aMimeType)), mValue(std::move(aValue)) {}
  UserAttribute(UserAttribute&& aOther) : mId(std::move(aOther.mId)), mInfo(std::move(aOther.mInfo)), mMimeType(std::move(aOther.mMimeType)), mValue(std::move(aOther.mValue))  {}
  UserAttribute& operator=(UserAttribute&& aOther) {
    mId = std::move(aOther.mId);
  mInfo = std::move(aOther.mInfo);
  mMimeType = std::move(aOther.mMimeType);
  mValue = std::move(aOther.mValue);
    return *this;
  }
  
  ~UserAttribute() {}
};
template<>
bool JSValIs<UserAttribute>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<UserAttribute>(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv);
template<>
bool ToJSVal<UserAttribute>(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<nsTArray<UserAttribute>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<nsTArray<UserAttribute>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv);
template<>
bool ToJSVal<nsTArray<UserAttribute>>(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationGetUserAttributesResult = MozPromise<nsTArray<UserAttribute>, Failure, true>;
struct UpdateUserAttributesArgs {
  nsTArray<UserAttribute> mUserAttributes;
  UpdateUserAttributesArgs() = default;
  UpdateUserAttributesArgs(nsTArray<UserAttribute>&& aUserAttributes) : mUserAttributes(std::move(aUserAttributes)) {}
  UpdateUserAttributesArgs(UpdateUserAttributesArgs&& aOther) : mUserAttributes(std::move(aOther.mUserAttributes))  {}
  UpdateUserAttributesArgs& operator=(UpdateUserAttributesArgs&& aOther) {
    mUserAttributes = std::move(aOther.mUserAttributes);
    return *this;
  }
  
  ~UpdateUserAttributesArgs() {}
};
template<>
bool JSValIs<UpdateUserAttributesArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<UpdateUserAttributesArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateUserAttributesArgs& aRv);
template<>
bool ToJSVal<UpdateUserAttributesArgs>(JSContext* aCx, const UpdateUserAttributesArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationUpdateUserAttributesResult = MozPromise<void*, Failure, true>;
class StaticString20 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"ForeignIdentity"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString20>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString20>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString20& aRv);
template<>
bool ToJSVal<StaticString20>(JSContext* aCx, const StaticString20& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_ForeignIdentity = StaticString20;
template<>
class SafeVariant<JSNull, nsString, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusForeignIdentityField {
  StaticString20 mType;
  BerytusForeignIdentityFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusForeignIdentityField() = default;
  BerytusForeignIdentityField(StaticString20&& aType, BerytusForeignIdentityFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusForeignIdentityField(BerytusForeignIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusForeignIdentityField& operator=(BerytusForeignIdentityField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusForeignIdentityField() {}
};
template<>
bool JSValIs<BerytusForeignIdentityField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusForeignIdentityField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityField& aRv);
template<>
bool ToJSVal<BerytusForeignIdentityField>(JSContext* aCx, const BerytusForeignIdentityField& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString21 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"Identity"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString21>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString21>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString21& aRv);
template<>
bool ToJSVal<StaticString21>(JSContext* aCx, const StaticString21& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_Identity = StaticString21;
struct BerytusIdentityField {
  StaticString21 mType;
  BerytusIdentityFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusIdentityField() = default;
  BerytusIdentityField(StaticString21&& aType, BerytusIdentityFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusIdentityField(BerytusIdentityField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusIdentityField& operator=(BerytusIdentityField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusIdentityField() {}
};
template<>
bool JSValIs<BerytusIdentityField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusIdentityField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityField& aRv);
template<>
bool ToJSVal<BerytusIdentityField>(JSContext* aCx, const BerytusIdentityField& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString22 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"Key"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString22>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString22>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString22& aRv);
template<>
bool ToJSVal<StaticString22>(JSContext* aCx, const StaticString22& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_Key = StaticString22;
template<>
class SafeVariant<ArrayBuffer, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<ArrayBuffer, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<ArrayBuffer, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusKeyFieldValue {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mPublicKey;
  BerytusKeyFieldValue() = default;
  BerytusKeyFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aPublicKey) : mPublicKey(std::move(aPublicKey)) {}
  BerytusKeyFieldValue(BerytusKeyFieldValue&& aOther) : mPublicKey(std::move(aOther.mPublicKey))  {}
  BerytusKeyFieldValue& operator=(BerytusKeyFieldValue&& aOther) {
    mPublicKey = std::move(aOther.mPublicKey);
    return *this;
  }
  
  ~BerytusKeyFieldValue() {}
};
template<>
bool JSValIs<BerytusKeyFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusKeyFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldValue& aRv);
template<>
bool ToJSVal<BerytusKeyFieldValue>(JSContext* aCx, const BerytusKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<JSNull, BerytusKeyFieldValue> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<JSNull, BerytusKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<JSNull, BerytusKeyFieldValue>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<JSNull, BerytusKeyFieldValue>* mVariant;
};
template<>
bool JSValIs<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusKeyFieldValue>& aRv);
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusKeyField {
  StaticString22 mType;
  BerytusKeyFieldOptions mOptions;
  SafeVariant<JSNull, BerytusKeyFieldValue> mValue;
  nsString mId;
  BerytusKeyField() = default;
  BerytusKeyField(StaticString22&& aType, BerytusKeyFieldOptions&& aOptions, SafeVariant<JSNull, BerytusKeyFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusKeyField(BerytusKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusKeyField& operator=(BerytusKeyField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusKeyField() {}
};
template<>
bool JSValIs<BerytusKeyField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusKeyField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyField& aRv);
template<>
bool ToJSVal<BerytusKeyField>(JSContext* aCx, const BerytusKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusPasswordField {
  StaticString14 mType;
  BerytusPasswordFieldOptions mOptions;
  SafeVariant<JSNull, nsString, BerytusEncryptedPacket> mValue;
  nsString mId;
  BerytusPasswordField() = default;
  BerytusPasswordField(StaticString14&& aType, BerytusPasswordFieldOptions&& aOptions, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusPasswordField(BerytusPasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusPasswordField& operator=(BerytusPasswordField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusPasswordField() {}
};
template<>
bool JSValIs<BerytusPasswordField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusPasswordField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordField& aRv);
template<>
bool ToJSVal<BerytusPasswordField>(JSContext* aCx, const BerytusPasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString23 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SecurePassword"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString23>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString23>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString23& aRv);
template<>
bool ToJSVal<StaticString23>(JSContext* aCx, const StaticString23& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SecurePassword = StaticString23;
struct BerytusSecurePasswordFieldValue {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mSalt;
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mVerifier;
  BerytusSecurePasswordFieldValue() = default;
  BerytusSecurePasswordFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aSalt, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aVerifier) : mSalt(std::move(aSalt)), mVerifier(std::move(aVerifier)) {}
  BerytusSecurePasswordFieldValue(BerytusSecurePasswordFieldValue&& aOther) : mSalt(std::move(aOther.mSalt)), mVerifier(std::move(aOther.mVerifier))  {}
  BerytusSecurePasswordFieldValue& operator=(BerytusSecurePasswordFieldValue&& aOther) {
    mSalt = std::move(aOther.mSalt);
  mVerifier = std::move(aOther.mVerifier);
    return *this;
  }
  
  ~BerytusSecurePasswordFieldValue() {}
};
template<>
bool JSValIs<BerytusSecurePasswordFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSecurePasswordFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldValue& aRv);
template<>
bool ToJSVal<BerytusSecurePasswordFieldValue>(JSContext* aCx, const BerytusSecurePasswordFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<JSNull, BerytusSecurePasswordFieldValue> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<JSNull, BerytusSecurePasswordFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<JSNull, BerytusSecurePasswordFieldValue>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<JSNull, BerytusSecurePasswordFieldValue>* mVariant;
};
template<>
bool JSValIs<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aRv);
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSecurePasswordField {
  StaticString23 mType;
  BerytusSecurePasswordFieldOptions mOptions;
  SafeVariant<JSNull, BerytusSecurePasswordFieldValue> mValue;
  nsString mId;
  BerytusSecurePasswordField() = default;
  BerytusSecurePasswordField(StaticString23&& aType, BerytusSecurePasswordFieldOptions&& aOptions, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSecurePasswordField(BerytusSecurePasswordField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusSecurePasswordField& operator=(BerytusSecurePasswordField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusSecurePasswordField() {}
};
template<>
bool JSValIs<BerytusSecurePasswordField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSecurePasswordField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordField& aRv);
template<>
bool ToJSVal<BerytusSecurePasswordField>(JSContext* aCx, const BerytusSecurePasswordField& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString24 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SharedKey"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString24>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString24>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString24& aRv);
template<>
bool ToJSVal<StaticString24>(JSContext* aCx, const StaticString24& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SharedKey = StaticString24;
struct BerytusSharedKeyFieldValue {
  SafeVariant<ArrayBuffer, BerytusEncryptedPacket> mPrivateKey;
  BerytusSharedKeyFieldValue() = default;
  BerytusSharedKeyFieldValue(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>&& aPrivateKey) : mPrivateKey(std::move(aPrivateKey)) {}
  BerytusSharedKeyFieldValue(BerytusSharedKeyFieldValue&& aOther) : mPrivateKey(std::move(aOther.mPrivateKey))  {}
  BerytusSharedKeyFieldValue& operator=(BerytusSharedKeyFieldValue&& aOther) {
    mPrivateKey = std::move(aOther.mPrivateKey);
    return *this;
  }
  
  ~BerytusSharedKeyFieldValue() {}
};
template<>
bool JSValIs<BerytusSharedKeyFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSharedKeyFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldValue& aRv);
template<>
bool ToJSVal<BerytusSharedKeyFieldValue>(JSContext* aCx, const BerytusSharedKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<JSNull, BerytusSharedKeyFieldValue> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<JSNull, BerytusSharedKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<JSNull, BerytusSharedKeyFieldValue>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<JSNull, BerytusSharedKeyFieldValue>* mVariant;
};
template<>
bool JSValIs<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aRv);
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSharedKeyField {
  StaticString24 mType;
  BerytusSharedKeyFieldOptions mOptions;
  SafeVariant<JSNull, BerytusSharedKeyFieldValue> mValue;
  nsString mId;
  BerytusSharedKeyField() = default;
  BerytusSharedKeyField(StaticString24&& aType, BerytusSharedKeyFieldOptions&& aOptions, SafeVariant<JSNull, BerytusSharedKeyFieldValue>&& aValue, nsString&& aId) : mType(std::move(aType)), mOptions(std::move(aOptions)), mValue(std::move(aValue)), mId(std::move(aId)) {}
  BerytusSharedKeyField(BerytusSharedKeyField&& aOther) : mType(std::move(aOther.mType)), mOptions(std::move(aOther.mOptions)), mValue(std::move(aOther.mValue)), mId(std::move(aOther.mId))  {}
  BerytusSharedKeyField& operator=(BerytusSharedKeyField&& aOther) {
    mType = std::move(aOther.mType);
  mOptions = std::move(aOther.mOptions);
  mValue = std::move(aOther.mValue);
  mId = std::move(aOther.mId);
    return *this;
  }
  
  ~BerytusSharedKeyField() {}
};
template<>
bool JSValIs<BerytusSharedKeyField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSharedKeyField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyField& aRv);
template<>
bool ToJSVal<BerytusSharedKeyField>(JSContext* aCx, const BerytusSharedKeyField& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>* mVariant;
};
template<>
bool JSValIs<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aRv);
template<>
bool ToJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext* aCx, const SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AddFieldArgs {
  SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField> mField;
  AddFieldArgs() = default;
  AddFieldArgs(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>&& aField) : mField(std::move(aField)) {}
  AddFieldArgs(AddFieldArgs&& aOther) : mField(std::move(aOther.mField))  {}
  AddFieldArgs& operator=(AddFieldArgs&& aOther) {
    mField = std::move(aOther.mField);
    return *this;
  }
  
  ~AddFieldArgs() {}
};
template<>
bool JSValIs<AddFieldArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<AddFieldArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv);
template<>
bool ToJSVal<AddFieldArgs>(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>* mVariant;
};
template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv);
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> const* InternalValue() const { return mVariant; }
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>* mVariant;
};
template<>
bool JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv);
template<>
bool ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationAddFieldResult = MozPromise<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>, Failure, true>;
struct FieldValueRejectionReason {
  nsString mCode;
  FieldValueRejectionReason() = default;
  FieldValueRejectionReason(nsString&& aCode) : mCode(std::move(aCode)) {}
  FieldValueRejectionReason(FieldValueRejectionReason&& aOther) : mCode(std::move(aOther.mCode))  {}
  FieldValueRejectionReason& operator=(FieldValueRejectionReason&& aOther) {
    mCode = std::move(aOther.mCode);
    return *this;
  }
  
  ~FieldValueRejectionReason() {}
};
template<>
bool JSValIs<FieldValueRejectionReason>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<FieldValueRejectionReason>(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv);
template<>
bool ToJSVal<FieldValueRejectionReason>(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>* mVariant;
};
template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aRv);
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct RejectFieldValueArgs {
  nsString mFieldId;
  FieldValueRejectionReason mReason;
  SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing> mOptionalNewValue;
  RejectFieldValueArgs() = default;
  RejectFieldValueArgs(nsString&& aFieldId, FieldValueRejectionReason&& aReason, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>&& aOptionalNewValue) : mFieldId(std::move(aFieldId)), mReason(std::move(aReason)), mOptionalNewValue(std::move(aOptionalNewValue)) {}
  RejectFieldValueArgs(RejectFieldValueArgs&& aOther) : mFieldId(std::move(aOther.mFieldId)), mReason(std::move(aOther.mReason)), mOptionalNewValue(std::move(aOther.mOptionalNewValue))  {}
  RejectFieldValueArgs& operator=(RejectFieldValueArgs&& aOther) {
    mFieldId = std::move(aOther.mFieldId);
  mReason = std::move(aOther.mReason);
  mOptionalNewValue = std::move(aOther.mOptionalNewValue);
    return *this;
  }
  
  ~RejectFieldValueArgs() {}
};
template<>
bool JSValIs<RejectFieldValueArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<RejectFieldValueArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv);
template<>
bool ToJSVal<RejectFieldValueArgs>(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationRejectFieldValueResult = MozPromise<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>, Failure, true>;
struct ApproveChallengeRequestArgs {
  SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo> mChallenge;
  ApproveChallengeRequestArgs() = default;
  ApproveChallengeRequestArgs(SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  ApproveChallengeRequestArgs(ApproveChallengeRequestArgs&& aOther) : mChallenge(std::move(aOther.mChallenge))  {}
  ApproveChallengeRequestArgs& operator=(ApproveChallengeRequestArgs&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
    return *this;
  }
  
  ~ApproveChallengeRequestArgs() {}
};
template<>
bool JSValIs<ApproveChallengeRequestArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<ApproveChallengeRequestArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv);
template<>
bool ToJSVal<ApproveChallengeRequestArgs>(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountAuthenticationApproveChallengeRequestResult = MozPromise<void*, Failure, true>;
class StaticString25 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"GenericWebAppFailure"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString25>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString25>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString25& aRv);
template<>
bool ToJSVal<StaticString25>(JSContext* aCx, const StaticString25& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_GenericWebAppFailure = StaticString25;
class StaticString26 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"UserInterrupt"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString26>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString26>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString26& aRv);
template<>
bool ToJSVal<StaticString26>(JSContext* aCx, const StaticString26& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_UserInterrupt = StaticString26;
class StaticString27 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"IdentityDoesNotExists"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString27>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString27>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString27& aRv);
template<>
bool ToJSVal<StaticString27>(JSContext* aCx, const StaticString27& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_IdentityDoesNotExists = StaticString27;
class StaticString28 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"IncorrectPassword"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString28>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString28>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString28& aRv);
template<>
bool ToJSVal<StaticString28>(JSContext* aCx, const StaticString28& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_IncorrectPassword = StaticString28;
class StaticString29 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"InvalidProof"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString29>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString29>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString29& aRv);
template<>
bool ToJSVal<StaticString29>(JSContext* aCx, const StaticString29& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_InvalidProof = StaticString29;
class StaticString30 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"InvalidSignature"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString30>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString30>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString30& aRv);
template<>
bool ToJSVal<StaticString30>(JSContext* aCx, const StaticString30& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_InvalidSignature = StaticString30;
class StaticString31 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"IncorrectOtp"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString31>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString31>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString31& aRv);
template<>
bool ToJSVal<StaticString31>(JSContext* aCx, const StaticString31& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_IncorrectOtp = StaticString31;
template<>
class SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31> const* InternalValue() const { return mVariant; }
  mozilla::Variant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>* InternalValue() { return mVariant; }
  
  nsString AsString() const {
    MOZ_ASSERT(mVariant);
    return mVariant->match(
        [](StaticString25& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString26& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString27& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString28& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString29& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString30& aStr) -> nsString {
          return aStr.GetString();
        },
    [](StaticString31& aStr) -> nsString {
          return aStr.GetString();
        }
    );
  }
protected:
  mozilla::Variant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>* mVariant;
};
template<>
bool JSValIs<SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>& aRv);
template<>
bool ToJSVal<SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>>(JSContext* aCx, const SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AbortChallengeArgs {
  SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo> mChallenge;
  SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31> mReason;
  AbortChallengeArgs() = default;
  AbortChallengeArgs(SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>&& aChallenge, SafeVariant<StaticString25, StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31>&& aReason) : mChallenge(std::move(aChallenge)), mReason(std::move(aReason)) {}
  AbortChallengeArgs(AbortChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge)), mReason(std::move(aOther.mReason))  {}
  AbortChallengeArgs& operator=(AbortChallengeArgs&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mReason = std::move(aOther.mReason);
    return *this;
  }
  
  ~AbortChallengeArgs() {}
};
template<>
bool JSValIs<AbortChallengeArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<AbortChallengeArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv);
template<>
bool ToJSVal<AbortChallengeArgs>(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountAuthenticationAbortChallengeResult = MozPromise<void*, Failure, true>;
struct CloseChallengeArgs {
  SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo> mChallenge;
  CloseChallengeArgs() = default;
  CloseChallengeArgs(SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>&& aChallenge) : mChallenge(std::move(aChallenge)) {}
  CloseChallengeArgs(CloseChallengeArgs&& aOther) : mChallenge(std::move(aOther.mChallenge))  {}
  CloseChallengeArgs& operator=(CloseChallengeArgs&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
    return *this;
  }
  
  ~CloseChallengeArgs() {}
};
template<>
bool JSValIs<CloseChallengeArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<CloseChallengeArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv);
template<>
bool ToJSVal<CloseChallengeArgs>(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountAuthenticationCloseChallengeResult = MozPromise<void*, Failure, true>;
class StaticString32 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"GetIdentityFields"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString32>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString32>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString32& aRv);
template<>
bool ToJSVal<StaticString32>(JSContext* aCx, const StaticString32& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_GetIdentityFields = StaticString32;
struct BerytusSendGetIdentityFieldsMessage {
  BerytusIdentificationChallengeInfo mChallenge;
  StaticString32 mName;
  nsTArray<nsString> mPayload;
  BerytusSendGetIdentityFieldsMessage() = default;
  BerytusSendGetIdentityFieldsMessage(BerytusIdentificationChallengeInfo&& aChallenge, StaticString32&& aName, nsTArray<nsString>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendGetIdentityFieldsMessage(BerytusSendGetIdentityFieldsMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendGetIdentityFieldsMessage& operator=(BerytusSendGetIdentityFieldsMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendGetIdentityFieldsMessage() {}
};
template<>
bool JSValIs<BerytusSendGetIdentityFieldsMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendGetIdentityFieldsMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetIdentityFieldsMessage& aRv);
template<>
bool ToJSVal<BerytusSendGetIdentityFieldsMessage>(JSContext* aCx, const BerytusSendGetIdentityFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString33 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"GetPasswordFields"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString33>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString33>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString33& aRv);
template<>
bool ToJSVal<StaticString33>(JSContext* aCx, const StaticString33& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_GetPasswordFields = StaticString33;
struct BerytusSendGetPasswordFieldsMessage {
  BerytusPasswordChallengeInfo mChallenge;
  StaticString33 mName;
  nsTArray<nsString> mPayload;
  BerytusSendGetPasswordFieldsMessage() = default;
  BerytusSendGetPasswordFieldsMessage(BerytusPasswordChallengeInfo&& aChallenge, StaticString33&& aName, nsTArray<nsString>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendGetPasswordFieldsMessage(BerytusSendGetPasswordFieldsMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendGetPasswordFieldsMessage& operator=(BerytusSendGetPasswordFieldsMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendGetPasswordFieldsMessage() {}
};
template<>
bool JSValIs<BerytusSendGetPasswordFieldsMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendGetPasswordFieldsMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetPasswordFieldsMessage& aRv);
template<>
bool ToJSVal<BerytusSendGetPasswordFieldsMessage>(JSContext* aCx, const BerytusSendGetPasswordFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString34 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SelectKey"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString34>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString34>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString34& aRv);
template<>
bool ToJSVal<StaticString34>(JSContext* aCx, const StaticString34& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SelectKey = StaticString34;
struct BerytusSendSelectKeyMessage {
  BerytusDigitalSignatureChallengeInfo mChallenge;
  StaticString34 mName;
  nsString mPayload;
  BerytusSendSelectKeyMessage() = default;
  BerytusSendSelectKeyMessage(BerytusDigitalSignatureChallengeInfo&& aChallenge, StaticString34&& aName, nsString&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSelectKeyMessage(BerytusSendSelectKeyMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendSelectKeyMessage& operator=(BerytusSendSelectKeyMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendSelectKeyMessage() {}
};
template<>
bool JSValIs<BerytusSendSelectKeyMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendSelectKeyMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectKeyMessage& aRv);
template<>
bool ToJSVal<BerytusSendSelectKeyMessage>(JSContext* aCx, const BerytusSendSelectKeyMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString35 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SignNonce"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString35>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString35>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString35& aRv);
template<>
bool ToJSVal<StaticString35>(JSContext* aCx, const StaticString35& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SignNonce = StaticString35;
template<>
class SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> const* InternalValue() const { return mVariant; }
  mozilla::Variant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusSendSignNonceMessage {
  BerytusDigitalSignatureChallengeInfo mChallenge;
  StaticString35 mName;
  SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendSignNonceMessage() = default;
  BerytusSendSignNonceMessage(BerytusDigitalSignatureChallengeInfo&& aChallenge, StaticString35&& aName, SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSignNonceMessage(BerytusSendSignNonceMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendSignNonceMessage& operator=(BerytusSendSignNonceMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendSignNonceMessage() {}
};
template<>
bool JSValIs<BerytusSendSignNonceMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendSignNonceMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSignNonceMessage& aRv);
template<>
bool ToJSVal<BerytusSendSignNonceMessage>(JSContext* aCx, const BerytusSendSignNonceMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString36 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"SelectSecurePassword"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString36>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString36>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString36& aRv);
template<>
bool ToJSVal<StaticString36>(JSContext* aCx, const StaticString36& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_SelectSecurePassword = StaticString36;
struct BerytusSendSelectSecurePasswordMessage {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  StaticString36 mName;
  nsString mPayload;
  BerytusSendSelectSecurePasswordMessage() = default;
  BerytusSendSelectSecurePasswordMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, StaticString36&& aName, nsString&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendSelectSecurePasswordMessage(BerytusSendSelectSecurePasswordMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendSelectSecurePasswordMessage& operator=(BerytusSendSelectSecurePasswordMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendSelectSecurePasswordMessage() {}
};
template<>
bool JSValIs<BerytusSendSelectSecurePasswordMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendSelectSecurePasswordMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectSecurePasswordMessage& aRv);
template<>
bool ToJSVal<BerytusSendSelectSecurePasswordMessage>(JSContext* aCx, const BerytusSendSelectSecurePasswordMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString37 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"ExchangePublicKeys"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString37>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString37>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString37& aRv);
template<>
bool ToJSVal<StaticString37>(JSContext* aCx, const StaticString37& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_ExchangePublicKeys = StaticString37;
struct BerytusSendExchangePublicKeysMessage {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  StaticString37 mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendExchangePublicKeysMessage() = default;
  BerytusSendExchangePublicKeysMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, StaticString37&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendExchangePublicKeysMessage(BerytusSendExchangePublicKeysMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendExchangePublicKeysMessage& operator=(BerytusSendExchangePublicKeysMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendExchangePublicKeysMessage() {}
};
template<>
bool JSValIs<BerytusSendExchangePublicKeysMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendExchangePublicKeysMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendExchangePublicKeysMessage& aRv);
template<>
bool ToJSVal<BerytusSendExchangePublicKeysMessage>(JSContext* aCx, const BerytusSendExchangePublicKeysMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString38 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"ComputeClientProof"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString38>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString38>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString38& aRv);
template<>
bool ToJSVal<StaticString38>(JSContext* aCx, const StaticString38& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_ComputeClientProof = StaticString38;
struct BerytusSendComputeClientProofMessage {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  StaticString38 mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendComputeClientProofMessage() = default;
  BerytusSendComputeClientProofMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, StaticString38&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendComputeClientProofMessage(BerytusSendComputeClientProofMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendComputeClientProofMessage& operator=(BerytusSendComputeClientProofMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendComputeClientProofMessage() {}
};
template<>
bool JSValIs<BerytusSendComputeClientProofMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendComputeClientProofMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendComputeClientProofMessage& aRv);
template<>
bool ToJSVal<BerytusSendComputeClientProofMessage>(JSContext* aCx, const BerytusSendComputeClientProofMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString39 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"VerifyServerProof"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString39>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString39>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString39& aRv);
template<>
bool ToJSVal<StaticString39>(JSContext* aCx, const StaticString39& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_VerifyServerProof = StaticString39;
struct BerytusSendVerifyServerProofMessage {
  BerytusSecureRemotePasswordChallengeInfo mChallenge;
  StaticString39 mName;
  SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket> mPayload;
  BerytusSendVerifyServerProofMessage() = default;
  BerytusSendVerifyServerProofMessage(BerytusSecureRemotePasswordChallengeInfo&& aChallenge, StaticString39&& aName, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendVerifyServerProofMessage(BerytusSendVerifyServerProofMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendVerifyServerProofMessage& operator=(BerytusSendVerifyServerProofMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendVerifyServerProofMessage() {}
};
template<>
bool JSValIs<BerytusSendVerifyServerProofMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendVerifyServerProofMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendVerifyServerProofMessage& aRv);
template<>
bool ToJSVal<BerytusSendVerifyServerProofMessage>(JSContext* aCx, const BerytusSendVerifyServerProofMessage& aValue, JS::MutableHandle<JS::Value> aRv);
class StaticString40 : public StaticStringBase {
public:
  constexpr static const nsLiteralString mLiteral =
      u"GetOtp"_ns;
  const nsLiteralString& GetString() const override {
    return mLiteral;
  }
};
template<>
bool JSValIs<StaticString40>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<StaticString40>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString40& aRv);
template<>
bool ToJSVal<StaticString40>(JSContext* aCx, const StaticString40& aValue, JS::MutableHandle<JS::Value> aRv);

using StaticString_GetOtp = StaticString40;
struct BerytusSendGetOtpMessage {
  BerytusOffChannelOtpChallengeInfo mChallenge;
  StaticString40 mName;
  nsString mPayload;
  BerytusSendGetOtpMessage() = default;
  BerytusSendGetOtpMessage(BerytusOffChannelOtpChallengeInfo&& aChallenge, StaticString40&& aName, nsString&& aPayload) : mChallenge(std::move(aChallenge)), mName(std::move(aName)), mPayload(std::move(aPayload)) {}
  BerytusSendGetOtpMessage(BerytusSendGetOtpMessage&& aOther) : mChallenge(std::move(aOther.mChallenge)), mName(std::move(aOther.mName)), mPayload(std::move(aOther.mPayload))  {}
  BerytusSendGetOtpMessage& operator=(BerytusSendGetOtpMessage&& aOther) {
    mChallenge = std::move(aOther.mChallenge);
  mName = std::move(aOther.mName);
  mPayload = std::move(aOther.mPayload);
    return *this;
  }
  
  ~BerytusSendGetOtpMessage() {}
};
template<>
bool JSValIs<BerytusSendGetOtpMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusSendGetOtpMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetOtpMessage& aRv);
template<>
bool ToJSVal<BerytusSendGetOtpMessage>(JSContext* aCx, const BerytusSendGetOtpMessage& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage> const* InternalValue() const { return mVariant; }
  mozilla::Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>* mVariant;
};
template<>
bool JSValIs<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aRv);
template<>
bool ToJSVal<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext* aCx, const SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<nsString, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<nsString, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<nsString, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<nsString, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
bool JSValIs<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>& aRv);
template<>
bool ToJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext* aCx, const Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeGetIdentityFieldsMessageResponse {
  Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>> mResponse;
  BerytusChallengeGetIdentityFieldsMessageResponse() = default;
  BerytusChallengeGetIdentityFieldsMessageResponse(Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeGetIdentityFieldsMessageResponse(BerytusChallengeGetIdentityFieldsMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  
  ~BerytusChallengeGetIdentityFieldsMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetIdentityFieldsMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext* aCx, const BerytusChallengeGetIdentityFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeGetPasswordFieldsMessageResponse {
  Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>> mResponse;
  BerytusChallengeGetPasswordFieldsMessageResponse() = default;
  BerytusChallengeGetPasswordFieldsMessageResponse(Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeGetPasswordFieldsMessageResponse(BerytusChallengeGetPasswordFieldsMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  
  ~BerytusChallengeGetPasswordFieldsMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetPasswordFieldsMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext* aCx, const BerytusChallengeGetPasswordFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeSelectKeyMessageResponse {
  BerytusKeyFieldValue mResponse;
  BerytusChallengeSelectKeyMessageResponse() = default;
  BerytusChallengeSelectKeyMessageResponse(BerytusKeyFieldValue&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSelectKeyMessageResponse(BerytusChallengeSelectKeyMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  BerytusChallengeSelectKeyMessageResponse& operator=(BerytusChallengeSelectKeyMessageResponse&& aOther) {
    mResponse = std::move(aOther.mResponse);
    return *this;
  }
  
  ~BerytusChallengeSelectKeyMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeSelectKeyMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeSelectKeyMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectKeyMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeSelectKeyMessageResponse>(JSContext* aCx, const BerytusChallengeSelectKeyMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeSignNonceMessageResponse {
  ArrayBuffer mResponse;
  BerytusChallengeSignNonceMessageResponse() = default;
  BerytusChallengeSignNonceMessageResponse(ArrayBuffer&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSignNonceMessageResponse(BerytusChallengeSignNonceMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  
  
  ~BerytusChallengeSignNonceMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeSignNonceMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeSignNonceMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSignNonceMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeSignNonceMessageResponse>(JSContext* aCx, const BerytusChallengeSignNonceMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeSelectSecurePasswordMessageResponse {
  SafeVariant<nsString, BerytusEncryptedPacket> mResponse;
  BerytusChallengeSelectSecurePasswordMessageResponse() = default;
  BerytusChallengeSelectSecurePasswordMessageResponse(SafeVariant<nsString, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeSelectSecurePasswordMessageResponse(BerytusChallengeSelectSecurePasswordMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  BerytusChallengeSelectSecurePasswordMessageResponse& operator=(BerytusChallengeSelectSecurePasswordMessageResponse&& aOther) {
    mResponse = std::move(aOther.mResponse);
    return *this;
  }
  
  ~BerytusChallengeSelectSecurePasswordMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectSecurePasswordMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext* aCx, const BerytusChallengeSelectSecurePasswordMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
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
  mozilla::Variant<nsString, ArrayBuffer, BerytusEncryptedPacket>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<nsString, ArrayBuffer, BerytusEncryptedPacket>* mVariant;
};
template<>
bool JSValIs<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aRv);
template<>
bool ToJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeExchangePublicKeysMessageResponse {
  SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> mResponse;
  BerytusChallengeExchangePublicKeysMessageResponse() = default;
  BerytusChallengeExchangePublicKeysMessageResponse(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeExchangePublicKeysMessageResponse(BerytusChallengeExchangePublicKeysMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  BerytusChallengeExchangePublicKeysMessageResponse& operator=(BerytusChallengeExchangePublicKeysMessageResponse&& aOther) {
    mResponse = std::move(aOther.mResponse);
    return *this;
  }
  
  ~BerytusChallengeExchangePublicKeysMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeExchangePublicKeysMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext* aCx, const BerytusChallengeExchangePublicKeysMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeComputeClientProofMessageResponse {
  SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket> mResponse;
  BerytusChallengeComputeClientProofMessageResponse() = default;
  BerytusChallengeComputeClientProofMessageResponse(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeComputeClientProofMessageResponse(BerytusChallengeComputeClientProofMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  BerytusChallengeComputeClientProofMessageResponse& operator=(BerytusChallengeComputeClientProofMessageResponse&& aOther) {
    mResponse = std::move(aOther.mResponse);
    return *this;
  }
  
  ~BerytusChallengeComputeClientProofMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeComputeClientProofMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeComputeClientProofMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeComputeClientProofMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeComputeClientProofMessageResponse>(JSContext* aCx, const BerytusChallengeComputeClientProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeVerifyServerProofMessageResponse {
  
  BerytusChallengeVerifyServerProofMessageResponse() = default;
  
  ~BerytusChallengeVerifyServerProofMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeVerifyServerProofMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeVerifyServerProofMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeVerifyServerProofMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeVerifyServerProofMessageResponse>(JSContext* aCx, const BerytusChallengeVerifyServerProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
struct BerytusChallengeGetOtpMessageResponse {
  SafeVariant<nsString, BerytusEncryptedPacket> mResponse;
  BerytusChallengeGetOtpMessageResponse() = default;
  BerytusChallengeGetOtpMessageResponse(SafeVariant<nsString, BerytusEncryptedPacket>&& aResponse) : mResponse(std::move(aResponse)) {}
  BerytusChallengeGetOtpMessageResponse(BerytusChallengeGetOtpMessageResponse&& aOther) : mResponse(std::move(aOther.mResponse))  {}
  BerytusChallengeGetOtpMessageResponse& operator=(BerytusChallengeGetOtpMessageResponse&& aOther) {
    mResponse = std::move(aOther.mResponse);
    return *this;
  }
  
  ~BerytusChallengeGetOtpMessageResponse() {}
};
template<>
bool JSValIs<BerytusChallengeGetOtpMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<BerytusChallengeGetOtpMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetOtpMessageResponse& aRv);
template<>
bool ToJSVal<BerytusChallengeGetOtpMessageResponse>(JSContext* aCx, const BerytusChallengeGetOtpMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);
template<>
class SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse> {
public:
  SafeVariant() : mVariant(nullptr) {}
  SafeVariant(SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>&& aOther) : mVariant(std::move(aOther.mVariant)) {
    aOther.mVariant = nullptr;
  }
  SafeVariant& operator=(SafeVariant&& aOther) {
    mVariant = std::move(aOther.mVariant);
    aOther.mVariant = nullptr;
    return *this;
  }
  ~SafeVariant() {
    delete mVariant;
  };
  template <typename... Args>
  void Init(Args&&... aTs) {
    MOZ_ASSERT(!mVariant);
    mVariant = new Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>(std::forward<Args>(aTs)...);
  }
  bool Inited() const {
    return mVariant;
  }
  mozilla::Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse> const* InternalValue() const { return mVariant; }
  mozilla::Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>* InternalValue() { return mVariant; }
  
protected:
  mozilla::Variant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>* mVariant;
};
template<>
bool JSValIs<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
template<>
bool FromJSVal<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>& aRv);
template<>
bool ToJSVal<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext* aCx, const SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountAuthenticationRespondToChallengeMessageResult = MozPromise<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>, Failure, true>;

class AgentProxy : public nsISupports {
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(AgentProxy)

public:
  AgentProxy(nsIGlobalObject* aGlobal, const nsAString& aManagerId);
  bool IsDisabled() const;

  template <typename W1, typename W2>
  already_AddRefed<dom::Promise> CallSendQuery(JSContext *aCx,
                                               const nsAString & aGroup,
                                               const nsAString &aMethod,
                                               const W1& aReqCx,
                                               const W2* aReqArgs,
                                               ErrorResult& aRv);
  template <typename W1>
  already_AddRefed<dom::Promise> CallSendQuery(JSContext *aCx,
                                               const nsAString & aGroup,
                                               const nsAString &aMethod,
                                               const W1& aReqCx,
                                               JS::Handle<JS::Value> aReqArgsJs,
                                               ErrorResult& aRv);

protected:
  virtual ~AgentProxy();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsString mManagerId;
  bool mDisabled;

public:
  RefPtr<ManagerGetSigningKeyResult> Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs);
  RefPtr<ManagerGetCredentialsMetadataResult> Manager_GetCredentialsMetadata(PreliminaryRequestContext& aContext, GetCredentialsMetadataArgs& aArgs);
  RefPtr<ChannelGenerateKeyExchangeParametersResult> Channel_GenerateKeyExchangeParameters(RequestContext& aContext, GenerateKeyExchangeParametersArgs& aArgs);
  RefPtr<ChannelEnableEndToEndEncryptionResult> Channel_EnableEndToEndEncryption(RequestContext& aContext, EnableEndToEndEncryptionArgs& aArgs);
  RefPtr<ChannelCloseChannelResult> Channel_CloseChannel(RequestContext& aContext);
  RefPtr<LoginApproveOperationResult> Login_ApproveOperation(RequestContext& aContext, ApproveOperationArgs& aArgs);
  RefPtr<LoginCloseOperationResult> Login_CloseOperation(RequestContextWithOperation& aContext);
  RefPtr<LoginGetRecordMetadataResult> Login_GetRecordMetadata(RequestContextWithOperation& aContext);
  RefPtr<LoginUpdateMetadataResult> Login_UpdateMetadata(RequestContextWithOperation& aContext, UpdateMetadataArgs& aArgs);
  RefPtr<AccountCreationApproveTransitionToAuthOpResult> AccountCreation_ApproveTransitionToAuthOp(RequestContextWithOperation& aContext, ApproveTransitionToAuthOpArgs& aArgs);
  RefPtr<AccountCreationGetUserAttributesResult> AccountCreation_GetUserAttributes(RequestContextWithLoginOperation& aContext);
  RefPtr<AccountCreationUpdateUserAttributesResult> AccountCreation_UpdateUserAttributes(RequestContextWithOperation& aContext, UpdateUserAttributesArgs& aArgs);
  RefPtr<AccountCreationAddFieldResult> AccountCreation_AddField(RequestContextWithLoginOperation& aContext, AddFieldArgs& aArgs);
  RefPtr<AccountCreationRejectFieldValueResult> AccountCreation_RejectFieldValue(RequestContextWithLoginOperation& aContext, RejectFieldValueArgs& aArgs);
  RefPtr<AccountAuthenticationApproveChallengeRequestResult> AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs);
  RefPtr<AccountAuthenticationAbortChallengeResult> AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs);
  RefPtr<AccountAuthenticationCloseChallengeResult> AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs);
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AccountAuthentication_RespondToChallengeMessage(RequestContextWithLoginOperation& aContext, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aArgs);

};

class OwnedAgentProxy final : public AgentProxy {
public:
    NS_DECL_ISUPPORTS_INHERITED

    OwnedAgentProxy(nsIGlobalObject* aGlobal, const nsAString& aManagerId);

    void Disable();
protected:
    ~OwnedAgentProxy();
};

// based on dom::MozPromiseRejectOnDestruction in PromiseNativeHandler.h
// we just reject with a berytus::Failure instead of an nsresult.
template <typename T>
class MozPromiseRejectWithBerytusFailureOnDestruction final
    : public dom::MozPromiseRejectOnDestructionBase {
 public:
  MozPromiseRejectWithBerytusFailureOnDestruction(const RefPtr<T>& aMozPromise,
                                StaticString aCallSite)
      : mMozPromise(aMozPromise), mCallSite(aCallSite) {
    MOZ_ASSERT(aMozPromise);
  }

 protected:
  ~MozPromiseRejectWithBerytusFailureOnDestruction() override {
    // Rejecting will be no-op if the promise is already settled
    mMozPromise->Reject(berytus::Failure(NS_BINDING_ABORTED), mCallSite);
  }

  RefPtr<T> mMozPromise;
  StaticString mCallSite;
};

}  // namespace mozilla::berytus

#endif