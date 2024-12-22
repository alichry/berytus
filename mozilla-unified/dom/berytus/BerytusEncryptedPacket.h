/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSENCRYPTEDPACKET_H_
#define DOM_BERYTUSENCRYPTEDPACKET_H_

#include "js/TypeDecls.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/SubtleCryptoBinding.h"
#include "mozilla/dom/UnionTypes.h" // ArrayBufferViewOrArrayBuffer
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/CryptoBuffer.h"

namespace mozilla::dom {

struct AesGcmParamsJSON;
struct BerytusEncryptedPacketJSON;
// typedefs in WebIDL, for one member, are not translated in C++
// once there's a union, remove these.
using BerytusEncryptionParams = AesGcmParams;
using BerytusEncryptionParamsJSON = AesGcmParamsJSON;

class BerytusEncryptionParams_Impl {
public:
  virtual ~BerytusEncryptionParams_Impl() = 0;
  virtual void GetAlgorithm(nsString& aRv) = 0;
  virtual void AsDictionary(JSContext* aCx,
                            JS::Heap<JSObject*>& aObj,
                            JS::MutableHandle<JSObject*> aRetVal,
                            ErrorResult& aErr) = 0;
  virtual nsresult ToJSON(BerytusEncryptionParamsJSON& aRv) = 0;
  virtual BerytusEncryptionParams_Impl* Clone(nsresult* aRv) = 0;
};

class BerytusAesGcmParams_Impl final : public BerytusEncryptionParams_Impl {
public:
  BerytusAesGcmParams_Impl(CryptoBuffer&& aIv,
                      CryptoBuffer&& aAdditionalData,
                      const uint8_t& aTagLen);
  BerytusAesGcmParams_Impl(BerytusAesGcmParams_Impl&& aOther);
  ~BerytusAesGcmParams_Impl();
  void GetAlgorithm(nsString& aRv) override;
  void AsDictionary(JSContext* aCx,
                    JS::Heap<JSObject*>& aObj,
                    JS::MutableHandle<JSObject*> aRetVal,
                    ErrorResult& aErr) override;
  nsresult ToJSON(BerytusEncryptionParamsJSON& aRv) override;
  static BerytusAesGcmParams_Impl* FromDictionary(const AesGcmParams& aDict,
                                                  nsresult& aRv);

  BerytusEncryptionParams_Impl* Clone(nsresult* aRv) override;
protected:
  CryptoBuffer mIv;
  CryptoBuffer mAdditionalData;
  uint8_t mTagLen;
};

// let x, p, v;
// x = (new Uint8Array([1])).buffer; p = { name: "AES-GCM", iv: (new Uint8Array([1,2])).buffer, tagLength: 128 }; v = new BerytusEncryptedPacket(p, x);

class BerytusEncryptedPacket final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                                     public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  //NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusEncryptedPacket)
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusEncryptedPacket)

public:
  BerytusEncryptedPacket(
    nsIGlobalObject* aGlobal,
    BerytusEncryptionParams_Impl* aParams,
    CryptoBuffer&& aCiphertext
  );
  BerytusEncryptedPacket(BerytusEncryptedPacket&& aOther);

protected:
  ~BerytusEncryptedPacket();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  BerytusEncryptionParams_Impl* mParams;
  CryptoBuffer mCiphertext;

  JS::Heap<JSObject*> mCachedParams;
  JS::Heap<JSObject*> mCachedCiphertextArrayBuffer;

public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  static already_AddRefed<BerytusEncryptedPacket> Constructor(
    const GlobalObject& aGlobal,
    const BerytusEncryptionParams& aParamsDict,
    const ArrayBufferViewOrArrayBuffer& aCiphertext,
    ErrorResult& aErr
  );

  void GetCiphertext(
    JSContext* aCx,
    JS::MutableHandle<JSObject*> aRetVal,
    ErrorResult& aErr
  );

  void GetParameters(JSContext* aCx,
                    JS::MutableHandle<JSObject*> aRetVal,
                    ErrorResult& aErr);

  void ToJSON(BerytusEncryptedPacketJSON& aRetVal, ErrorResult& aErr);

  already_AddRefed<BerytusEncryptedPacket> Clone(nsresult* aRv);
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSENCRYPTEDPACKET_H_
