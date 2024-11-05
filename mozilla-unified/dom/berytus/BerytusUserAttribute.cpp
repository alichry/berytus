/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusUserAttribute.h"
#include "BerytusEncryptedPacket.h"
#include "ErrorList.h"
#include "mozilla/dom/BerytusUserAttributeBinding.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/Base64.h"

#define RET_NULLPTR_IF(arg) \
  if ((arg)) { \
    return nullptr; \
  }

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

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
void BerytusUserAttribute::ToJSON(
  BerytusUserAttributeJSON& aRetVal,
  ErrorResult& aRv
) const
{
  NS_WARN_IF(true);
}

NS_IMPL_ADDREF_INHERITED(BerytusUserAttributeString, BerytusUserAttribute)
NS_IMPL_RELEASE_INHERITED(BerytusUserAttributeString, BerytusUserAttribute)
NS_INTERFACE_MAP_BEGIN(BerytusUserAttributeString)
NS_INTERFACE_MAP_END_INHERITING(BerytusUserAttribute)

BerytusUserAttributeString::BerytusUserAttributeString(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo,
  const nsAString& aValue
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo), mValue(aValue)
{
}

BerytusUserAttributeString::~BerytusUserAttributeString() {}

void BerytusUserAttributeString::GetValue(JSContext* aCx,
                                          ValueType& aRetVal,
                                          ErrorResult& aRv) const {
  aRetVal.SetAsString().Assign(mValue);
}

void BerytusUserAttributeString::PopulateValueInJSON(JSONValueType& aRetVal,
                                                     ErrorResult& aErr) const {
  aRetVal.SetAsString().Assign(mValue);
}

NS_IMPL_CYCLE_COLLECTION_INHERITED_WITH_JS_MEMBERS(BerytusUserAttributeArrayBuffer, BerytusUserAttribute, (), (mValue))
NS_IMPL_ADDREF_INHERITED(BerytusUserAttributeArrayBuffer, BerytusUserAttribute)
NS_IMPL_RELEASE_INHERITED(BerytusUserAttributeArrayBuffer, BerytusUserAttribute)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusUserAttributeArrayBuffer)
NS_INTERFACE_MAP_END_INHERITING(BerytusUserAttribute)

BerytusUserAttributeArrayBuffer::BerytusUserAttributeArrayBuffer(
  nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo,
  const ArrayBuffer& aValue
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo),  mValue(aValue.Obj())
{
  mozilla::HoldJSObjects(this);
}

BerytusUserAttributeArrayBuffer::~BerytusUserAttributeArrayBuffer() {
  mozilla::DropJSObjects(this);
}

void BerytusUserAttributeArrayBuffer::GetValue(JSContext* aCx,
                                               ValueType& aRetVal,
                                               ErrorResult& aRv) const {
  aRetVal.SetAsArrayBuffer().Init(mValue);
}

void BerytusUserAttributeArrayBuffer::PopulateValueInJSON(JSONValueType& aRetVal,
                                                          ErrorResult& aErr) const {

  JS::AutoCheckCannotGC nogc;
  bool isShared;
  size_t length = JS::GetArrayBufferByteLength(mValue);
  uint8_t* data = JS::GetArrayBufferData(mValue, &isShared, nogc);
  nsAutoCString base64Url;
  nsresult res = Base64URLEncode(
      length, data,
      Base64URLEncodePaddingPolicy::Omit, base64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aErr.Throw(res);
    return;
  }
  aRetVal.SetAsString().Assign(NS_ConvertASCIItoUTF16(base64Url));
}

BerytusUserAttributeEncryptedPacket::BerytusUserAttributeEncryptedPacket(
    nsIGlobalObject* aGlobal,
  const nsAString& aId,
  const nsAString& aMimeType,
  const nsAString& aInfo,
  const RefPtr<BerytusEncryptedPacket>& aValue
) : BerytusUserAttribute(aGlobal, aId, aMimeType, aInfo),  mValue(aValue)
{}

BerytusUserAttributeEncryptedPacket::~BerytusUserAttributeEncryptedPacket() {}
void BerytusUserAttributeEncryptedPacket::GetValue(JSContext* aCx,
                                                   ValueType& aRetVal,
                                                   ErrorResult& aRv) const {
  aRetVal.SetAsBerytusEncryptedPacket() = mValue;
}

void BerytusUserAttributeEncryptedPacket::PopulateValueInJSON(JSONValueType& aRetVal,
                                                          ErrorResult& aErr) const {
  mValue->ToJSON(aRetVal.SetAsBerytusEncryptedPacketJSON(), aErr);
}

} // namespace mozilla::dom
