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

namespace mozilla::berytus {

using ArrayBuffer = mozilla::dom::ArrayBuffer;
bool JSValIsInt32(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Int32FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, int32_t& aRv);
bool Int32ToJSVal(JSContext* aCx, const int32_t& aValue, JS::MutableHandle<JS::Value> aRv);
struct DocumentMetadata {
  int32_t mId;
  ;
  
};
bool JSValIsDocumentMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool DocumentMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv);
bool DocumentMetadataToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct PreliminaryRequestContext {
  DocumentMetadata mDocument;
  ;
  
};
bool JSValIsPreliminaryRequestContext(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool PreliminaryRequestContextFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv);
bool PreliminaryRequestContextToJSVal(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsString(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool StringFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv);
bool StringToJSVal(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv);
struct CryptoActor {
  nsString mEd25519Key;
  ;
  
};
bool JSValIsCryptoActor(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool CryptoActorFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CryptoActor& aRv);
bool CryptoActorToJSVal(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv);
struct UriParams {
  nsString mUri;
  nsString mScheme;
  nsString mHostname;
  int32_t mPort;
  nsString mPath;
  ;
  
};
bool JSValIsUriParams(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool UriParamsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv);
bool UriParamsToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv);
struct OriginActor {
  UriParams mOriginalUri;
  UriParams mCurrentUri;
  ;
  
};
bool JSValIsOriginActor(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool OriginActorFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv);
bool OriginActorToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_CryptoActor__OriginActor_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_CryptoActor__OriginActor_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<CryptoActor, OriginActor>** aRv);
bool Variant_CryptoActor__OriginActor_ToJSVal(JSContext* aCx, const Variant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetSigningKeyArgs {
  
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  
  ~GetSigningKeyArgs() {
    delete mWebAppActor;
  }
};
bool JSValIsGetSigningKeyArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool GetSigningKeyArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetSigningKeyArgs& aRv);
bool GetSigningKeyArgsToJSVal(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv);

