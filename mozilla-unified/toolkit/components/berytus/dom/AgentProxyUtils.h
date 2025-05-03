/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BERYTUS_AGENTPROXYUTILS_H_
#define BERYTUS_AGENTPROXYUTILS_H_

#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusAccountBinding.h" // BerytusFieldRejectionParameters
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BerytusUserAttributeBinding.h" // OwningStringOrArrayBufferViewOrArrayBufferOrBerytusEncryptedPacket
#include "mozilla/dom/BerytusUserAttribute.h"

namespace mozilla::dom {
  class BerytusAccount;
  class BerytusEncryptedPacket;
  class BerytusAesGcmParams_Impl;
  class BerytusEncryptionParams_Impl;
  class BerytusLoginOperation;
  class BerytusWebAppActor;
  class BerytusBuffer;
  class BerytusChallenge;
  //class BerytusField;
  class BerytusKeyFieldValue;
  class BerytusSharedKeyFieldValue;
  class BerytusSecurePasswordFieldValue;
  class BerytusIdentityField;
  class BerytusForeignIdentityField;
  class BerytusPasswordField;
  class BerytusSecurePasswordField;
  class BerytusKeyField;
  class BerytusSharedKeyField;
  class BerytusChannel;

  struct BerytusChannelConstraints;
  class BerytusSecretManagerActor;
}

