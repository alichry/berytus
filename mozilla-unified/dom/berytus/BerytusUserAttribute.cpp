/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusUserAttribute.h"
#include "ErrorList.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/dom/BerytusUserAttributeBinding.h"
#include "mozilla/Base64.h"
#include "mozilla/dom/RemoteWorkerTypes.h"
#include "nsString.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusUserAttribute, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusUserAttribute)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusUserAttribute)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusUserAttribute)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusUserAttribute::BerytusUserAttribute(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo
) : mGlobal(aGlobal), mId(aId), mMimeType(aMimeType), mInfo(aInfo)
{
    // Add |MOZ_COUNT_CTOR(BerytusUserAttribute);| for a non-refcounted object.
}

BerytusUserAttribute::~BerytusUserAttribute()
{
    // Add |MOZ_COUNT_DTOR(BerytusUserAttribute);| for a non-refcounted object.
}

nsIGlobalObject* BerytusUserAttribute::GetParentObject() const { return mGlobal; }

JSObject*
BerytusUserAttribute::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusUserAttribute_Binding::Wrap(aCx, this, aGivenProto);
}

void BerytusUserAttribute::GetId(nsString& aRetVal) const
{
  aRetVal.Assign(mId);
}

void BerytusUserAttribute::GetMimeType(nsString& aRetVal) const
{
  aRetVal.Assign(mMimeType);
}

void BerytusUserAttribute::GetInfo(nsString& aRetVal) const
{
  aRetVal.Assign(mInfo);
}

bool BerytusUserAttributeImpl<nsString>::CanSetValue(const SourceValueType& aVal) const {
  return aVal.IsString();
}

bool BerytusUserAttributeImpl<ArrayBuffer>::CanSetValue(const SourceValueType& aVal) const {
  return aVal.IsArrayBuffer() || aVal.IsArrayBufferView();
}

bool BerytusUserAttributeImpl<BerytusEncryptedPacket>::CanSetValue(const SourceValueType& aVal) const {
  return aVal.IsBerytusEncryptedPacket();
}

void BerytusUserAttributeImpl<nsString>::SetValue(JSContext* aCx,
                                                  const SourceValueType& aVal,
                                                  ErrorResult& aRv) {
  if (NS_WARN_IF(!aVal.IsString())) {
    aRv.ThrowTypeError("Invalid value type for string-based attribute");
    return;
  }
  mValue.SetAsString().Assign(aVal.GetAsString());
}

void BerytusUserAttributeImpl<ArrayBuffer>::SetValue(JSContext* aCx,
                                                     const SourceValueType& aVal,
                                                     ErrorResult& aRv) {
  if (aVal.IsArrayBufferView()) {
    JS::Rooted<JSObject*> view(aCx, aVal.GetAsArrayBufferView().Obj());
    bool isShared;
    JSObject* ab = JS_GetArrayBufferViewBuffer(aCx, view, &isShared);
    if (NS_WARN_IF(!ab)) {
      aRv.ThrowTypeError("Failed to get ArrayBuffer from ArrayBufferView");
      return;
    }
    // TODO(berytus): Should we check if buffer is detached?
    mValue.Uninit();
    if (NS_WARN_IF(!mValue.SetAsArrayBuffer().Init(ab))) {
      aRv.ThrowInvalidStateError("Failed to initialize ArrayBuffer");
      return;
    }
    return;
  }
  if (NS_WARN_IF(!aVal.IsArrayBuffer())) {
    aRv.ThrowTypeError("Invalid value type for ArrayBuffer-based attribute");
    return;
  }
  SetValueInternal(aVal.GetAsArrayBuffer(), aRv);
}

void BerytusUserAttributeImpl<BerytusEncryptedPacket>::SetValue(
    JSContext* aCx,
    const SourceValueType& aVal,
    ErrorResult& aRv) {
  if (NS_WARN_IF(!aVal.IsBerytusEncryptedPacket())) {
    aRv.ThrowTypeError("Invalid value type for BerytusEncryptedPacket-based attribute");
    return;
  }
  SetValueInternal(aVal.GetAsBerytusEncryptedPacket(), aRv);
}

void BerytusUserAttribute::ToJSON(
  BerytusUserAttributeJSON& aRetVal,
  ErrorResult& aRv
) const
{
  aRetVal.mId.Assign(mId);
  if (mInfo.Length() > 0) {
    aRetVal.mInfo.Construct(mInfo);
  }
  if (mMimeType.Length() > 0) {
    aRetVal.mMimeType.Construct(mMimeType);
  }
  aRetVal.mEncoding = ValueEncodingType();
  PopulateValueInJSON(aRetVal.mValue, aRv);
}