bool JSValIsMaybe_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsString>& aRv);
bool Maybe_nsString_ToJSVal(JSContext* aCx, const Maybe<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_int32_t_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_int32_t_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<int32_t>& aRv);
bool Maybe_int32_t_ToJSVal(JSContext* aCx, const Maybe<int32_t>& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialAccountIdentity {
  nsString mFieldId;
  nsString mFieldValue;
  ;
  
};
bool JSValIsPartialAccountIdentity(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool PartialAccountIdentityFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialAccountIdentity& aRv);
bool PartialAccountIdentityToJSVal(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsnsTArray_PartialAccountIdentity_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_PartialAccountIdentity_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<PartialAccountIdentity>& aRv);
bool nsTArray_PartialAccountIdentity_ToJSVal(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_nsTArray_PartialAccountIdentity__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsTArray_PartialAccountIdentity__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<PartialAccountIdentity>>& aRv);
bool Maybe_nsTArray_PartialAccountIdentity__ToJSVal(JSContext* aCx, const Maybe<nsTArray<PartialAccountIdentity>>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AccountConstraints {
  Maybe<nsString> mCategory;
  Maybe<int32_t> mSchemaVersion;
  Maybe<nsTArray<PartialAccountIdentity>> mIdentity;
  ;
  
};
bool JSValIsAccountConstraints(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool AccountConstraintsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AccountConstraints& aRv);
bool AccountConstraintsToJSVal(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
struct GetCredentialsMetadataArgs {
  AccountConstraints mAccountConstraints;
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  
  ~GetCredentialsMetadataArgs() {
    delete mWebAppActor;
  }
};
bool JSValIsGetCredentialsMetadataArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool GetCredentialsMetadataArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv);
bool GetCredentialsMetadataArgsToJSVal(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);

bool JSValIsnsTArray_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<nsString>& aRv);
bool nsTArray_nsString_ToJSVal(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_nsTArray_nsString__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_nsTArray_nsString__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<nsString>>& aRv);
bool Maybe_nsTArray_nsString__ToJSVal(JSContext* aCx, const Maybe<nsTArray<nsString>>& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsBool(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool BoolFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool BoolToJSVal(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsMaybe_AccountConstraints_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool Maybe_AccountConstraints_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<AccountConstraints>& aRv);
bool Maybe_AccountConstraints_ToJSVal(JSContext* aCx, const Maybe<AccountConstraints>& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChannelConstraints {
  Maybe<nsTArray<nsString>> mSecretManagerPublicKey;
  bool mEnableEndToEndEncryption;
  Maybe<AccountConstraints> mAccount;
  ;
  
};
bool JSValIsChannelConstraints(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChannelConstraintsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv);
bool ChannelConstraintsToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChannelMetadata {
  nsString mId;
  ChannelConstraints mConstraints;
  CryptoActor mScmActor;
  Variant<CryptoActor, OriginActor>* mWebAppActor = nullptr;
  
  ~ChannelMetadata() {
    delete mWebAppActor;
  }
};
bool JSValIsChannelMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChannelMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv);
bool ChannelMetadataToJSVal(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestContext {
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  ;
  
};
bool JSValIsRequestContext(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RequestContextFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv);
bool RequestContextToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv);
struct InitialKeyExchangeParametersDraft {
  nsString mChannelId;
  nsString mWebAppX25519Key;
  ;
  
};
bool JSValIsInitialKeyExchangeParametersDraft(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool InitialKeyExchangeParametersDraftFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv);
bool InitialKeyExchangeParametersDraftToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv);
struct GenerateKeyExchangeParametersArgs {
  InitialKeyExchangeParametersDraft mParamsDraft;
  ;
  
};
bool JSValIsGenerateKeyExchangeParametersArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool GenerateKeyExchangeParametersArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GenerateKeyExchangeParametersArgs& aRv);
bool GenerateKeyExchangeParametersArgsToJSVal(JSContext* aCx, const GenerateKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsArrayBuffer(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ArrayBufferFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBuffer& aRv);
bool ArrayBufferToJSVal(JSContext* aCx, const ArrayBuffer& aValue, JS::MutableHandle<JS::Value> aRv);
struct PartialKeyExchangeParametersFromScm {
  nsString mScmX25519Key;
  nsString mHkdfHash;
  ArrayBuffer mHkdfSalt;
  ArrayBuffer mHkdfInfo;
  int32_t mAesKeyLength;
  ;
  
};
bool JSValIsPartialKeyExchangeParametersFromScm(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool PartialKeyExchangeParametersFromScmFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialKeyExchangeParametersFromScm& aRv);
bool PartialKeyExchangeParametersFromScmToJSVal(JSContext* aCx, const PartialKeyExchangeParametersFromScm& aValue, JS::MutableHandle<JS::Value> aRv);

struct KeyExchangeParameters {
  nsString mPacket;
  nsString mChannelId;
  nsString mWebAppX25519Key;
  nsString mScmX25519Key;
  nsString mHkdfHash;
  ArrayBuffer mHkdfSalt;
  ArrayBuffer mHkdfInfo;
  int32_t mAesKeyLength;
  ;
  
};
bool JSValIsKeyExchangeParameters(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool KeyExchangeParametersFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv);
bool KeyExchangeParametersToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
struct EnableEndToEndEncryptionArgs {
  KeyExchangeParameters mParams;
  ArrayBuffer mWebAppPacketSignature;
  ;
  
};
bool JSValIsEnableEndToEndEncryptionArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EnableEndToEndEncryptionArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv);
bool EnableEndToEndEncryptionArgsToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv);


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
struct RequestedUserAttribute {
  bool mRequired;
  Variant<nsString>* mId = nullptr;
  
  ~RequestedUserAttribute() {
    delete mId;
  }
};
bool JSValIsRequestedUserAttribute(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RequestedUserAttributeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv);
bool RequestedUserAttributeToJSVal(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
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
struct OperationState {
  
  ;
  
};
bool JSValIsOperationState(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool OperationStateFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv);
bool OperationStateToJSVal(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv);
struct LoginOperationMetadata {
  ELoginUserIntent mIntent;
  nsTArray<RequestedUserAttribute> mRequestedUserAttributes;
  nsString mId;
  EOpeationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  ;
  
};
bool JSValIsLoginOperationMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool LoginOperationMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv);
bool LoginOperationMetadataToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct ApproveOperationArgs {
  LoginOperationMetadata mOperation;
  ;
  
};
bool JSValIsApproveOperationArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ApproveOperationArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv);
bool ApproveOperationArgsToJSVal(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv);

struct OperationMetadata {
  nsString mId;
  EOpeationType mType;
  EOperationStatus mStatus;
  OperationState mState;
  ;
  
};
bool JSValIsOperationMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool OperationMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv);
bool OperationMetadataToJSVal(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct RequestContextWithOperation {
  OperationMetadata mOperation;
  ChannelMetadata mChannel;
  DocumentMetadata mDocument;
  ;
  
};
bool JSValIsRequestContextWithOperation(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RequestContextWithOperationFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv);
bool RequestContextWithOperationToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv);
enum EMetadataStatus {
  EMetadataStatus_Pending = 0,
    EMetadataStatus_Created = 1,
    EMetadataStatus_Retired = 2,
  EMetadataStatus_EndGuard_
};
bool JSValIsEMetadataStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool EMetadataStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv);
bool EMetadataStatusToJSVal(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv);
struct RecordMetadata {
  int32_t mVersion;
  EMetadataStatus mStatus;
  nsString mCategory;
  nsString mChangePassUrl;
  ;
  
};
bool JSValIsRecordMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RecordMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv);
bool RecordMetadataToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv);

struct UpdateMetadataArgs {
  RecordMetadata mMetadata;
  ;
  
};
bool JSValIsUpdateMetadataArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool UpdateMetadataArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv);
bool UpdateMetadataArgsToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct ApproveTransitionToAuthOpArgs {
  LoginOperationMetadata mNewAuthOp;
  ;
  
};
bool JSValIsApproveTransitionToAuthOpArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ApproveTransitionToAuthOpArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv);
bool ApproveTransitionToAuthOpArgsToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct UserAttribute {
  nsString mMimeType;
  nsString mValue;
  Variant<nsString>* mId = nullptr;
  
  ~UserAttribute() {
    delete mId;
  }
};
bool JSValIsUserAttribute(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool UserAttributeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv);
bool UserAttributeToJSVal(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsnsTArray_UserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool nsTArray_UserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv);
bool nsTArray_UserAttribute_ToJSVal(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv);

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
struct BaseFieldMetadata {
  EFieldType mFieldType;
  nsString mFieldId;
  Maybe<nsString> mDescription;
  ;
  
};
bool JSValIsBaseFieldMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool BaseFieldMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BaseFieldMetadata& aRv);
bool BaseFieldMetadataToJSVal(JSContext* aCx, const BaseFieldMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsNothing(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool NothingFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv);
bool NothingToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_nsString__ArrayBuffer__Nothing_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer__Nothing_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer, Nothing>** aRv);
bool Variant_nsString__ArrayBuffer__Nothing_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv);
struct AddFieldArgs {
  BaseFieldMetadata mField;
  Variant<nsString, ArrayBuffer, Nothing>* mValue = nullptr;
  
  ~AddFieldArgs() {
    delete mValue;
  }
};
bool JSValIsAddFieldArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool AddFieldArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv);
bool AddFieldArgsToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct FieldValueRejectionReason {
  nsString mCode;
  ;
  
};
bool JSValIsFieldValueRejectionReason(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool FieldValueRejectionReasonFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv);
bool FieldValueRejectionReasonToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv);
struct RejectFieldValueArgs {
  BaseFieldMetadata mField;
  FieldValueRejectionReason mReason;
  Variant<nsString, ArrayBuffer, Nothing>* mOptionalNewValue = nullptr;
  
  ~RejectFieldValueArgs() {
    delete mOptionalNewValue;
  }
};
bool JSValIsRejectFieldValueArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RejectFieldValueArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv);
bool RejectFieldValueArgsToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv);
bool JSValIsVariant_nsString__ArrayBuffer_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv);
bool Variant_nsString__ArrayBuffer_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer>** aRv);
bool Variant_nsString__ArrayBuffer_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer>& aValue, JS::MutableHandle<JS::Value> aRv);

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
struct ChallengeParameters {
  
  ;
  
};
bool JSValIsChallengeParameters(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengeParametersFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeParameters& aRv);
bool ChallengeParametersToJSVal(JSContext* aCx, const ChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv);
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
struct ChallengeMetadata {
  nsString mId;
  EChallengeType mType;
  ChallengeParameters mParameters;
  EChallengeStatus mStatus;
  ;
  
};
bool JSValIsChallengeMetadata(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengeMetadataFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMetadata& aRv);
bool ChallengeMetadataToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv);
struct ApproveChallengeRequestArgs {
  ChallengeMetadata mChallenge;
  ;
  
};
bool JSValIsApproveChallengeRequestArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ApproveChallengeRequestArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv);
bool ApproveChallengeRequestArgsToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengeAbortionReason {
  nsString mCode;
  ;
  
};
bool JSValIsChallengeAbortionReason(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengeAbortionReasonFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeAbortionReason& aRv);
bool ChallengeAbortionReasonToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv);
struct AbortChallengeArgs {
  ChallengeMetadata mChallenge;
  ChallengeAbortionReason mReason;
  ;
  
};
bool JSValIsAbortChallengeArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool AbortChallengeArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv);
bool AbortChallengeArgsToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct CloseChallengeArgs {
  ChallengeMetadata mChallenge;
  ;
  
};
bool JSValIsCloseChallengeArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool CloseChallengeArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv);
bool CloseChallengeArgsToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengePayload {
  
  ;
  
};
bool JSValIsChallengePayload(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengePayloadFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengePayload& aRv);
bool ChallengePayloadToJSVal(JSContext* aCx, const ChallengePayload& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengeMessage {
  nsString mName;
  ChallengePayload mPayload;
  ;
  
};
bool JSValIsChallengeMessage(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengeMessageFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessage& aRv);
bool ChallengeMessageToJSVal(JSContext* aCx, const ChallengeMessage& aValue, JS::MutableHandle<JS::Value> aRv);
struct RespondToChallengeMessageArgs {
  ChallengeMetadata mChallenge;
  ChallengeMessage mChallengeMessage;
  ;
  
};
bool JSValIsRespondToChallengeMessageArgs(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool RespondToChallengeMessageArgsFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RespondToChallengeMessageArgs& aRv);
bool RespondToChallengeMessageArgsToJSVal(JSContext* aCx, const RespondToChallengeMessageArgs& aValue, JS::MutableHandle<JS::Value> aRv);
struct ChallengeMessageResponse {
  ChallengePayload mPayload;
  ;
  
};
bool JSValIsChallengeMessageResponse(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv);
bool ChallengeMessageResponseFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessageResponse& aRv);
bool ChallengeMessageResponseToJSVal(JSContext* aCx, const ChallengeMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv);


class AgentProxy final : public nsISupports {
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(AgentProxy)

public:
  AgentProxy(nsIGlobalObject* aGlobal, const nsAString& aManagerId);

protected:
  ~AgentProxy();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsString mManagerId;

public:
  void Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs, std::function<void(nsString&)>&& aResolvedCb);
  void Manager_GetCredentialsMetadata(PreliminaryRequestContext& aContext, GetCredentialsMetadataArgs& aArgs, std::function<void(int32_t&)>&& aResolvedCb);
  void Channel_GenerateKeyExchangeParameters(RequestContext& aContext, GenerateKeyExchangeParametersArgs& aArgs, std::function<void(PartialKeyExchangeParametersFromScm&)>&& aResolvedCb);
  void Channel_EnableEndToEndEncryption(RequestContext& aContext, EnableEndToEndEncryptionArgs& aArgs, std::function<void(ArrayBuffer&)>&& aResolvedCb);
  void Channel_CloseChannel(RequestContext& aContext, std::function<void(void*)>&& aResolvedCb);
  void Login_ApproveOperation(RequestContext& aContext, ApproveOperationArgs& aArgs, std::function<void(ELoginUserIntent&)>&& aResolvedCb);
  void Login_CloseOpeation(RequestContextWithOperation& aContext, std::function<void(void*)>&& aResolvedCb);
  void Login_GetRecordMetadata(RequestContextWithOperation& aContext, std::function<void(RecordMetadata&)>&& aResolvedCb);
  void Login_UpdateMetadata(RequestContextWithOperation& aContext, UpdateMetadataArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountCreation_ApproveTransitionToAuthOp(RequestContextWithOperation& aContext, ApproveTransitionToAuthOpArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountCreation_GetUserAttributes(RequestContextWithOperation& aContext, std::function<void(nsTArray<UserAttribute>)>&& aResolvedCb);
  void AccountCreation_AddField(RequestContextWithOperation& aContext, AddFieldArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountCreation_RejectFieldValue(RequestContextWithOperation& aContext, RejectFieldValueArgs& aArgs, std::function<void(Variant<nsString, ArrayBuffer>&)>&& aResolvedCb);
  void AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs, std::function<void(void*)>&& aResolvedCb);
  void AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, RespondToChallengeMessageArgs& aArgs, std::function<void(ChallengeMessageResponse&)>&& aResolvedCb);

};
}  // namespace mozilla::berytus

#endif