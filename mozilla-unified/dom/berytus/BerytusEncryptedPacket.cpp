/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "ErrorList.h"
#include "mozilla/dom/BerytusEncryptedPacketBinding.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "js/ArrayBuffer.h"
#include "mozilla/Base64.h"
#include "mozilla/dom/WebCryptoCommon.h" //WEBCRYPTO_ALG_AES_GCM
#include "js/PropertyAndElement.h"

namespace mozilla::dom {

// Only needed for refcounted objects.
//NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE(BerytusEncryptedPacket, mGlobal)
NS_IMPL_CYCLE_COLLECTION_WRAPPERCACHE_WITH_JS_MEMBERS(BerytusEncryptedPacket, (mGlobal), (mCachedParams, mCachedCiphertextArrayBuffer))

NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusEncryptedPacket)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusEncryptedPacket)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusEncryptedPacket)
  NS_WRAPPERCACHE_INTERFACE_MAP_ENTRY
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusEncryptionParams_Impl::~BerytusEncryptionParams_Impl() {}

BerytusAesGcmParams_Impl::BerytusAesGcmParams_Impl(CryptoBuffer&& aIv,
                         CryptoBuffer&& aAdditionalData,
                         const uint8_t& aTagLen) : mIv(std::move(aIv)),
                                            mAdditionalData(std::move(aAdditionalData)),
                                            mTagLen(aTagLen) {

}

BerytusAesGcmParams_Impl::BerytusAesGcmParams_Impl(
  BerytusAesGcmParams_Impl&& aOther) : BerytusAesGcmParams_Impl(std::move(aOther.mIv), std::move(aOther.mAdditionalData), aOther.mTagLen) {

}

BerytusAesGcmParams_Impl::~BerytusAesGcmParams_Impl() {

}

BerytusAesGcmParams_Impl*
BerytusAesGcmParams_Impl::FromDictionary(const AesGcmParams& aDict,
                                   nsresult& aRv) {
  CryptoBuffer iv;
  CryptoBuffer addData;
  if (!iv.Assign(aDict.mIv)) {
    aRv = NS_ERROR_DOM_SECURITY_ERR;
    return nullptr;
  }

  if (aDict.mAdditionalData.WasPassed() && !addData.Assign(aDict.mAdditionalData.Value())) {
    aRv = NS_ERROR_DOM_SECURITY_ERR;
    return nullptr;
  }
  if (!aDict.mTagLength.WasPassed()) {
    aRv = NS_ERROR_DOM_SECURITY_ERR;
    return nullptr;
  }
  aRv = NS_OK;
  return new BerytusAesGcmParams_Impl(
    std::move(iv),
    std::move(addData),
    aDict.mTagLength.Value()
  );
}

void BerytusAesGcmParams_Impl::GetAlgorithm(nsString& aRv) {
  aRv.Assign(NS_ConvertASCIItoUTF16(WEBCRYPTO_ALG_AES_GCM));
}

void BerytusAesGcmParams_Impl::AsDictionary(JSContext* aCx,
                              JS::Heap<JSObject*>& aObj,
                              JS::MutableHandle<JSObject*> aRetVal,
                              ErrorResult& aErr) {
  //JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  aObj = JS_NewPlainObject(aCx);
  JS::Rooted<JSObject*> obj(aCx, aObj.get());

  JSObject* ivObj = ArrayBuffer::Create(aCx, mIv, aErr);
  if (NS_WARN_IF(aErr.Failed())) {
    return;
  }
  JS::Rooted<JS::Value> iv(aCx, JS::ObjectValue(*ivObj));
  if (NS_WARN_IF(aErr.Failed())) {
    return;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "iv", iv))) {
    aErr.Throw(NS_ERROR_FAILURE);
    return;
  }

  if (mAdditionalData.Length() > 0) {
    JSObject* addDataObj = ArrayBuffer::Create(aCx, mAdditionalData, aErr);
    if (NS_WARN_IF(aErr.Failed())) {
      return;
    }
    JS::Rooted<JS::Value> addData(aCx, JS::ObjectValue(*addDataObj));

    if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "additionalData", addData))) {
      aErr.Throw(NS_ERROR_FAILURE);
      return;
    }
  }
  JS::Rooted<JS::Value> tagLen(aCx, JS::Int32Value(mTagLen));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "tagLength", tagLen))) {
    aErr.Throw(NS_ERROR_FAILURE);
    return;
  }

  JS::Rooted<JS::Value> name(aCx, JS::StringValue(JS_NewStringCopyZ(aCx, WEBCRYPTO_ALG_AES_GCM)));
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", name))) {
    aErr.Throw(NS_ERROR_FAILURE);
    return;
  }
  aRetVal.set(obj);
}

