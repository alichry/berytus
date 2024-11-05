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
bool JSValIsNumber(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NumberFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv);
bool NumberToJSVal(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv);
struct DocumentMetadata : IJSWord<DocumentMetadata> {
  double mId;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct PreliminaryRequestContext : IJSWord<PreliminaryRequestContext> {
  DocumentMetadata mDocument;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv);
  static bool ToJSVal(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsString(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool StringFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv);
bool StringToJSVal(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv);
struct CryptoActor : IJSWord<CryptoActor> {
  nsString mEd25519Key;
  
  
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
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv);
  static bool ToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct OriginActor : IJSWord<OriginActor> {
  UriParams mOriginalUri;
  UriParams mCurrentUri;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv);
  static bool ToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_CryptoActor__OriginActor_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_CryptoActor__OriginActor_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<CryptoActor, OriginActor>** aRv);
bool Variant_CryptoActor__OriginActor_ToJSVal(JSContext* aCx, const Variant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetSigningKeyArgs : IJSWord<GetSigningKeyArgs> {
  
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  
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
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv);
  static bool ToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GetCredentialsMetadataArgs : IJSWord<GetCredentialsMetadataArgs> {
  ChannelConstraints mChannelConstraints;
  AccountConstraints mAccountConstraints;
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  
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
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct InitialKeyExchangeParametersDraft : IJSWord<InitialKeyExchangeParametersDraft> {
  nsString mChannelId;
  nsString mWebAppX25519Key;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv);
  static bool ToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct GenerateKeyExchangeParametersArgs : IJSWord<GenerateKeyExchangeParametersArgs> {
  InitialKeyExchangeParametersDraft mParamsDraft;
  
  
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
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct EnableEndToEndEncryptionArgs : IJSWord<EnableEndToEndEncryptionArgs> {
  KeyExchangeParameters mParams;
  ArrayBuffer mWebAppPacketSignature;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using ChannelEnableEndToEndEncryptionResult = MozPromise<ArrayBuffer, Failure, true>;
using ChannelCloseChannelResult = MozPromise<void*, Failure, true>;
enum ELoginUserIntent {
  ELoginUserIntent_PendingDeclaration = 0,
    ELoginUserIntent_Authenticate = 1,
    ELoginUserIntent_Register = 2,
  ELoginUserIntent_EndGuard_
};
bool JSValIsELoginUserIntent(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ELoginUserIntentFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv);
bool ELoginUserIntentToJSVal(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_nsString_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString>** aRv);
bool Variant_nsString_ToJSVal(JSContext* aCx, const Variant<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestedUserAttribute : IJSWord<RequestedUserAttribute> {
  bool mRequired;
  Variant<nsString>* mId = nullptr;
  
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
enum EOpeationType {
  EOpeationType_PendingDeclaration = 0,
    EOpeationType_Registration = 1,
    EOpeationType_Authentication = 2,
  EOpeationType_EndGuard_
};
bool JSValIsEOpeationType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EOpeationTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOpeationType& aRv);
bool EOpeationTypeToJSVal(JSContext* aCx, const EOpeationType& aValue, JS::MutableHandle<JS::Value> aRv);
enum EOperationStatus {
  EOperationStatus_Pending = 0,
    EOperationStatus_Created = 1,
    EOperationStatus_Aborted = 2,
    EOperationStatus_Finished = 4,
  EOperationStatus_EndGuard_
};
bool JSValIsEOperationStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EOperationStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv);
bool EOperationStatusToJSVal(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv);
struct OperationState : IJSWord<OperationState> {
  
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv);
  static bool ToJSVal(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct LoginOperationMetadata : IJSWord<LoginOperationMetadata> {
  ELoginUserIntent mIntent;
  nsTArray<RequestedUserAttribute> mRequestedUserAttributes;
  nsString mId;
  EOpeationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveOperationArgs : IJSWord<ApproveOperationArgs> {
  LoginOperationMetadata mOperation;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginApproveOperationResult = MozPromise<ELoginUserIntent, Failure, true>;
struct OperationMetadata : IJSWord<OperationMetadata> {
  nsString mId;
  EOpeationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RequestContextWithOperation : IJSWord<RequestContextWithOperation> {
  OperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv);
  static bool ToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginCloseOpeationResult = MozPromise<void*, Failure, true>;
enum EMetadataStatus {
  EMetadataStatus_Pending = 0,
    EMetadataStatus_Created = 1,
    EMetadataStatus_Retired = 2,
  EMetadataStatus_EndGuard_
};
bool JSValIsEMetadataStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EMetadataStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv);
bool EMetadataStatusToJSVal(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv);
struct RecordMetadata : IJSWord<RecordMetadata> {
  double mVersion;
  EMetadataStatus mStatus;
  nsString mCategory;
  nsString mChangePassUrl;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginGetRecordMetadataResult = MozPromise<RecordMetadata, Failure, true>;
struct UpdateMetadataArgs : IJSWord<UpdateMetadataArgs> {
  RecordMetadata mMetadata;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using LoginUpdateMetadataResult = MozPromise<void*, Failure, true>;
struct ApproveTransitionToAuthOpArgs : IJSWord<ApproveTransitionToAuthOpArgs> {
  LoginOperationMetadata mNewAuthOp;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationApproveTransitionToAuthOpResult = MozPromise<void*, Failure, true>;
struct UserAttribute : IJSWord<UserAttribute> {
  nsString mMimeType;
  nsString mValue;
  Variant<nsString>* mId = nullptr;
  
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
enum EFieldType {
  EFieldType_Identity = 0,
    EFieldType_ForeignIdentity = 1,
    EFieldType_Password = 2,
    EFieldType_SecurePassword = 3,
    EFieldType_Key = 4,
  EFieldType_EndGuard_
};
bool JSValIsEFieldType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EFieldTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EFieldType& aRv);
bool EFieldTypeToJSVal(JSContext* aCx, const EFieldType& aValue, JS::MutableHandle<JS::Value> aRv);
struct BaseFieldMetadata : IJSWord<BaseFieldMetadata> {
  EFieldType mFieldType;
  nsString mFieldId;
  Maybe<nsString> mDescription;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BaseFieldMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const BaseFieldMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsNothing(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NothingFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
bool NothingToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_nsString__ArrayBuffer__Nothing_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer__Nothing_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer, Nothing>** aRv);
bool Variant_nsString__ArrayBuffer__Nothing_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AddFieldArgs : IJSWord<AddFieldArgs> {
  BaseFieldMetadata mField;
  Variant<nsString, ArrayBuffer, Nothing>* mValue = nullptr;
  
  ~AddFieldArgs() {
    delete mValue;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountCreationAddFieldResult = MozPromise<void*, Failure, true>;
struct FieldValueRejectionReason : IJSWord<FieldValueRejectionReason> {
  nsString mCode;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RejectFieldValueArgs : IJSWord<RejectFieldValueArgs> {
  BaseFieldMetadata mField;
  FieldValueRejectionReason mReason;
  Variant<nsString, ArrayBuffer, Nothing>* mOptionalNewValue = nullptr;
  
  ~RejectFieldValueArgs() {
    delete mOptionalNewValue;
  }
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
bool JSValIsVariant_nsString__ArrayBuffer_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer>** aRv);
bool Variant_nsString__ArrayBuffer_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer>& aValue, JS::MutableHandle<JS::Value> aRv);
using AccountCreationRejectFieldValueResult = MozPromise<Variant<nsString, ArrayBuffer>*, Failure, true>;
enum EChallengeType {
  EChallengeType_Identification = 0,
    EChallengeType_DigitalSignature = 1,
    EChallengeType_Password = 2,
    EChallengeType_SecureRemotePassword = 3,
    EChallengeType_ForeignIdentityOtp = 4,
  EChallengeType_EndGuard_
};
bool JSValIsEChallengeType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EChallengeTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeType& aRv);
bool EChallengeTypeToJSVal(JSContext* aCx, const EChallengeType& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengeParameters : IJSWord<ChallengeParameters> {
  
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeParameters& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
};
enum EChallengeStatus {
  EChallengeStatus_Invalid = 0,
    EChallengeStatus_Pending = 1,
    EChallengeStatus_Active = 2,
    EChallengeStatus_Aborted = 3,
    EChallengeStatus_Sealed = 4,
  EChallengeStatus_EndGuard_
};
bool JSValIsEChallengeStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EChallengeStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeStatus& aRv);
bool EChallengeStatusToJSVal(JSContext* aCx, const EChallengeStatus& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengeMetadata : IJSWord<ChallengeMetadata> {
  nsString mId;
  EChallengeType mType;
  ChallengeParameters mParameters;
  EChallengeStatus mStatus;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMetadata& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ApproveChallengeRequestArgs : IJSWord<ApproveChallengeRequestArgs> {
  ChallengeMetadata mChallenge;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationApproveChallengeRequestResult = MozPromise<void*, Failure, true>;
struct ChallengeAbortionReason : IJSWord<ChallengeAbortionReason> {
  nsString mCode;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeAbortionReason& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct AbortChallengeArgs : IJSWord<AbortChallengeArgs> {
  ChallengeMetadata mChallenge;
  ChallengeAbortionReason mReason;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationAbortChallengeResult = MozPromise<void*, Failure, true>;
struct CloseChallengeArgs : IJSWord<CloseChallengeArgs> {
  ChallengeMetadata mChallenge;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
using AccountAuthenticationCloseChallengeResult = MozPromise<void*, Failure, true>;
struct ChallengePayload : IJSWord<ChallengePayload> {
  
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengePayload& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengePayload& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ChallengeMessage : IJSWord<ChallengeMessage> {
  nsString mName;
  ChallengePayload mPayload;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessage& aRv);
  static bool ToJSVal(JSContext* aCx, const ChallengeMessage& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct RespondToChallengeMessageArgs : IJSWord<RespondToChallengeMessageArgs> {
  ChallengeMetadata mChallenge;
  ChallengeMessage mChallengeMessage;
  
  
  static bool IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
  static bool FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RespondToChallengeMessageArgs& aRv);
  static bool ToJSVal(JSContext* aCx, const RespondToChallengeMessageArgs& aValue, JS::MutableHandle<JS::Value> aRv);
};
struct ChallengeMessageResponse : IJSWord<ChallengeMessageResponse> {
  ChallengePayload mPayload;
  
  
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