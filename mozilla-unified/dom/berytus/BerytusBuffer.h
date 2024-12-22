/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSBUFFER_H_
#define DOM_BERYTUSBUFFER_H_

#include "ErrorList.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/CryptoBuffer.h"
#include "mozilla/dom/TypedArray.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "nsDebug.h"
#include "nsISupports.h"

namespace mozilla {

namespace dom {

class BerytusBuffer final : public nsISupports /* or NonRefcountedDOMObject if this is a
                            non-refcounted object */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(BerytusBuffer)

  BerytusBuffer(CryptoBuffer&& aBuffer);
  BerytusBuffer(const RefPtr<BerytusEncryptedPacket>& aPacket);
protected:
  ~BerytusBuffer();

  RefPtr<BerytusEncryptedPacket> mAsPacket;
  CryptoBuffer mAsBuffer;
  JS::Heap<JSObject*> mCachedBuffer;

 public:
  static already_AddRefed<BerytusBuffer> FromArrayBuffer(
    const ArrayBuffer& aValue,
    nsresult& aRv
  );
  static already_AddRefed<BerytusBuffer> FromArrayBufferView(
    const ArrayBufferView& aValue,
    nsresult& aRv
  );

  template<typename... T>
  static already_AddRefed<BerytusBuffer> FromVariant(
    nsIGlobalObject* aGlobal,
    const Variant<T...>& aValue,
    nsresult& aRv
  );

  void Get(JSContext* aCx,
           OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
           ErrorResult& aRv);

  void ToJSON(JSContext* aCx,
              JS::MutableHandle<JS::Value> aRetVal,
              ErrorResult& aRv);


  already_AddRefed<BerytusBuffer> Clone(nsresult* aRv) const;
  
public:

  struct CreateMatcher {
    nsIGlobalObject* mGlobal;
    nsresult mRv;

    CreateMatcher(nsIGlobalObject* aGlobal) : mGlobal(aGlobal), mRv(NS_OK) {}

    already_AddRefed<BerytusBuffer> operator()(const ArrayBuffer& aValue) {
      return FromArrayBuffer(aValue, mRv);
    }
    already_AddRefed<BerytusBuffer> operator()(const ArrayBufferView& aValue) {
      return FromArrayBufferView(aValue, mRv);
    }
    already_AddRefed<BerytusBuffer> operator()(const RefPtr<BerytusEncryptedPacket>& aPacket) {
      RefPtr<BerytusBuffer> buff = new BerytusBuffer(aPacket);
      return buff.forget();
    }
  };
};

}  // namespace dom
}  // namespace mozilla

#endif