nsresult BerytusAesGcmParams_Impl::ToJSON(BerytusEncryptionParamsJSON& aRv) {
  nsAutoCString ivBase64Url, additionalDataBase64Url;
  nsresult res;
  aRv.mName.Assign(NS_ConvertASCIItoUTF16(WEBCRYPTO_ALG_AES_GCM));
  res = Base64URLEncode(
      mIv.Length(), mIv.Elements(),
      Base64URLEncodePaddingPolicy::Omit, ivBase64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    return NS_ERROR_DOM_SECURITY_ERR;
  }
  aRv.mIv = NS_ConvertASCIItoUTF16(ivBase64Url);

  if (mAdditionalData.Length() > 0) {
    res = Base64URLEncode(
      mAdditionalData.Length(), mAdditionalData.Elements(),
      Base64URLEncodePaddingPolicy::Omit, additionalDataBase64Url);
    if (NS_WARN_IF(NS_FAILED(res))) {
      return NS_ERROR_DOM_SECURITY_ERR;
    }
    aRv.mAdditionalData.Construct(NS_ConvertASCIItoUTF16(additionalDataBase64Url));
  }

  aRv.mTagLength.Construct(mTagLen);
  return NS_OK;
}

BerytusEncryptionParams_Impl* BerytusAesGcmParams_Impl::Clone(nsresult* aRv) {
  CryptoBuffer copiedIv;
  CryptoBuffer copiedAddData;
  if (!copiedIv.Assign(mIv)) {
    *aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  if (!copiedAddData.Assign(mAdditionalData)) {
    *aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  *aRv = NS_OK;
  return new BerytusAesGcmParams_Impl(std::move(copiedIv),
                                      std::move(copiedAddData),
                                      mTagLen);
}

BerytusEncryptedPacket::BerytusEncryptedPacket(
  nsIGlobalObject* aGlobal,
  BerytusEncryptionParams_Impl* aParams,
  CryptoBuffer&& aCiphertext
) : mGlobal(aGlobal),
    mParams(aParams),
    mCiphertext(std::move(aCiphertext)),
    mCachedParams(nullptr),
    mCachedCiphertextArrayBuffer(nullptr)
{
  mozilla::HoldJSObjects(this);
}

BerytusEncryptedPacket::BerytusEncryptedPacket(
  BerytusEncryptedPacket&& aOther) : mGlobal(std::move(aOther.mGlobal)),
                                     mParams(std::move(aOther.mParams)),
                                     mCiphertext(std::move(aOther.mCiphertext)),
                                     mCachedParams(std::move(aOther.mCachedParams)),
                                     mCachedCiphertextArrayBuffer(std::move(aOther.mCachedCiphertextArrayBuffer)) {
  mozilla::HoldJSObjects(this);
  aOther.mParams = nullptr; // do not permit the moved packet free the params.
}

BerytusEncryptedPacket::~BerytusEncryptedPacket() {
  mozilla::DropJSObjects(this);
  delete mParams;
  // Add |MOZ_COUNT_DTOR(BerytusEncryptedPacket);| for a non-refcounted object.
}

already_AddRefed<BerytusEncryptedPacket> BerytusEncryptedPacket::Constructor(
  const GlobalObject& aGlobal,
  const BerytusEncryptionParams& aParamsDict,
  const ArrayBufferViewOrArrayBuffer& aCiphertext,
  ErrorResult& aErr
) {
  nsresult res;
  nsCOMPtr<nsIGlobalObject> global = do_QueryInterface(aGlobal.GetAsSupports());
  if (!global) {
    aErr.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  CryptoBuffer ciph;
  if (!ciph.Assign(aCiphertext)) {
    aErr.Throw(NS_ERROR_DOM_SECURITY_ERR);
    return nullptr;
  }

  BerytusEncryptionParams_Impl* encParams = BerytusAesGcmParams_Impl::FromDictionary(
    aParamsDict,
    res
  );
  if (NS_WARN_IF(NS_FAILED(res))) {
    aErr.Throw(res);
    return nullptr;
  }

  RefPtr<BerytusEncryptedPacket> obj = new BerytusEncryptedPacket(
    global,
    encParams,
    std::move(ciph)
  );
  return obj.forget();
}

void BerytusEncryptedPacket::GetParameters(JSContext* aCx,
                    JS::MutableHandle<JSObject*> aRetVal,
                    ErrorResult& aErr) {
  if (! mCachedParams) {
    //JS::Rooted<JSObject*> obj(aCx);
    // mParams->AsDictionary(aCx, &obj, aErr);
    mParams->AsDictionary(aCx, mCachedParams, aRetVal, aErr);
    if (NS_WARN_IF(aErr.Failed())) {
      return;
    }
    //mCachedParams.set(obj);
    MOZ_ASSERT(mCachedParams);
    return;
  }
  aRetVal.set(mCachedParams);
}

void BerytusEncryptedPacket::GetCiphertext(
  JSContext* aCx,
  JS::MutableHandle<JSObject*> aRetVal,
  ErrorResult& aErr
) {
  if (! mCachedCiphertextArrayBuffer) {
    mCachedCiphertextArrayBuffer = ArrayBuffer::Create(aCx, mCiphertext, aErr);
    if (NS_WARN_IF(aErr.Failed())) {
      return;
    }
  }
  aRetVal.set(mCachedCiphertextArrayBuffer);
}

void BerytusEncryptedPacket::ToJSON(BerytusEncryptedPacketJSON& aRetVal, ErrorResult& aErr) {
  nsAutoCString ciphertextBase64Url;
  nsresult res;
  res = Base64URLEncode(
      mCiphertext.Length(), mCiphertext.Elements(),
      Base64URLEncodePaddingPolicy::Omit, ciphertextBase64Url);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aErr.Throw(res);
    return;
  }
  aRetVal.mCiphertext = NS_ConvertASCIItoUTF16(ciphertextBase64Url);
  BerytusEncryptionParamsJSON paramsJson;
  res = mParams->ToJSON(aRetVal.mParameters);
  if (NS_WARN_IF(NS_FAILED(res))) {
    aErr.Throw(res);
    return;
  }
}

nsIGlobalObject* BerytusEncryptedPacket::GetParentObject() const { return mGlobal; }

JSObject*
BerytusEncryptedPacket::WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto)
{
  return BerytusEncryptedPacket_Binding::Wrap(aCx, this, aGivenProto);
}

already_AddRefed<BerytusEncryptedPacket> BerytusEncryptedPacket::Clone(nsresult* aRv)
{
  BerytusEncryptionParams_Impl* copiedParams = mParams->Clone(aRv);
  if (*aRv != NS_OK) {
    return nullptr;
  }
  CryptoBuffer copiedCiphertext;
  if (!copiedCiphertext.Assign(mCiphertext)) {
    *aRv = NS_ERROR_OUT_OF_MEMORY;
    return nullptr;
  }
  RefPtr<BerytusEncryptedPacket> packet = new BerytusEncryptedPacket(
    mGlobal,
    copiedParams,
    std::move(copiedCiphertext)
  );
  *aRv = NS_OK;
  return packet.forget();
}

} // namespace mozilla::dom
