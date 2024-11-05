/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSBUFFER_H_
#define DOM_BERYTUSBUFFER_H_

#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
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

  void Get(JSContext* aCx,
           OwningArrayBufferOrBerytusEncryptedPacket& aRetVal,
           ErrorResult& aRv);

  void ToJSON(JSContext* aCx,
              JS::MutableHandle<JS::Value> aRetVal,
              ErrorResult& aRv);
};

}  // namespace dom
}  // namespace mozilla

#endif