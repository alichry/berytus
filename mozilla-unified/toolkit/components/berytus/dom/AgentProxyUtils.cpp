/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * vim: set ts=8 sts=2 et sw=2 tw=80:
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/berytus/AgentProxyUtils.h"
#include "ErrorList.h"
#include "js/Value.h"
#include "js/experimental/TypedData.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusBuffer.h"
#include "mozilla/dom/BerytusChannelBinding.h"
#include "mozilla/dom/BerytusCryptoWebAppActor.h"
#include "mozilla/dom/BerytusAnonymousWebAppActor.h"
#include "mozilla/dom/BerytusFieldBinding.h"
#include "mozilla/dom/RootedDictionary.h"
#include "nsError.h"
#include "nsIGlobalObject.h"
#include "nsStringFwd.h"
#include "mozilla/dom/BerytusFieldMap.h"
#include "mozilla/dom/BerytusAccount.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/BerytusBuffer.h"
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BerytusKeyFieldValue.h"
#include "mozilla/dom/BerytusSharedKeyFieldValue.h"
#include "mozilla/dom/BerytusSecurePasswordFieldValue.h"
#include "mozilla/dom/BerytusIdentityField.h"
#include "mozilla/dom/BerytusForeignIdentityField.h"
#include "mozilla/dom/BerytusPasswordField.h"
#include "mozilla/dom/BerytusSecurePasswordField.h"
#include "mozilla/dom/BerytusKeyField.h"
#include "mozilla/dom/BerytusSharedKeyField.h"
#include "mozilla/dom/BerytusChannel.h"

namespace mozilla::berytus {

nsresult Utils_DocumentMetadata(nsIGlobalObject* aGlobal, berytus::DocumentMetadata& aRv) {
  nsPIDOMWindowInner* inner = aGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    return NS_ERROR_FAILURE;
  }
  if (NS_WARN_IF(inner->WindowID() > std::numeric_limits<double>::max())) {
    return NS_ERROR_FAILURE;
  }
  aRv.mId = static_cast<double>(inner->WindowID());
  return NS_OK;
}

nsresult Utils_ChannelMetadata(nsIGlobalObject* aGlobal, const RefPtr<const dom::BerytusChannel>& aChannel, berytus::ChannelMetadata& aRetVal) {
  const dom::BerytusChannelConstraints& cts = aChannel->Constraints();
  aChannel->GetID(aRetVal.mId);
  if (cts.mAccount.WasPassed()) {
    aRetVal.mConstraints.mAccount.emplace();
    if (cts.mAccount.Value().mCategory.WasPassed()) {
      aRetVal.mConstraints.mAccount->mCategory.emplace(cts.mAccount.Value().mCategory.Value());
    }
    if (cts.mAccount.Value().mSchemaVersion.WasPassed()) {
      aRetVal.mConstraints.mAccount->mSchemaVersion.emplace(cts.mAccount.Value().mSchemaVersion.Value());
    }
    if (cts.mAccount.Value().mIdentity.WasPassed()) {
      aRetVal.mConstraints.mAccount->mIdentity.emplace();
      for (const auto& identity : cts.mAccount.Value().mIdentity.Value().Entries()) {
        PartialAccountIdentity entry;
        entry.mFieldId.Assign(identity.mKey);
        entry.mFieldValue.Assign(identity.mValue);
        aRetVal.mConstraints.mAccount->mIdentity->AppendElement(std::move(entry));
      }
    }
  }
  if (cts.mEnableEndToEndEncryption.WasPassed()) {
    aRetVal.mConstraints.mEnableEndToEndEncryption =
      cts.mEnableEndToEndEncryption.Value();
  }
  if (cts.mSecretManagerPublicKey.WasPassed()) {
    aRetVal.mConstraints.mSecretManagerPublicKey.emplace();
    for (const auto& key : cts.mSecretManagerPublicKey.Value()) {
      aRetVal.mConstraints.mSecretManagerPublicKey->AppendElement(nsString(key));
    }
  }
  nsresult rv;
  rv = berytus::Utils_WebAppActorToVariant(aChannel->GetWebAppActor(), aRetVal.mWebAppActor);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  MOZ_ASSERT(aRetVal.mWebAppActor.Inited());
  RefPtr<dom::BerytusSecretManagerActor> scmActor =
    aChannel->GetSecretManager();
  scmActor->GetEd25519Key(aRetVal.mScmActor.mEd25519Key);
  return NS_OK;
}

void Utils_OperationMetadata(const RefPtr<const dom::BerytusLoginOperation>& aOperation, berytus::OperationMetadata& aRv) {
  nsString id;
  aOperation->GetID(aRv.mId);

  dom::BerytusOnboardingIntent intent = aOperation->Intent();

  intent == dom::BerytusOnboardingIntent::Authenticate
    ? aRv.mType.SetAsAuthentication()
    : aRv.mType.SetAsRegistration();
  // TODO(berytus): account for aborted.
  aOperation->Active()
    ? aRv.mStatus.SetAsCreated()
    : aRv.mStatus.SetAsFinished();
}