already_AddRefed<BerytusUserAttribute> BerytusUserAttribute::Create(
    JSContext* aCx,
    nsIGlobalObject* aGlobal,
    RefPtr<BerytusChannel>& aChannel,
    const nsAString& aId,
    const nsAString& aMimeType,
    const nsAString& aInfo,
    const SourceValueType& aValue,
    ErrorResult& aRv
) {
  RefPtr<BerytusUserAttribute> attr;
  if (aValue.IsArrayBuffer() || aValue.IsArrayBufferView()) {
    attr = new BerytusUserAttributeImpl<ArrayBuffer>(
      aGlobal,
      aId,
      aMimeType,
      aInfo
    );
  } else if (aValue.IsString()) {
    attr = new BerytusUserAttributeImpl<nsString>(
      aGlobal,
      aId,
      aMimeType,
      aInfo
    );
  } else if (aValue.IsBerytusEncryptedPacket()) {
    attr = new BerytusUserAttributeImpl<BerytusEncryptedPacket>(
      aGlobal,
      aId,
      aMimeType,
      aInfo
    );
  } else {
    MOZ_ASSERT(false, "Unrecognised source value type");
    aRv.ThrowTypeError("Unrecognised source value type");
    return nullptr;
  }
  attr->Attach(aChannel, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  attr->SetValue(aCx, aValue, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  return attr.forget();
}

NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusUserAttributeImpl<nsString>, BerytusUserAttribute)

BerytusUserAttributeImpl<nsString>::BerytusUserAttributeImpl(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo)
{
}

BerytusUserAttributeImpl<nsString>::~BerytusUserAttributeImpl() {}

void BerytusUserAttributeImpl<nsString>::GetValue(JSContext* aCx,
                                          ValueType& aRetVal,
                                          ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsString());
  aRetVal.SetAsString().Assign(mValue.GetAsString());
}

BerytusUserAttributeValueEncodingType BerytusUserAttributeImpl<nsString>::ValueEncodingType() const {
  return BerytusUserAttributeValueEncodingType::None;
}

void BerytusUserAttributeImpl<nsString>::PopulateValueInJSON(JSONValueType& aValue,
                                                     ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsString());
  aValue.Assign(mValue.GetAsString());
}

void BerytusUserAttributeImpl<nsString>::SetValueInternal(const nsString& aValue) {
  mValue.SetAsString().Assign(aValue);
}


NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusUserAttributeImpl<ArrayBuffer>, BerytusUserAttribute)

BerytusUserAttributeImpl<ArrayBuffer>::BerytusUserAttributeImpl(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo)
{
}

BerytusUserAttributeImpl<ArrayBuffer>::~BerytusUserAttributeImpl() {}


void BerytusUserAttributeImpl<ArrayBuffer>::GetValue(JSContext* aCx,
                                               ValueType& aRetVal,
                                               ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsArrayBuffer());
  aRetVal.SetAsArrayBuffer().Init(mValue.GetAsArrayBuffer().Obj());
}

void BerytusUserAttributeImpl<ArrayBuffer>::SetValueInternal(
    const ArrayBuffer& aValue,
    ErrorResult& aRv) {
  mValue.Uninit();
  if (NS_WARN_IF(!mValue.SetAsArrayBuffer().Init(aValue.Obj()))) {
    aRv.ThrowInvalidStateError("Failed to initialize ArrayBuffer");
  }
}

BerytusUserAttributeValueEncodingType BerytusUserAttributeImpl<ArrayBuffer>::ValueEncodingType() const {
  MOZ_ASSERT(mValue.IsArrayBuffer());
  return BerytusUserAttributeValueEncodingType::Base64URLString;
}

void BerytusUserAttributeImpl<ArrayBuffer>::PopulateValueInJSON(JSONValueType& aValue,
                                                          ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsArrayBuffer());
  JS::AutoCheckCannotGC nogc;
  bool isShared;
  JSObject* obj = mValue.GetAsArrayBuffer().Obj();
  size_t length = JS::GetArrayBufferByteLength(obj);
  uint8_t* data = JS::GetArrayBufferData(obj, &isShared, nogc);
  nsAutoCString base64Url;
  nsresult res = Base64URLEncode(
      length, data,
      Base64URLEncodePaddingPolicy::Omit, base64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aRv.Throw(res);
    return;
  }
  aValue.Assign(NS_ConvertASCIItoUTF16(base64Url));
}

NS_IMPL_ISUPPORTS_CYCLE_COLLECTION_INHERITED_0(BerytusUserAttributeImpl<BerytusEncryptedPacket>, BerytusUserAttribute)

BerytusUserAttributeImpl<BerytusEncryptedPacket>::BerytusUserAttributeImpl(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo)
{
}

BerytusUserAttributeImpl<BerytusEncryptedPacket>::~BerytusUserAttributeImpl() {}

void BerytusUserAttributeImpl<BerytusEncryptedPacket>::GetValue(JSContext* aCx,
                                                   ValueType& aRetVal,
                                                   ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsBerytusEncryptedPacket());
  aRetVal.SetAsBerytusEncryptedPacket() = mValue.GetAsBerytusEncryptedPacket();
}

void BerytusUserAttributeImpl<BerytusEncryptedPacket>::SetValueInternal(
    const RefPtr<BerytusEncryptedPacket>& aValue,
    ErrorResult& aRv) {
  if (mChannel) {
    RefPtr<BerytusChannel> ch = mChannel.get();
    aValue->Attach(ch, aRv);
    NS_ENSURE_TRUE_VOID(!aRv.Failed());
  }
  mValue.SetAsBerytusEncryptedPacket() = aValue;
}

BerytusUserAttributeValueEncodingType BerytusUserAttributeImpl<BerytusEncryptedPacket>::ValueEncodingType() const {
  MOZ_ASSERT(mValue.IsBerytusEncryptedPacket());
  return BerytusUserAttributeValueEncodingType::EncryptedPacketJSON;
}

void BerytusUserAttributeImpl<BerytusEncryptedPacket>::PopulateValueInJSON(JSONValueType& aValue,
                                                          ErrorResult& aRv) const {
  MOZ_ASSERT(mValue.IsBerytusEncryptedPacket());
  nsCString utf8Val;
  mValue.GetAsBerytusEncryptedPacket()->SerializeExposedToString(utf8Val, aRv);
  aValue.Assign(NS_ConvertUTF8toUTF16(utf8Val));
}

} // namespace mozilla::dom