namespace mozilla::berytus {

[[nodiscard]]
nsresult Utils_DocumentMetadata(nsIGlobalObject* aGlobal, berytus::DocumentMetadata& aRv);
[[nodiscard]]
nsresult Utils_ChannelMetadata(nsIGlobalObject* aGlobal, const RefPtr<const dom::BerytusChannel>& aChannel, berytus::ChannelMetadata& aRetVal);
[[nodiscard]]
nsresult Utils_ChannelMetadata(
    nsIGlobalObject* aGlobal,
    const nsString& aChannelId,
    const mozilla::dom::BerytusChannelConstraints& aCts,
    const RefPtr<const dom::BerytusWebAppActor>& aWebAppActor,
    const RefPtr<const mozilla::dom::BerytusSecretManagerActor>& aScmActor,
    berytus::ChannelMetadata& aRetVal);
void Utils_OperationMetadata(const RefPtr<const dom::BerytusLoginOperation>& aOperation, berytus::OperationMetadata& aRv);
void Utils_LoginOperationMetadata(const RefPtr<const dom::BerytusLoginOperation>& aOperation, const dom::BerytusAccount* aAccount, berytus::LoginOperationMetadata& aRetVal);

nsresult Utils_PreliminaryRequestContext(nsIGlobalObject* aGlobal,
                                         PreliminaryRequestContext& aOut);

nsresult Utils_RequestContext(nsIGlobalObject* aGlobal,
                              const RefPtr<const dom::BerytusChannel>& aChannel,
                              RequestContext& aRv);

nsresult Utils_RequestContextWithOperationMetadata(
    nsIGlobalObject *aGlobal,
    const RefPtr<dom::BerytusChannel> &aChannel,
    const RefPtr<const dom::BerytusLoginOperation>& aOperation,
    RequestContextWithOperation &aRv);

nsresult Utils_RequestContextWithLoginOperationMetadata(
    nsIGlobalObject *aGlobal,
    const RefPtr<dom::BerytusChannel> &aChannel,
    const RefPtr<const dom::BerytusLoginOperation>& aOperation,
    const dom::BerytusAccount* aAccount,
    RequestContextWithLoginOperation &aRv);

void Utils_nsURIToUriParams(nsIURI* aSrcURI, UriParams& aOut);

nsresult Utils_WebAppActorToVariant(const RefPtr<const mozilla::dom::BerytusWebAppActor>& aActor,
                                    SafeVariant<berytus::CryptoActor, berytus::OriginActor>& aRetVal);

template <typename ...T>
bool Utils_ArrayBufferToSafeVariant(const ArrayBuffer& aBuf, SafeVariant<T...>& aRetVal);
template <typename ...T>
bool Utils_ArrayBufferViewToSafeVariant(const ArrayBufferView& aBuf, SafeVariant<T...>& aRetVal);

namespace utils {

using WebAppActorProxy = SafeVariant<berytus::CryptoActor, berytus::OriginActor>;
using AesGcmParams_ImplProxy = berytus::AesGcmParams;
using EncryptedPacketParametersProxy = decltype(berytus::BerytusEncryptedPacket::mParameters);
using EncryptedPacketProxy = berytus::BerytusEncryptedPacket;
using FieldProxy = decltype(berytus::AddFieldArgs::mField);
using IdentityFieldProxy = berytus::BerytusIdentityField;
using ForeignIdentityFieldProxy = berytus::BerytusForeignIdentityField;
using PasswordFieldProxy = berytus::BerytusPasswordField;
using SecurePasswordFieldProxy = berytus::BerytusSecurePasswordField;
using KeyFieldProxy = berytus::BerytusKeyField;
using SharedKeyFieldProxy = berytus::BerytusSharedKeyField;
using SecurePasswordFieldValueProxy = berytus::BerytusSecurePasswordFieldValue;
using KeyFieldValueProxy = berytus::BerytusKeyFieldValue;
using SharedKeyFieldValueProxy = berytus::BerytusSharedKeyFieldValue;
using FieldValueUnionProxy = berytus::AccountCreationAddFieldResult::ResolveValueType;
//using FieldValueUnionProxy = decltype(berytus::RejectFieldValueArgs::mOptionalNewValue);
using FieldValueUnion = dom::Nullable<dom::BerytusField::ValueUnion>;
using OptionalFieldValueUnionProxy = decltype(berytus::RejectFieldValueArgs::mOptionalNewValue);
using UserAttributeProxy = berytus::UserAttribute;
using UserAttributeDefinition = dom::BerytusUserAttributeDefinition;
using UserAttributeValue = dom::BerytusUserAttribute::SourceValueType;
using UserAttributeValueProxy = decltype(berytus::UserAttribute::mValue);
using FieldOptionsUnionProxy = decltype(berytus::FieldInfo::mOptions);
using ChallengeInfoUnionProxy = decltype(ApproveChallengeRequestArgs::mChallenge);

class FromProxy {
public:
  static dom::BerytusAesGcmParams_Impl* BerytusAesGcmParams_Impl(
      const AesGcmParams_ImplProxy& aProxy, nsresult& aRv);
  static dom::BerytusEncryptionParams_Impl* BerytusEncryptionParams_Impl(
      const EncryptedPacketParametersProxy& aProxy, nsresult& aRv);
  static already_AddRefed<dom::BerytusEncryptedPacket> BerytusEncryptedPacket(
      nsIGlobalObject* aGlobal, const EncryptedPacketProxy& aProxy,
      nsresult& aRv);
  template <typename... T>
  static already_AddRefed<dom::BerytusBuffer> BerytusBuffer(
      nsIGlobalObject* aGlobal, const Variant<T...>& aProxy,
      nsresult& aRv);
  static nsresult BerytusUserAttributeValue(
    nsIGlobalObject* aGlobal,
    const UserAttributeValueProxy& aValueProxy,
    UserAttributeValue& aRetVal);
  static nsresult BerytusFieldValueUnion(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const FieldValueUnionProxy& aProxy,
      FieldValueUnion& aRetVal);

  static already_AddRefed<dom::BerytusSecurePasswordFieldValue> BerytusSecurePasswordFieldValue(
      nsIGlobalObject* aGlobal, const SecurePasswordFieldValueProxy& aProxy,
      nsresult& aRv);
  static already_AddRefed<dom::BerytusKeyFieldValue> BerytusKeyFieldValue(
      nsIGlobalObject* aGlobal, const KeyFieldValueProxy& aProxy,
      nsresult& aRv);
  static already_AddRefed<dom::BerytusSharedKeyFieldValue> BerytusSharedKeyFieldValue(
      nsIGlobalObject* aGlobal, const SharedKeyFieldValueProxy& aProxy,
      nsresult& aRv);