void Utils_LoginOperationMetadata(const RefPtr<const dom::BerytusLoginOperation>& aOperation, const dom::BerytusAccount* aAccount, berytus::LoginOperationMetadata& aRetVal) {
  nsString id;
  aOperation->GetID(aRetVal.mId);

  dom::BerytusOnboardingIntent intent = aOperation->Intent();

  intent == dom::BerytusOnboardingIntent::Authenticate
    ? aRetVal.mType.SetAsAuthentication()
    : aRetVal.mType.SetAsRegistration();
  // TODO(berytus): account for aborted.
  aOperation->Active()
    ? aRetVal.mStatus.SetAsCreated()
    : aRetVal.mStatus.SetAsFinished();
  RefPtr<dom::BerytusFieldMap> fields = aAccount->Fields();
  for (const auto& field : fields->List()) {
    berytus::FieldInfo info;
    field->GetId(info.mId);
    info.mType.mVal = static_cast<uint8_t>(field->Type());
    aRetVal.mFields.AppendElement(std::move(info));
  }
}

nsresult Utils_PreliminaryRequestContext(nsIGlobalObject* aGlobal,
                                         PreliminaryRequestContext& aOut) {
  return Utils_DocumentMetadata(aGlobal, aOut.mDocument);
}

nsresult Utils_RequestContext(nsIGlobalObject* aGlobal, const RefPtr<const dom::BerytusChannel>& aChannel, RequestContext& aRv) {
  nsresult rv = Utils_DocumentMetadata(aGlobal, aRv.mDocument);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  rv = Utils_ChannelMetadata(aGlobal, aChannel, aRv.mChannel);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  return NS_OK;
}

nsresult Utils_RequestContextWithOperationMetadata(
    nsIGlobalObject *aGlobal,
    const RefPtr<dom::BerytusChannel> &aChannel,
    const RefPtr<const dom::BerytusLoginOperation>& aOperation,
    RequestContextWithOperation &aRv) {
  nsresult rv = Utils_DocumentMetadata(aGlobal, aRv.mDocument);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  rv = Utils_ChannelMetadata(aGlobal, aChannel, aRv.mChannel);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  Utils_OperationMetadata(aOperation, aRv.mOperation);
  return NS_OK;
}

nsresult Utils_RequestContextWithLoginOperationMetadata(
    nsIGlobalObject *aGlobal,
    const RefPtr<dom::BerytusChannel> &aChannel,
    const RefPtr<const dom::BerytusLoginOperation>& aOperation,
    const dom::BerytusAccount* aAccount,
    RequestContextWithLoginOperation &aRv) {
nsresult rv = Utils_DocumentMetadata(aGlobal, aRv.mDocument);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  rv = Utils_ChannelMetadata(aGlobal, aChannel, aRv.mChannel);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  Utils_LoginOperationMetadata(aOperation, aAccount, aRv.mOperation);
  return NS_OK;
}

void Utils_nsURIToUriParams(nsIURI* aSrcURI, UriParams& aOut) {
  MOZ_ASSERT(aSrcURI);
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

nsresult
Utils_WebAppActorToVariant(const RefPtr<dom::BerytusWebAppActor>& aActor,
                           SafeVariant<berytus::CryptoActor, berytus::OriginActor>& aRetVal) {
  if (aActor->Type() == dom::BerytusWebAppActorType::CryptoActor) {
    CryptoActor sActor = CryptoActor();
    static_cast<dom::BerytusCryptoWebAppActor*>(aActor.get())
      ->GetEd25519Key(sActor.mEd25519Key);
    aRetVal.Init(std::move(sActor));
    return NS_OK;
  }
  nsresult rv;
  OriginActor sActor;
  auto* anonActor = static_cast<dom::BerytusAnonymousWebAppActor*>(aActor.get());
  nsIURI* originalURI = anonActor->GetOriginalURI(rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  MOZ_ASSERT(originalURI);
  nsIURI* currentURI = anonActor->GetCurrentURI(rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  MOZ_ASSERT(currentURI);
  Utils_nsURIToUriParams(originalURI, sActor.mOriginalUri);
  Utils_nsURIToUriParams(currentURI, sActor.mCurrentUri);
  aRetVal.Init(std::move(sActor));
  return NS_OK;
}

template <typename ...T>
bool Utils_ArrayBufferToSafeVariant(const ArrayBuffer& aBuf,
                                        SafeVariant<T...>& aRetVal) {
  ArrayBuffer newBuf;
  if (NS_WARN_IF(!newBuf.Init(aBuf.Obj()))) {
    return false;
  }
  aRetVal.Init(mozilla::VariantType<ArrayBuffer>(), std::move(newBuf));
  return true;
}

template <typename ...T>
bool Utils_ArrayBufferViewToSafeVariant(const ArrayBufferView& aBuf,
                                        SafeVariant<T...>& aRetVal) {
  ArrayBufferView newBuf;
  if (NS_WARN_IF(!newBuf.Init(aBuf.Obj()))) {
    return false;
  }
  aRetVal.Init(mozilla::VariantType<ArrayBufferView>(), std::move(newBuf));
  return true;
}

namespace utils {

dom::BerytusAesGcmParams_Impl* FromProxy::BerytusAesGcmParams_Impl(
      const AesGcmParams_ImplProxy& aProxy, nsresult& aRv) {
  // TODO: perhaps impl a utility function that converts a
  // Variant<ArrayBuffer, ArrayBufferView> to CryptoBuffer?
  MOZ_ASSERT(aProxy.mIv.Inited());
  dom::CryptoBuffer iv, addData;
  if (aProxy.mIv.InternalValue()->is<dom::ArrayBuffer>()) {
    if (NS_WARN_IF(!iv.Assign(aProxy.mIv.InternalValue()->as<ArrayBuffer>()))) {
      aRv = NS_ERROR_OUT_OF_MEMORY;
      return nullptr;
    }
  } else if (aProxy.mIv.InternalValue()->is<ArrayBufferView>()) {
    if (NS_WARN_IF(!iv.Assign(aProxy.mIv.InternalValue()->as<ArrayBufferView>()))) {
      aRv = NS_ERROR_OUT_OF_MEMORY;
      return nullptr;
    }
  } else {
    MOZ_ASSERT(false, "IV should either be an ArrayBuffer or ArrayBufferView");
    aRv = NS_ERROR_INVALID_ARG;
    return nullptr;
  }
  if (aProxy.mAdditionalData.Inited()) {
    if (aProxy.mAdditionalData.InternalValue()->is<ArrayBuffer>()) {
      if (NS_WARN_IF(!addData.Assign(aProxy.mAdditionalData.InternalValue()->as<ArrayBuffer>()))) {
        aRv = NS_ERROR_OUT_OF_MEMORY;
        return nullptr;
      }
    } else if (aProxy.mAdditionalData.InternalValue()->is<ArrayBufferView>()) {
      if (NS_WARN_IF(!addData.Assign(aProxy.mAdditionalData.InternalValue()->as<ArrayBufferView>()))) {
        aRv = NS_ERROR_OUT_OF_MEMORY;
        return nullptr;
      }
    } else {
      MOZ_ASSERT(false, "AdditionalData should either be an ArrayBuffer or ArrayBufferView");
      aRv = NS_ERROR_INVALID_ARG;
      return nullptr;
    }
  }
  // TODO(berytus): Why is tagLength optional?
  return new dom::BerytusAesGcmParams_Impl(std::move(iv),
                                      std::move(addData),
                                      aProxy.mTagLength.isNothing()
                                      ? 0 : static_cast<uint8_t>(aProxy.mTagLength.value()));
}
dom::BerytusEncryptionParams_Impl* FromProxy::BerytusEncryptionParams_Impl(
    const EncryptedPacketParametersProxy& aProxy, nsresult& aRv) {
  return FromProxy::BerytusAesGcmParams_Impl(aProxy, aRv);
}
already_AddRefed<dom::BerytusEncryptedPacket> FromProxy::BerytusEncryptedPacket(
    nsIGlobalObject* aGlobal, const EncryptedPacketProxy& aProxy,
    nsresult& aRv) {
  dom::BerytusEncryptionParams_Impl* params = FromProxy::BerytusEncryptionParams_Impl(aProxy.mParameters, aRv);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  dom::CryptoBuffer ciphertext;
  if (NS_WARN_IF(!ciphertext.Assign(aProxy.mCiphertext))) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  aRv = NS_OK;
  RefPtr<dom::BerytusEncryptedPacket> packet = new dom::BerytusEncryptedPacket(
    aGlobal,
    params,
    std::move(ciphertext)
  );
  return packet.forget();
}

template <typename... T>
already_AddRefed<dom::BerytusBuffer> FromProxy::BerytusBuffer(
    nsIGlobalObject* aGlobal, const Variant<T...>& aProxy,
    nsresult& aRv) {
  struct CreateMatcher {
    nsIGlobalObject* mGlobal;
    nsresult mRv;

    CreateMatcher(nsIGlobalObject* aGlobal) : mGlobal(aGlobal), mRv(NS_OK) {}

    already_AddRefed<dom::BerytusBuffer> operator()(const ArrayBuffer& aBuffer) {
      auto buff = dom::BerytusBuffer::FromArrayBuffer(aBuffer, mRv);
      return buff;
    }

    already_AddRefed<dom::BerytusBuffer> operator()(const ArrayBufferView& aBuffer) {
      auto buff = dom::BerytusBuffer::FromArrayBufferView(aBuffer, mRv);
      return buff;
    }

    already_AddRefed<dom::BerytusBuffer> operator()(const EncryptedPacketProxy& aProxyPacket) {
      RefPtr<dom::BerytusEncryptedPacket> packet = FromProxy::BerytusEncryptedPacket(mGlobal, aProxyPacket, mRv);
      if (NS_WARN_IF(NS_FAILED(mRv))) {
        return nullptr;
      }
      RefPtr<dom::BerytusBuffer> buff = new dom::BerytusBuffer(packet);
      mRv = NS_OK;
      return buff.forget();
    }
  };
  CreateMatcher matcher(aGlobal);
  auto buffer = aProxy.match(matcher);
  if (NS_WARN_IF(NS_FAILED(matcher.mRv))) {
    aRv = matcher.mRv;
    return nullptr;
  }
  aRv = NS_OK;
  return buffer;
}

nsresult FromProxy::BerytusFieldValueUnion(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const FieldValueUnionProxy& aProxy,
    FieldValueUnion& aRetVal) {
  FieldValueMatcher matcher(aGlobal, aCx, aRetVal);
  if (aProxy.IsNull()) {
    aRetVal.SetNull();
    return NS_OK;
  }
  if (NS_WARN_IF(!aProxy.Value().Inited())) {
    return NS_ERROR_INVALID_ARG;;
  }
  nsresult rv = aProxy.Value().InternalValue()->match(matcher);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  MOZ_ASSERT(!aRetVal.IsNull());
  return NS_OK;
}

already_AddRefed<dom::BerytusSecurePasswordFieldValue> FromProxy::BerytusSecurePasswordFieldValue(
      nsIGlobalObject* aGlobal, const SecurePasswordFieldValueProxy& aProxy,
      nsresult& aRv) {
  MOZ_ASSERT(aProxy.mSalt.Inited());
  MOZ_ASSERT(aProxy.mVerifier.Inited());
  RefPtr<dom::BerytusBuffer> salt;
  RefPtr<dom::BerytusBuffer> verifier;
  salt = FromProxy::BerytusBuffer(aGlobal, *aProxy.mSalt.InternalValue(), aRv);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  verifier = FromProxy::BerytusBuffer(aGlobal, *aProxy.mVerifier.InternalValue(), aRv);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  RefPtr<dom::BerytusSecurePasswordFieldValue> value =
    new dom::BerytusSecurePasswordFieldValue(aGlobal, salt, verifier);
  return value.forget();
}
already_AddRefed<dom::BerytusKeyFieldValue> FromProxy::BerytusKeyFieldValue(
    nsIGlobalObject* aGlobal, const KeyFieldValueProxy& aProxy,
    nsresult& aRv) {
  MOZ_ASSERT(aProxy.mPublicKey.Inited());
  RefPtr<dom::BerytusBuffer> publicKeyBuff = FromProxy::BerytusBuffer(aGlobal, *aProxy.mPublicKey.InternalValue(), aRv);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  RefPtr<dom::BerytusKeyFieldValue> fieldValue =
    new dom::BerytusKeyFieldValue(aGlobal, publicKeyBuff);
  return fieldValue.forget();
}
already_AddRefed<dom::BerytusSharedKeyFieldValue> FromProxy::BerytusSharedKeyFieldValue(
    nsIGlobalObject* aGlobal, const SharedKeyFieldValueProxy& aProxy,
    nsresult& aRv) {
  MOZ_ASSERT(aProxy.mPrivateKey.Inited());
  RefPtr<dom::BerytusBuffer> privateKeyBuff = FromProxy::BerytusBuffer(aGlobal, *aProxy.mPrivateKey.InternalValue(), aRv);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  RefPtr<dom::BerytusSharedKeyFieldValue> fieldValue =
    new dom::BerytusSharedKeyFieldValue(aGlobal, privateKeyBuff);
  return fieldValue.forget();
}

already_AddRefed<dom::BerytusIdentityField> FromProxy::BerytusIdentityField(
    nsIGlobalObject* aGlobal,
    JSContext* aCx,
    const IdentityFieldProxy& aProxy,
    ErrorResult& aRv) {
  MOZ_ASSERT(aProxy.mValue.Inited());
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusIdentityFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusIdentityField> field = new dom::BerytusIdentityField(
      aGlobal, id, std::move(options));
  SetFieldValueMatcher matcher(aGlobal, aCx, field, aRv);
  aProxy.mValue.InternalValue()->match(matcher);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}
already_AddRefed<dom::BerytusForeignIdentityField> FromProxy::BerytusForeignIdentityField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const ForeignIdentityFieldProxy& aProxy,
    ErrorResult& aRv) {
  MOZ_ASSERT(aProxy.mValue.Inited());
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusForeignIdentityFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusForeignIdentityField> field = new dom::BerytusForeignIdentityField(
      aGlobal, id, std::move(options));
  SetFieldValueMatcher matcher(aGlobal, aCx, field, aRv);
  aProxy.mValue.InternalValue()->match(matcher);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}
already_AddRefed<dom::BerytusPasswordField> FromProxy::BerytusPasswordField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const PasswordFieldProxy& aProxy,
    ErrorResult& aRv) {
  MOZ_ASSERT(aProxy.mValue.Inited());
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusPasswordFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusPasswordField> field = new dom::BerytusPasswordField(
      aGlobal, id, std::move(options));
  SetFieldValueMatcher matcher(aGlobal, aCx, field, aRv);
  aProxy.mValue.InternalValue()->match(matcher);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}
already_AddRefed<dom::BerytusSecurePasswordField> FromProxy::BerytusSecurePasswordField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const SecurePasswordFieldProxy& aProxy,
    ErrorResult& aRv) {
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusSecurePasswordFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusSecurePasswordField> field = new dom::BerytusSecurePasswordField(
      aGlobal, id, std::move(options));
  RefPtr<dom::BerytusSecurePasswordFieldValue> fieldValue = nullptr;
  if (NS_WARN_IF(!aProxy.mValue.Inited())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (! aProxy.mValue.InternalValue()->is<berytus::JSNull>()) {
    nsresult rv;
    fieldValue = FromProxy::BerytusSecurePasswordFieldValue(
        aGlobal,
        aProxy.mValue.InternalValue()->as<berytus::BerytusSecurePasswordFieldValue>(),
        rv);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.Throw(rv);
      return nullptr;
    }
  }

  dom::Nullable<dom::BerytusField::ValueUnion> val;
  if (!fieldValue) {
    val.SetNull();
  } else {
    val.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  }
  field->SetValue(aCx, val, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}
already_AddRefed<dom::BerytusKeyField> FromProxy::BerytusKeyField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const KeyFieldProxy& aProxy,
    ErrorResult& aRv) {
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusKeyFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusKeyField> field = new dom::BerytusKeyField(
      aGlobal, id, std::move(options));
  RefPtr<dom::BerytusKeyFieldValue> fieldValue = nullptr;
  if (NS_WARN_IF(!aProxy.mValue.Inited())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!aProxy.mValue.InternalValue()->is<berytus::JSNull>()) {
    nsresult rv;
    fieldValue = FromProxy::BerytusKeyFieldValue(
        aGlobal,
        aProxy.mValue.InternalValue()->as<berytus::BerytusKeyFieldValue>(),
        rv);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.Throw(rv);
      return nullptr;
    }
  }
  dom::Nullable<dom::BerytusField::ValueUnion> val;
  if (!fieldValue) {
    val.SetNull();
  } else {
    val.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  }
  field->SetValue(aCx, val, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}
already_AddRefed<dom::BerytusSharedKeyField> FromProxy::BerytusSharedKeyField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const SharedKeyFieldProxy& aProxy,
    ErrorResult& aRv) {
  nsString id;
  id.Assign(aProxy.mId);
  JS::Rooted<JS::Value> optionsV(aCx);
  if (NS_WARN_IF(!mozilla::berytus::ToJSVal(aCx, aProxy.mOptions, &optionsV))) {
    aRv.ThrowTypeError("Unable to deserialise field options");
    return nullptr;
  }
  dom::RootedDictionary<dom::BerytusSharedKeyFieldOptions> options(aCx);
  if (NS_WARN_IF(!options.Init(aCx, optionsV))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::BerytusSharedKeyField> field = new dom::BerytusSharedKeyField(
      aGlobal, id, std::move(options));
  RefPtr<dom::BerytusSharedKeyFieldValue> fieldValue = nullptr;
  if (NS_WARN_IF(!aProxy.mValue.Inited())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (! aProxy.mValue.InternalValue()->is<berytus::JSNull>()) {
    nsresult rv;
    fieldValue = FromProxy::BerytusSharedKeyFieldValue(
        aGlobal,
        aProxy.mValue.InternalValue()->as<berytus::BerytusSharedKeyFieldValue>(),
        rv);
    if (NS_WARN_IF(NS_FAILED(rv))) {
      aRv.Throw(rv);
      return nullptr;
    }
  }
  dom::Nullable<dom::BerytusField::ValueUnion> val;
  if (!fieldValue) {
    val.SetNull();
  } else {
    val.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  }
  field->SetValue(aCx, val, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return field.forget();
}

already_AddRefed<dom::BerytusField> FromProxy::BerytusField(
    nsIGlobalObject* aGlobal, JSContext* aCx,
    const FieldProxy& aProxy,
    ErrorResult& aRv) {
  MOZ_ASSERT(aProxy.Inited());
  struct FieldMatcher {
    nsIGlobalObject* mGlobal;
    JSContext* mCx;
    ErrorResult mRv;

    FieldMatcher(nsIGlobalObject* aGlobal, JSContext* aCx)
                 : mGlobal(aGlobal), mCx(aCx) {}

    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusIdentityField& aProxy) {
      return FromProxy::BerytusIdentityField(mGlobal, mCx, aProxy, mRv);
    }
    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusForeignIdentityField& aProxy) {
      return FromProxy::BerytusForeignIdentityField(mGlobal, mCx, aProxy, mRv);
    }
    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusPasswordField& aProxy) {
      return FromProxy::BerytusPasswordField(mGlobal, mCx, aProxy, mRv);
    }
    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusSecurePasswordField& aProxy) {
      return FromProxy::BerytusSecurePasswordField(mGlobal, mCx, aProxy, mRv);
    }
    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusKeyField& aProxy) {
      return FromProxy::BerytusKeyField(mGlobal, mCx, aProxy, mRv);
    }
    already_AddRefed<dom::BerytusField> operator()(const berytus::BerytusSharedKeyField& aProxy) {
      return FromProxy::BerytusSharedKeyField(mGlobal, mCx, aProxy, mRv);
    }
  };
  FieldMatcher matcher(aGlobal, aCx);
  RefPtr<dom::BerytusField> field = aProxy.InternalValue()->match(matcher);
  aRv = std::move(matcher.mRv);
  return field.forget();
}

