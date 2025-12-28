/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSCHANNELCONTAINER_H_
#define DOM_BERYTUSCHANNELCONTAINER_H_

#include "BerytusEncryptedPacket.h"
#include "js/TypeDecls.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/MozPromise.h"
#include "mozilla/berytus/MaskManagerChild.h"
#include "mozilla/berytus/PUnmaskerChild.h"
#include "nsIGlobalObject.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "nsCycleCollectionParticipant.h"
#include "nsWrapperCache.h"

namespace mozilla::berytus {
class UnmaskerChild;
}

namespace mozilla::dom {

class InternalRequest;
class OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
namespace fetch {
  using OwningBodyInit = OwningBlobOrArrayBufferViewOrArrayBufferOrFormDataOrURLSearchParamsOrUSVString;
}

using PacketObserver = mozilla::dom::BerytusEncryptedPacket::PacketObserver;

// we need to create a channel observer
// and we need to access the global/inner window's
// BerytusChannelContainer.

class BerytusChannelContainer final : public nsISupports,
                                      public nsWrapperCache
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusChannelContainer)

public:
  BerytusChannelContainer(nsIGlobalObject* aGlobal);

  static already_AddRefed<BerytusChannelContainer> GetInstance(
      nsPIDOMWindowInner* aWindow);

  void HoldObserver(RefPtr<PacketObserver>& aObserver);

  bool IsSignedUrl(const nsCString& aReqUrl) const;

  RefPtr<mozilla::berytus::MaskManagerChild> GetMaskManager();
  // TODO(berytus): Remove this method; use manager directly.
  RefPtr<mozilla::berytus::UnmaskerChild> CreateUnmasker();

protected:
  ~BerytusChannelContainer();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  nsTArray<RefPtr<PacketObserver>> mPacketObservers;
  RefPtr<mozilla::berytus::MaskManagerChild> mMaskManager;
public:
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSCHANNELCONTAINER_H_