  // TODO(berytus): Remove Berytus*Field methods
  static already_AddRefed<dom::BerytusIdentityField> BerytusIdentityField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const IdentityFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusForeignIdentityField> BerytusForeignIdentityField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const ForeignIdentityFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusPasswordField> BerytusPasswordField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const PasswordFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusSecurePasswordField> BerytusSecurePasswordField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const SecurePasswordFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusKeyField> BerytusKeyField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const KeyFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusSharedKeyField> BerytusSharedKeyField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const SharedKeyFieldProxy& aProxy,
      ErrorResult& aRv);
  static already_AddRefed<dom::BerytusField> BerytusField(
      nsIGlobalObject* aGlobal, JSContext* aCx,
      const FieldProxy& aProxy,
      ErrorResult& aRv);
protected:
    struct UserAttributeValueMatcher {
        nsIGlobalObject* mGlobal;
        UserAttributeValue& mRetVal;

        UserAttributeValueMatcher(nsIGlobalObject* aGlobal,
                                  UserAttributeValue& aRetVal)
                                  : mGlobal(aGlobal), mRetVal(aRetVal) {}

        nsresult operator()(const nsString& aStr);
        nsresult operator()(const EncryptedPacketProxy& aPacketProxy);
        nsresult operator()(const ArrayBuffer& aBuf);
        nsresult operator()(const ArrayBufferView& aBuf);
    };
    struct FieldValueMatcher {
        nsIGlobalObject* mGlobal;
        JSContext* mCx;
        FieldValueUnion& mRetVal;

        FieldValueMatcher(nsIGlobalObject* aGlobal, JSContext* aCx,
                          FieldValueUnion& aRetVal)
                        : mGlobal(aGlobal), mCx(aCx), mRetVal(aRetVal) {}

        nsresult operator()(const berytus::JSNull&);
        nsresult operator()(const Nothing&);
        nsresult operator()(const nsString& aStr);
        nsresult operator()(const berytus::BerytusEncryptedPacket& aProxyPacket);
        nsresult operator()(const berytus::BerytusSecurePasswordFieldValue& aSrpProxyVal);
        nsresult operator()(const berytus::BerytusKeyFieldValue& aKeyProxyVal);
        nsresult operator()(const berytus::BerytusSharedKeyFieldValue& aSharedKeyProxyVal);
    };

    // TODO(berytus): Remove this. Won't be needed when Berytus*Field methods are removed.
    struct SetFieldValueMatcher {
        nsIGlobalObject* mGlobal;
        JSContext* mCx;
        dom::BerytusField* mField;
        ErrorResult& mRv;

        SetFieldValueMatcher(nsIGlobalObject* aGlobal, JSContext* aCx,
                        dom::BerytusField* aField, ErrorResult& aRv)
                        : mGlobal(aGlobal), mCx(aCx),
                          mField(aField), mRv(aRv) {}

        void operator()(const berytus::JSNull&);
        void operator()(const Nothing&);
        void operator()(const nsString& aStr);
        void operator()(const berytus::BerytusEncryptedPacket& aProxyPacket);
        void operator()(const berytus::BerytusSecurePasswordFieldValue& aSrpProxyVal);
        void operator()(const berytus::BerytusKeyFieldValue& aKeyProxyVal);
        void operator()(const berytus::BerytusSharedKeyFieldValue& aSharedKeyProxyVal);
    };
};

class ToProxy {
public:
    static bool BerytusField(JSContext* aCx,
                             const RefPtr<dom::BerytusField>& aField,
                             FieldProxy& aRetVal);
    static bool BerytusEncryptedPacket(JSContext* aCx,
                                       const RefPtr<dom::BerytusEncryptedPacket>& aPacket,
                                       EncryptedPacketProxy& aRetVal);
    static bool BerytusOptionalFieldValueUnion(
        JSContext* aCx,
        const FieldValueUnion& aFieldValue,
        OptionalFieldValueUnionProxy& aRetVal);

    static bool BerytusUserAttributeDefinition(
        JSContext* aCx,
        const UserAttributeDefinition& aAttr,
        UserAttributeProxy& aRetVal
    );

    static void BerytusFieldOptionsUnion(
        const RefPtr<dom::BerytusField>& aField,
        FieldOptionsUnionProxy& aRetVal
    );

    static void BerytusChallengeInfoUnion(
        const RefPtr<dom::BerytusChallenge>& aChallenge,
        ChallengeInfoUnionProxy& aRetVal
    );
};

}

}  // namespace mozilla::berytus

#endif