nsresult FromProxy::FieldValueMatcher::operator()(const JSNull&) {
  // set as null
  mRetVal.SetNull();
  return NS_OK;
}

nsresult FromProxy::FieldValueMatcher::operator()(const Nothing&) {
  // set as null
  mRetVal.SetNull();
  return NS_OK;
}

nsresult FromProxy::FieldValueMatcher::operator()(const nsString& aStr) {
  mRetVal.SetValue().SetAsString().Assign(aStr);
  return NS_OK;
}

nsresult FromProxy::FieldValueMatcher::operator()(
    const berytus::BerytusEncryptedPacket& aProxyPacket) {
  nsresult rv;
  RefPtr<dom::BerytusEncryptedPacket> packet =
      FromProxy::BerytusEncryptedPacket(mGlobal, aProxyPacket, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  mRetVal.SetValue().SetAsBerytusEncryptedPacket() = packet;
  return NS_OK;
}

nsresult FromProxy::FieldValueMatcher::operator()(
    const berytus::BerytusSecurePasswordFieldValue& aSrpProxyVal) {
  nsresult rv;
  RefPtr<dom::BerytusSecurePasswordFieldValue> fieldValue =
      FromProxy::BerytusSecurePasswordFieldValue(mGlobal, aSrpProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  mRetVal.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  return NS_OK;
}

nsresult FromProxy::FieldValueMatcher::operator()(
    const berytus::BerytusKeyFieldValue& aKeyProxyVal) {
  nsresult rv;
  RefPtr<dom::BerytusKeyFieldValue> fieldValue =
      FromProxy::BerytusKeyFieldValue(mGlobal, aKeyProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  mRetVal.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  return NS_OK;
}
nsresult FromProxy::FieldValueMatcher::operator()(
    const berytus::BerytusSharedKeyFieldValue& aSharedKeyProxyVal) {
  nsresult rv;
  RefPtr<dom::BerytusSharedKeyFieldValue> fieldValue =
      FromProxy::BerytusSharedKeyFieldValue(mGlobal, aSharedKeyProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  mRetVal.SetValue().SetAsBerytusFieldValueDictionary() = fieldValue;
  return NS_OK;
}

nsresult FromProxy::BerytusUserAttributeValue(
    nsIGlobalObject* aGlobal,
    const UserAttributeValueProxy& aValueProxy,
    UserAttributeValue& aRetVal) {
  UserAttributeValueMatcher matcher(aGlobal, aRetVal);
  if (NS_WARN_IF(!aValueProxy.Inited())) {
    return NS_ERROR_INVALID_ARG;
  }
  nsresult rv = aValueProxy.InternalValue()->match(matcher);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  return NS_OK;
}

nsresult FromProxy::UserAttributeValueMatcher::operator()(const nsString& aStr) {
  mRetVal.SetAsString().Assign(aStr);
  return NS_OK;
}
nsresult FromProxy::UserAttributeValueMatcher::operator()(const EncryptedPacketProxy& aPacketProxy) {
  nsresult rv;
  RefPtr<dom::BerytusEncryptedPacket> packet = FromProxy::BerytusEncryptedPacket(mGlobal,aPacketProxy, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    return rv;
  }
  mRetVal.SetAsBerytusEncryptedPacket() = packet;
  return NS_OK;
}
nsresult FromProxy::UserAttributeValueMatcher::operator()(const ArrayBuffer& aBuf) {
  if (NS_WARN_IF(!mRetVal.SetAsArrayBuffer().Init(aBuf.Obj()))) {
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}
nsresult FromProxy::UserAttributeValueMatcher::operator()(const ArrayBufferView& aBuf) {
  if (NS_WARN_IF(!mRetVal.SetAsArrayBufferView().Init(aBuf.Obj()))) {
    return NS_ERROR_FAILURE;
  }
  return NS_OK;
}


void FromProxy::SetFieldValueMatcher::operator()(const JSNull&) {
  // set as null
  dom::Nullable<dom::BerytusField::ValueUnion> v;
  v.SetNull();
  mField->SetValue(mCx, v, mRv);
}

void FromProxy::SetFieldValueMatcher::operator()(const Nothing&) {
  // set as null
  dom::Nullable<dom::BerytusField::ValueUnion> v;
  v.SetNull();
  mField->SetValue(mCx, v, mRv);
}

void FromProxy::SetFieldValueMatcher::operator()(const nsString& aStr) {
  dom::BerytusField::ValueUnion v;
  v.SetAsString().Assign(aStr);
  mField->SetValue(
      mCx, dom::Nullable<dom::BerytusField::ValueUnion>(std::move(v)), mRv);
}

void FromProxy::SetFieldValueMatcher::operator()(
    const berytus::BerytusEncryptedPacket& aProxyPacket) {
  nsresult rv;
  dom::BerytusField::ValueUnion v;
  RefPtr<dom::BerytusEncryptedPacket> packet =
      FromProxy::BerytusEncryptedPacket(mGlobal, aProxyPacket, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    mRv.Throw(rv);
    return;
  }
  v.SetAsBerytusEncryptedPacket() = packet;
  mField->SetValue(
      mCx, dom::Nullable<dom::BerytusField::ValueUnion>(std::move(v)), mRv);
}
void FromProxy::SetFieldValueMatcher::operator()(
    const berytus::BerytusSecurePasswordFieldValue& aSrpProxyVal) {
  nsresult rv;
  dom::BerytusField::ValueUnion v;
  RefPtr<dom::BerytusSecurePasswordFieldValue> fieldValue =
      FromProxy::BerytusSecurePasswordFieldValue(mGlobal, aSrpProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    mRv.Throw(rv);
    return;
  }
  v.SetAsBerytusFieldValueDictionary() = fieldValue;
  mField->SetValue(
      mCx, dom::Nullable<dom::BerytusField::ValueUnion>(std::move(v)), mRv);
}
void FromProxy::SetFieldValueMatcher::operator()(
    const berytus::BerytusKeyFieldValue& aKeyProxyVal) {
  nsresult rv;
  dom::BerytusField::ValueUnion v;
  RefPtr<dom::BerytusKeyFieldValue> fieldValue =
      FromProxy::BerytusKeyFieldValue(mGlobal, aKeyProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    mRv.Throw(rv);
    return;
  }
  v.SetAsBerytusFieldValueDictionary() = fieldValue;
  mField->SetValue(
      mCx, dom::Nullable<dom::BerytusField::ValueUnion>(std::move(v)), mRv);
}
void FromProxy::SetFieldValueMatcher::operator()(
    const berytus::BerytusSharedKeyFieldValue& aSharedKeyProxyVal) {
  nsresult rv;
  dom::BerytusField::ValueUnion v;
  RefPtr<dom::BerytusSharedKeyFieldValue> fieldValue =
      FromProxy::BerytusSharedKeyFieldValue(mGlobal, aSharedKeyProxyVal, rv);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    mRv.Throw(rv);
    return;
  }
  v.SetAsBerytusFieldValueDictionary() = fieldValue;
  mField->SetValue(
      mCx, dom::Nullable<dom::BerytusField::ValueUnion>(std::move(v)), mRv);
}


bool ToProxy::BerytusField(JSContext* aCx,
                           const RefPtr<dom::BerytusField>& aField,
                           FieldProxy& aRetVal) {
  JS::Rooted<JS::Value> jsField(aCx);
  if (NS_WARN_IF(!GetOrCreateDOMReflector(aCx, aField, &jsField))) {
    return false;
  }
  if (aField->Type() == dom::BerytusFieldType::Identity) {
    berytus::BerytusIdentityField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(VariantType<berytus::BerytusIdentityField>{}, std::move(proxyField));
    return true;
  }
  if (aField->Type() == dom::BerytusFieldType::ForeignIdentity) {
    berytus::BerytusForeignIdentityField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(std::move(proxyField));
    return true;
  }
  if (aField->Type() == dom::BerytusFieldType::Password) {
    berytus::BerytusPasswordField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(std::move(proxyField));
    return true;
  }
  if (aField->Type() == dom::BerytusFieldType::SecurePassword) {
    berytus::BerytusSecurePasswordField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(std::move(proxyField));
    return true;
  }
  if (aField->Type() == dom::BerytusFieldType::Key) {
    berytus::BerytusKeyField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(std::move(proxyField));
    return true;
  }
  if (aField->Type() == dom::BerytusFieldType::SharedKey) {
    berytus::BerytusSharedKeyField proxyField;
    if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsField, proxyField))) {
      return false;
    }
    aRetVal.Init(std::move(proxyField));
    return true;
  }
  MOZ_ASSERT(false, "Unrecognised Berytus Field Type");
  return false;
}

bool ToProxy::BerytusEncryptedPacket(JSContext* aCx,
                                     const RefPtr<dom::BerytusEncryptedPacket>& aPacket,
                                     EncryptedPacketProxy& aRetVal) {
  JS::Rooted<JS::Value> jsPacket(aCx);
  if (NS_WARN_IF(!GetOrCreateDOMReflector(aCx, aPacket, &jsPacket))) {
    return false;
  }
  if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsPacket, aRetVal))) {
    return false;
  }
  return true;
}

bool ToProxy::BerytusOptionalFieldValueUnion(JSContext* aCx,
                                const FieldValueUnion& aFieldValue,
                                OptionalFieldValueUnionProxy& aRetVal) {
  if (aFieldValue.IsNull()) {
    aRetVal.Init(mozilla::Nothing());
    return true;
  }
  const auto& value = aFieldValue.Value();
  if (value.IsBerytusFieldValueDictionary()) {
    const auto& dict = value.GetAsBerytusFieldValueDictionary();
    JS::Rooted<JS::Value> jsDict(aCx);
    if (NS_WARN_IF(!GetOrCreateDOMReflector(aCx, dict, &jsDict))) {
      return false;
    }
    switch (dict->Type()) {
      case dom::BerytusFieldType::SecurePassword: {
        BerytusSecurePasswordFieldValue fvProxy;
        if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsDict, fvProxy))) {
          return false;
        }
        aRetVal.Init(std::move(fvProxy));
        break;
      }
      case dom::BerytusFieldType::Key: {
        BerytusKeyFieldValue fvProxy;
        if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsDict, fvProxy))) {
          return false;
        }
        aRetVal.Init(std::move(fvProxy));
        break;
      }
      case dom::BerytusFieldType::SharedKey: {
        BerytusSharedKeyFieldValue fvProxy;
        if (NS_WARN_IF(!berytus::FromJSVal(aCx, jsDict, fvProxy))) {
          return false;
        }
        aRetVal.Init(std::move(fvProxy));
        break;
      }
      default:
        MOZ_ASSERT(false, "Unrecognised/unsupported field type when converting field value dictionary to a proxy equivalent");
        return false;
    }
    return true;
  }
  if (value.IsString()) {
    aRetVal.Init(nsString(value.GetAsString()));
    return true;
  }
  if (value.IsBerytusEncryptedPacket()) {
    EncryptedPacketProxy packetProxy;
    if (NS_WARN_IF(!ToProxy::BerytusEncryptedPacket(aCx, value.GetAsBerytusEncryptedPacket(), packetProxy))) {
      return false;
    }
    aRetVal.Init(std::move(packetProxy));
    return true;
  }
  MOZ_ASSERT(false, "Unrecognised/unsupported DOM field value union.");
  return false;
}

bool ToProxy::BerytusUserAttributeDefinition(
    JSContext* aCx,
    const UserAttributeDefinition& aAttr,
    UserAttributeProxy& aRetVal) {
  Maybe<nsString> info;
  Maybe<nsString> mimeType;
  if (aAttr.mInfo.WasPassed()) {
    aRetVal.mInfo.emplace(nsString(aAttr.mInfo.Value()));
  }
  if (aAttr.mMimeType.WasPassed()) {
    aRetVal.mMimeType.emplace(nsString(aAttr.mMimeType.Value()));
  }
  aRetVal.mId.Assign(aAttr.mId);
  if (aAttr.mValue.IsArrayBuffer()) {
    return !NS_WARN_IF(!berytus::Utils_ArrayBufferToSafeVariant(aAttr.mValue.GetAsArrayBuffer(), aRetVal.mValue));
  }
  if (aAttr.mValue.IsArrayBufferView()) {
    return !NS_WARN_IF(!berytus::Utils_ArrayBufferViewToSafeVariant(aAttr.mValue.GetAsArrayBufferView(), aRetVal.mValue));
  }
  if (aAttr.mValue.IsBerytusEncryptedPacket()) {
    EncryptedPacketProxy packetProxy;
    if (NS_WARN_IF(!ToProxy::BerytusEncryptedPacket(aCx, aAttr.mValue.GetAsBerytusEncryptedPacket(), packetProxy))) {
      return false;
    }
    aRetVal.mValue.Init(std::move(packetProxy));
    return true;
  }
  if (aAttr.mValue.IsString()) {
    aRetVal.mValue.Init(nsString(aAttr.mValue.GetAsString()));
    return true;
  }
  MOZ_ASSERT(false, "Unrecognised user attribute definition value union member");
  return false;
}

}

};