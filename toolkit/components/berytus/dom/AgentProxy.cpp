/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "mozilla/berytus/AgentProxy.h"
#include "js/PropertyAndElement.h"
#include "js/String.h"
#include "js/Value.h"
#include "mozilla/dom/JSActorService.h"
#include "mozilla/dom/WindowGlobalChild.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/JSWindowActorChild.h"
#include "mozilla/dom/Promise.h"
#include "mozilla/dom/Promise-inl.h"

namespace mozilla::berytus {

NS_IMPL_CYCLE_COLLECTION(AgentProxy, mGlobal)
NS_IMPL_CYCLE_COLLECTING_ADDREF(AgentProxy)
NS_IMPL_CYCLE_COLLECTING_RELEASE(AgentProxy)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(AgentProxy)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

AgentProxy::AgentProxy(
    nsIGlobalObject* aGlobal, const nsAString& aManagerId)
    : mGlobal(aGlobal), mManagerId(aManagerId) {}

AgentProxy::~AgentProxy() {}

bool JSValIsInt32(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isInt32();
  return true;
}
bool Int32FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, int32_t& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  aRv = aValue.toInt32();
  return true;
}
bool Int32ToJSVal(JSContext* aCx, const int32_t& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool DocumentMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsInt32(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool DocumentMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Int32FromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  
  return true;
}
            
bool DocumentMetadata::ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!Int32ToJSVal(aCx, aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool PreliminaryRequestContext::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool PreliminaryRequestContext::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument))) {
    return false;
  }
  
  return true;
}
            
bool PreliminaryRequestContext::ToJSVal(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsString(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isString();
  return true;
}
bool StringFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  JSString* str = aValue.toString();
  JS::AutoCheckCannotGC nogc;
  size_t len;
  const char16_t* buf =
    JS_GetTwoByteStringCharsAndLength(aCx, nogc, str, &len);
  aRv = nsString(buf, len);
  return true;
}
bool StringToJSVal(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.get(), aValue.Length()));
  aRv.setString(rStr);
  return true;
}
bool CryptoActor::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool CryptoActor::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CryptoActor& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mEd25519Key))) {
    return false;
  }
  
  return true;
}
            
bool CryptoActor::ToJSVal(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mEd25519Key, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "ed25519Key", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool UriParams::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsInt32(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool UriParams::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mUri))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mScheme))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mHostname))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Int32FromJSVal(aCx, propVal, aRv.mPort))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mPath))) {
    return false;
  }
  
  return true;
}
            
bool UriParams::ToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mUri, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "uri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mScheme, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scheme", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mHostname, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hostname", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!Int32ToJSVal(aCx, aValue.mPort, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "port", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mPath, &memberVal4))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "path", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool OriginActor::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!UriParams::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!UriParams::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool OriginActor::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!UriParams::FromJSVal(aCx, propVal, aRv.mOriginalUri))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!UriParams::FromJSVal(aCx, propVal, aRv.mCurrentUri))) {
    return false;
  }
  
  return true;
}
            
bool OriginActor::ToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!UriParams::ToJSVal(aCx, aValue.mOriginalUri, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "originalUri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!UriParams::ToJSVal(aCx, aValue.mCurrentUri, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "currentUri", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsVariant_CryptoActor__OriginActor_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  bool isValid = false;
  
  if (NS_WARN_IF(!CryptoActor::IsJSValueValid(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }


  if (NS_WARN_IF(!OriginActor::IsJSValueValid(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }

  aRv = false;
  return true;
}
bool Variant_CryptoActor__OriginActor_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<CryptoActor, OriginActor>** aRv) {
  bool isValid = false;
  if (NS_WARN_IF(!CryptoActor::IsJSValueValid(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    CryptoActor nv;
    if (NS_WARN_IF(!CryptoActor::FromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<CryptoActor, OriginActor>(nv);
    return true;
  }
if (NS_WARN_IF(!OriginActor::IsJSValueValid(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    OriginActor nv;
    if (NS_WARN_IF(!OriginActor::FromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<CryptoActor, OriginActor>(nv);
    return true;
  }

  NS_WARN_IF(true);
  return false;
}
struct Variant_CryptoActor__OriginActor_ToJSValMatcher {
  JSContext* mCx;
  JS::MutableHandle<JS::Value> mRv;
  Variant_CryptoActor__OriginActor_ToJSValMatcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}

  bool operator()(const CryptoActor& aVal) {
    return CryptoActor::ToJSVal(mCx, aVal, mRv);
  }

  bool operator()(const OriginActor& aVal) {
    return OriginActor::ToJSVal(mCx, aVal, mRv);
  }
};
bool Variant_CryptoActor__OriginActor_ToJSVal(JSContext* aCx, const Variant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv) {
  return aValue.match(Variant_CryptoActor__OriginActor_ToJSValMatcher(aCx, aRv));
}
bool GetSigningKeyArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_CryptoActor__OriginActor_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool GetSigningKeyArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetSigningKeyArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_FromJSVal(aCx, propVal, &aRv.mWebAppActor))) {
    return false;
  }
  
  return true;
}
            
bool GetSigningKeyArgs::ToJSVal(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_ToJSVal(aCx, *aValue.mWebAppActor, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool JSValIsMaybe_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIsString(aCx, aValue, aRv);
}
bool Maybe_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsString>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_nsString_ToJSVal(JSContext* aCx, const Maybe<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return StringToJSVal(aCx, aValue.ref(), aRv);
}
bool JSValIsMaybe_int32_t_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIsInt32(aCx, aValue, aRv);
}
bool Maybe_int32_t_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<int32_t>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!Int32FromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_int32_t_ToJSVal(JSContext* aCx, const Maybe<int32_t>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return Int32ToJSVal(aCx, aValue.ref(), aRv);
}
bool PartialAccountIdentity::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool PartialAccountIdentity::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialAccountIdentity& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mFieldId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mFieldValue))) {
    return false;
  }
  
  return true;
}
            
bool PartialAccountIdentity::ToJSVal(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mFieldId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mFieldValue, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldValue", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsnsTArray_PartialAccountIdentity_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  return true;
}
bool nsTArray_PartialAccountIdentity_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<PartialAccountIdentity>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isArray;
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, obj, &isArray))) {
    return false;
  }
  if (NS_WARN_IF(!isArray)) {
    return false;
  }
  uint32_t length;
  if (NS_WARN_IF(!JS::GetArrayLength(aCx, obj, &length))) {
    return false;
  }
  for (uint32_t i = 0; i < length; i++) {
    JS::Rooted<JS::Value> value(aCx);

    if (NS_WARN_IF(!JS_GetElement(aCx, obj, i, &value))) {
      return false;
    }

    PartialAccountIdentity item;
    if (NS_WARN_IF(!PartialAccountIdentity::FromJSVal(aCx, value, item))) {
      return false;
    }
    aRv.AppendElement(item);
  }
  return true;
}
bool nsTArray_PartialAccountIdentity_ToJSVal(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const PartialAccountIdentity& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!PartialAccountIdentity::ToJSVal(aCx, item, &value))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
bool JSValIsMaybe_nsTArray_PartialAccountIdentity__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIsnsTArray_PartialAccountIdentity_(aCx, aValue, aRv);
}
bool Maybe_nsTArray_PartialAccountIdentity__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<PartialAccountIdentity>>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!nsTArray_PartialAccountIdentity_FromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_nsTArray_PartialAccountIdentity__ToJSVal(JSContext* aCx, const Maybe<nsTArray<PartialAccountIdentity>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return nsTArray_PartialAccountIdentity_ToJSVal(aCx, aValue.ref(), aRv);
}
bool AccountConstraints::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_nsString_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_int32_t_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_nsTArray_PartialAccountIdentity__(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool AccountConstraints::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AccountConstraints& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_nsString_FromJSVal(aCx, propVal, aRv.mCategory))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_int32_t_FromJSVal(aCx, propVal, aRv.mSchemaVersion))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_nsTArray_PartialAccountIdentity__FromJSVal(aCx, propVal, aRv.mIdentity))) {
    return false;
  }
  
  return true;
}
            
bool AccountConstraints::ToJSVal(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!Maybe_nsString_ToJSVal(aCx, aValue.mCategory, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!Maybe_int32_t_ToJSVal(aCx, aValue.mSchemaVersion, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "schemaVersion", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!Maybe_nsTArray_PartialAccountIdentity__ToJSVal(aCx, aValue.mIdentity, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "identity", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool GetCredentialsMetadataArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_CryptoActor__OriginActor_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!AccountConstraints::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool GetCredentialsMetadataArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_FromJSVal(aCx, propVal, &aRv.mWebAppActor))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!AccountConstraints::FromJSVal(aCx, propVal, aRv.mAccountConstraints))) {
    return false;
  }
  
  return true;
}
            
bool GetCredentialsMetadataArgs::ToJSVal(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_ToJSVal(aCx, *aValue.mWebAppActor, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!AccountConstraints::ToJSVal(aCx, aValue.mAccountConstraints, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "accountConstraints", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool JSValIsnsTArray_nsString_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  return true;
}
bool nsTArray_nsString_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<nsString>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isArray;
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, obj, &isArray))) {
    return false;
  }
  if (NS_WARN_IF(!isArray)) {
    return false;
  }
  uint32_t length;
  if (NS_WARN_IF(!JS::GetArrayLength(aCx, obj, &length))) {
    return false;
  }
  for (uint32_t i = 0; i < length; i++) {
    JS::Rooted<JS::Value> value(aCx);

    if (NS_WARN_IF(!JS_GetElement(aCx, obj, i, &value))) {
      return false;
    }

    nsString item;
    if (NS_WARN_IF(!StringFromJSVal(aCx, value, item))) {
      return false;
    }
    aRv.AppendElement(item);
  }
  return true;
}
bool nsTArray_nsString_ToJSVal(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const nsString& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!StringToJSVal(aCx, item, &value))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
bool JSValIsMaybe_nsTArray_nsString__(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIsnsTArray_nsString_(aCx, aValue, aRv);
}
bool Maybe_nsTArray_nsString__FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<nsString>>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!nsTArray_nsString_FromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_nsTArray_nsString__ToJSVal(JSContext* aCx, const Maybe<nsTArray<nsString>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return nsTArray_nsString_ToJSVal(aCx, aValue.ref(), aRv);
}
bool JSValIsBool(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isBoolean();
  return true;
}
bool BoolFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isBoolean())) {
    return false;
  }
  aRv = aValue.toBoolean();
  return true;
}
bool BoolToJSVal(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setBoolean(aValue);
  return true;
}
bool JSValIsMaybe_AccountConstraints_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return AccountConstraints::IsJSValueValid(aCx, aValue, aRv);
}
bool Maybe_AccountConstraints_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<AccountConstraints>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!AccountConstraints::FromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_AccountConstraints_ToJSVal(JSContext* aCx, const Maybe<AccountConstraints>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return AccountConstraints::ToJSVal(aCx, aValue.ref(), aRv);
}
bool ChannelConstraints::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_nsTArray_nsString__(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsBool(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_AccountConstraints_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChannelConstraints::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_nsTArray_nsString__FromJSVal(aCx, propVal, aRv.mSecretManagerPublicKey))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BoolFromJSVal(aCx, propVal, aRv.mEnableEndToEndEncryption))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_AccountConstraints_FromJSVal(aCx, propVal, aRv.mAccount))) {
    return false;
  }
  
  return true;
}
            
bool ChannelConstraints::ToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!Maybe_nsTArray_nsString__ToJSVal(aCx, aValue.mSecretManagerPublicKey, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "secretManagerPublicKey", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!BoolToJSVal(aCx, aValue.mEnableEndToEndEncryption, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "enableEndToEndEncryption", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!Maybe_AccountConstraints_ToJSVal(aCx, aValue.mAccount, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "account", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool ChannelMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelConstraints::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_CryptoActor__OriginActor_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!CryptoActor::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChannelMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelConstraints::FromJSVal(aCx, propVal, aRv.mConstraints))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_FromJSVal(aCx, propVal, &aRv.mWebAppActor))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!CryptoActor::FromJSVal(aCx, propVal, aRv.mScmActor))) {
    return false;
  }
  
  return true;
}
            
bool ChannelMetadata::ToJSVal(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ChannelConstraints::ToJSVal(aCx, aValue.mConstraints, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "constraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_CryptoActor__OriginActor_ToJSVal(aCx, *aValue.mWebAppActor, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!CryptoActor::ToJSVal(aCx, aValue.mScmActor, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmActor", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool RequestContext::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RequestContext::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelMetadata::FromJSVal(aCx, propVal, aRv.mChannel))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument))) {
    return false;
  }
  
  return true;
}
            
bool RequestContext::ToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChannelMetadata::ToJSVal(aCx, aValue.mChannel, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool InitialKeyExchangeParametersDraft::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool InitialKeyExchangeParametersDraft::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, InitialKeyExchangeParametersDraft& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mChannelId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mWebAppX25519Key))) {
    return false;
  }
  
  return true;
}
            
bool InitialKeyExchangeParametersDraft::ToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mChannelId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mWebAppX25519Key, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppX25519Key", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool GenerateKeyExchangeParametersArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "paramsDraft", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!InitialKeyExchangeParametersDraft::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool GenerateKeyExchangeParametersArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, GenerateKeyExchangeParametersArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "paramsDraft", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!InitialKeyExchangeParametersDraft::FromJSVal(aCx, propVal, aRv.mParamsDraft))) {
    return false;
  }
  
  return true;
}
            
bool GenerateKeyExchangeParametersArgs::ToJSVal(JSContext* aCx, const GenerateKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!InitialKeyExchangeParametersDraft::ToJSVal(aCx, aValue.mParamsDraft, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "paramsDraft", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsArrayBuffer(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS::IsArrayBufferObject(obj);
  return true;
}
bool ArrayBufferFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBuffer& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  if (NS_WARN_IF(!JS::IsArrayBufferObject(obj))) {
    return false;
  }
  size_t length = 0u;
  uint8_t* bytes = nullptr;
  if (NS_WARN_IF(!JS::GetObjectAsArrayBuffer(obj, &length, &bytes))) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}
bool ArrayBufferToJSVal(JSContext* aCx, const ArrayBuffer& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setObject(*aValue.Obj());
  return true;
}
bool PartialKeyExchangeParametersFromScm::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsInt32(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool PartialKeyExchangeParametersFromScm::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialKeyExchangeParametersFromScm& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mScmX25519Key))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mHkdfHash))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfSalt))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfInfo))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Int32FromJSVal(aCx, propVal, aRv.mAesKeyLength))) {
    return false;
  }
  
  return true;
}
            
bool PartialKeyExchangeParametersFromScm::ToJSVal(JSContext* aCx, const PartialKeyExchangeParametersFromScm& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mScmX25519Key, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmX25519Key", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mHkdfHash, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfHash", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!ArrayBufferToJSVal(aCx, aValue.mHkdfSalt, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfSalt", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!ArrayBufferToJSVal(aCx, aValue.mHkdfInfo, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfInfo", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!Int32ToJSVal(aCx, aValue.mAesKeyLength, &memberVal4))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "aesKeyLength", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool KeyExchangeParameters::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "packet", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsInt32(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool KeyExchangeParameters::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, KeyExchangeParameters& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "packet", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mPacket))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mChannelId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mWebAppX25519Key))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mScmX25519Key))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mHkdfHash))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfSalt))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfInfo))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Int32FromJSVal(aCx, propVal, aRv.mAesKeyLength))) {
    return false;
  }
  
  return true;
}
            
bool KeyExchangeParameters::ToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mPacket, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "packet", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mChannelId, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelId", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mWebAppX25519Key, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppX25519Key", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mScmX25519Key, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmX25519Key", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mHkdfHash, &memberVal4))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfHash", memberVal4))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal5(aCx);
  
  if (NS_WARN_IF(!ArrayBufferToJSVal(aCx, aValue.mHkdfSalt, &memberVal5))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfSalt", memberVal5))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal6(aCx);
  
  if (NS_WARN_IF(!ArrayBufferToJSVal(aCx, aValue.mHkdfInfo, &memberVal6))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfInfo", memberVal6))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal7(aCx);
  
  if (NS_WARN_IF(!Int32ToJSVal(aCx, aValue.mAesKeyLength, &memberVal7))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "aesKeyLength", memberVal7))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool EnableEndToEndEncryptionArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "params", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!KeyExchangeParameters::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppPacketSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool EnableEndToEndEncryptionArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EnableEndToEndEncryptionArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "params", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!KeyExchangeParameters::FromJSVal(aCx, propVal, aRv.mParams))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppPacketSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, propVal, aRv.mWebAppPacketSignature))) {
    return false;
  }
  
  return true;
}
            
bool EnableEndToEndEncryptionArgs::ToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!KeyExchangeParameters::ToJSVal(aCx, aValue.mParams, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "params", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ArrayBufferToJSVal(aCx, aValue.mWebAppPacketSignature, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppPacketSignature", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}



bool JSValIsELoginUserIntent(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(ELoginUserIntent::ELoginUserIntent_EndGuard_)) {
    return false;
  }
  return true;
}
bool ELoginUserIntentFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(ELoginUserIntent::ELoginUserIntent_EndGuard_)) {
    return false;
  }
  aRv = static_cast<ELoginUserIntent>(aValue.toInt32());
  return true;
}
bool ELoginUserIntentToJSVal(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool JSValIsVariant_nsString_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  bool isValid = false;
  
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }

  aRv = false;
  return true;
}
bool Variant_nsString_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString>** aRv) {
  bool isValid = false;
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    nsString nv;
    if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<nsString>(nv);
    return true;
  }

  NS_WARN_IF(true);
  return false;
}
struct Variant_nsString_ToJSValMatcher {
  JSContext* mCx;
  JS::MutableHandle<JS::Value> mRv;
  Variant_nsString_ToJSValMatcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}

  bool operator()(const nsString& aVal) {
    return StringToJSVal(mCx, aVal, mRv);
  }
};
bool Variant_nsString_ToJSVal(JSContext* aCx, const Variant<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  return aValue.match(Variant_nsString_ToJSValMatcher(aCx, aRv));
}
bool RequestedUserAttribute::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_nsString_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsBool(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RequestedUserAttribute::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString_FromJSVal(aCx, propVal, &aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BoolFromJSVal(aCx, propVal, aRv.mRequired))) {
    return false;
  }
  
  return true;
}
            
bool RequestedUserAttribute::ToJSVal(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mId)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString_ToJSVal(aCx, *aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!BoolToJSVal(aCx, aValue.mRequired, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "required", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsnsTArray_RequestedUserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  return true;
}
bool nsTArray_RequestedUserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<RequestedUserAttribute>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isArray;
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, obj, &isArray))) {
    return false;
  }
  if (NS_WARN_IF(!isArray)) {
    return false;
  }
  uint32_t length;
  if (NS_WARN_IF(!JS::GetArrayLength(aCx, obj, &length))) {
    return false;
  }
  for (uint32_t i = 0; i < length; i++) {
    JS::Rooted<JS::Value> value(aCx);

    if (NS_WARN_IF(!JS_GetElement(aCx, obj, i, &value))) {
      return false;
    }

    RequestedUserAttribute item;
    if (NS_WARN_IF(!RequestedUserAttribute::FromJSVal(aCx, value, item))) {
      return false;
    }
    aRv.AppendElement(item);
  }
  return true;
}
bool nsTArray_RequestedUserAttribute_ToJSVal(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const RequestedUserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!RequestedUserAttribute::ToJSVal(aCx, item, &value))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
bool JSValIsEOpeationType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EOpeationType::EOpeationType_EndGuard_)) {
    return false;
  }
  return true;
}
bool EOpeationTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOpeationType& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EOpeationType::EOpeationType_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EOpeationType>(aValue.toInt32());
  return true;
}
bool EOpeationTypeToJSVal(JSContext* aCx, const EOpeationType& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool JSValIsEOperationStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EOperationStatus::EOperationStatus_EndGuard_)) {
    return false;
  }
  return true;
}
bool EOperationStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EOperationStatus::EOperationStatus_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EOperationStatus>(aValue.toInt32());
  return true;
}
bool EOperationStatusToJSVal(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool OperationState::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  aRv = true;
  return true;
}
bool OperationState::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  return true;
}
            
bool OperationState::ToJSVal(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  aRv.setObject(*obj);
  return true;
}

bool LoginOperationMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsELoginUserIntent(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsnsTArray_RequestedUserAttribute_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEOpeationType(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEOperationStatus(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationState::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool LoginOperationMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ELoginUserIntentFromJSVal(aCx, propVal, aRv.mIntent))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!nsTArray_RequestedUserAttribute_FromJSVal(aCx, propVal, aRv.mRequestedUserAttributes))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOpeationTypeFromJSVal(aCx, propVal, aRv.mType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatusFromJSVal(aCx, propVal, aRv.mStatus))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationState::FromJSVal(aCx, propVal, aRv.mState))) {
    return false;
  }
  
  return true;
}
            
bool LoginOperationMetadata::ToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ELoginUserIntentToJSVal(aCx, aValue.mIntent, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "intent", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!nsTArray_RequestedUserAttribute_ToJSVal(aCx, aValue.mRequestedUserAttributes, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "requestedUserAttributes", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mId, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!EOpeationTypeToJSVal(aCx, aValue.mType, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!EOperationStatusToJSVal(aCx, aValue.mStatus, &memberVal4))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal4))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal5(aCx);
  
  if (NS_WARN_IF(!OperationState::ToJSVal(aCx, aValue.mState, &memberVal5))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "state", memberVal5))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool ApproveOperationArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!LoginOperationMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ApproveOperationArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!LoginOperationMetadata::FromJSVal(aCx, propVal, aRv.mOperation))) {
    return false;
  }
  
  return true;
}
            
bool ApproveOperationArgs::ToJSVal(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!LoginOperationMetadata::ToJSVal(aCx, aValue.mOperation, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool OperationMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEOpeationType(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEOperationStatus(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationState::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool OperationMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOpeationTypeFromJSVal(aCx, propVal, aRv.mType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatusFromJSVal(aCx, propVal, aRv.mStatus))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationState::FromJSVal(aCx, propVal, aRv.mState))) {
    return false;
  }
  
  return true;
}
            
bool OperationMetadata::ToJSVal(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!EOpeationTypeToJSVal(aCx, aValue.mType, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!EOperationStatusToJSVal(aCx, aValue.mStatus, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!OperationState::ToJSVal(aCx, aValue.mState, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "state", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool RequestContextWithOperation::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RequestContextWithOperation::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!OperationMetadata::FromJSVal(aCx, propVal, aRv.mOperation))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelMetadata::FromJSVal(aCx, propVal, aRv.mChannel))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument))) {
    return false;
  }
  
  return true;
}
            
bool RequestContextWithOperation::ToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!OperationMetadata::ToJSVal(aCx, aValue.mOperation, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ChannelMetadata::ToJSVal(aCx, aValue.mChannel, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool JSValIsEMetadataStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EMetadataStatus::EMetadataStatus_EndGuard_)) {
    return false;
  }
  return true;
}
bool EMetadataStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EMetadataStatus::EMetadataStatus_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EMetadataStatus>(aValue.toInt32());
  return true;
}
bool EMetadataStatusToJSVal(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool RecordMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsInt32(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEMetadataStatus(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RecordMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Int32FromJSVal(aCx, propVal, aRv.mVersion))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EMetadataStatusFromJSVal(aCx, propVal, aRv.mStatus))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mCategory))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mChangePassUrl))) {
    return false;
  }
  
  return true;
}
            
bool RecordMetadata::ToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!Int32ToJSVal(aCx, aValue.mVersion, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "version", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!EMetadataStatusToJSVal(aCx, aValue.mStatus, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mCategory, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mChangePassUrl, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "changePassUrl", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool UpdateMetadataArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!RecordMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool UpdateMetadataArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!RecordMetadata::FromJSVal(aCx, propVal, aRv.mMetadata))) {
    return false;
  }
  
  return true;
}
            
bool UpdateMetadataArgs::ToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!RecordMetadata::ToJSVal(aCx, aValue.mMetadata, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "metadata", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool ApproveTransitionToAuthOpArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!LoginOperationMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ApproveTransitionToAuthOpArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!LoginOperationMetadata::FromJSVal(aCx, propVal, aRv.mNewAuthOp))) {
    return false;
  }
  
  return true;
}
            
bool ApproveTransitionToAuthOpArgs::ToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!LoginOperationMetadata::ToJSVal(aCx, aValue.mNewAuthOp, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "newAuthOp", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool UserAttribute::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_nsString_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool UserAttribute::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString_FromJSVal(aCx, propVal, &aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mMimeType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mValue))) {
    return false;
  }
  
  return true;
}
            
bool UserAttribute::ToJSVal(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mId)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString_ToJSVal(aCx, *aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mMimeType, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "mimeType", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mValue, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsnsTArray_UserAttribute_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  return true;
}
bool nsTArray_UserAttribute_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isArray;
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, obj, &isArray))) {
    return false;
  }
  if (NS_WARN_IF(!isArray)) {
    return false;
  }
  uint32_t length;
  if (NS_WARN_IF(!JS::GetArrayLength(aCx, obj, &length))) {
    return false;
  }
  for (uint32_t i = 0; i < length; i++) {
    JS::Rooted<JS::Value> value(aCx);

    if (NS_WARN_IF(!JS_GetElement(aCx, obj, i, &value))) {
      return false;
    }

    UserAttribute item;
    if (NS_WARN_IF(!UserAttribute::FromJSVal(aCx, value, item))) {
      return false;
    }
    aRv.AppendElement(item);
  }
  return true;
}
bool nsTArray_UserAttribute_ToJSVal(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const UserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!UserAttribute::ToJSVal(aCx, item, &value))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}

bool JSValIsEFieldType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EFieldType::EFieldType_EndGuard_)) {
    return false;
  }
  return true;
}
bool EFieldTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EFieldType& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EFieldType::EFieldType_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EFieldType>(aValue.toInt32());
  return true;
}
bool EFieldTypeToJSVal(JSContext* aCx, const EFieldType& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool BaseFieldMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEFieldType(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "description", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsMaybe_nsString_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool BaseFieldMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BaseFieldMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EFieldTypeFromJSVal(aCx, propVal, aRv.mFieldType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mFieldId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "description", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Maybe_nsString_FromJSVal(aCx, propVal, aRv.mDescription))) {
    return false;
  }
  
  return true;
}
            
bool BaseFieldMetadata::ToJSVal(JSContext* aCx, const BaseFieldMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!EFieldTypeToJSVal(aCx, aValue.mFieldType, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldType", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mFieldId, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!Maybe_nsString_ToJSVal(aCx, aValue.mDescription, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "description", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsNothing(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isUndefined();
  return true;
}
bool NothingFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv) {
  if (NS_WARN_IF(!aValue.isUndefined())) {
    return false;
  }
  // Nothing to do...
  return true;
}
bool NothingToJSVal(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setUndefined();
  return true;
}
bool JSValIsVariant_nsString__ArrayBuffer__Nothing_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  bool isValid = false;
  
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }


  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }


  if (NS_WARN_IF(!JSValIsNothing(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }

  aRv = false;
  return true;
}
bool Variant_nsString__ArrayBuffer__Nothing_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer, Nothing>** aRv) {
  bool isValid = false;
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    nsString nv;
    if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<nsString, ArrayBuffer, Nothing>(nv);
    return true;
  }
if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    *aRv = new Variant<nsString, ArrayBuffer, Nothing>(ArrayBuffer());
    if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, aValue, (*aRv)->as<ArrayBuffer>()))) {
      return false;
    }
    return true;
    
  }
if (NS_WARN_IF(!JSValIsNothing(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    Nothing nv;
    if (NS_WARN_IF(!NothingFromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<nsString, ArrayBuffer, Nothing>(nv);
    return true;
  }

  NS_WARN_IF(true);
  return false;
}
struct Variant_nsString__ArrayBuffer__Nothing_ToJSValMatcher {
  JSContext* mCx;
  JS::MutableHandle<JS::Value> mRv;
  Variant_nsString__ArrayBuffer__Nothing_ToJSValMatcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}

  bool operator()(const nsString& aVal) {
    return StringToJSVal(mCx, aVal, mRv);
  }

  bool operator()(const ArrayBuffer& aVal) {
    return ArrayBufferToJSVal(mCx, aVal, mRv);
  }

  bool operator()(const Nothing& aVal) {
    return NothingToJSVal(mCx, aVal, mRv);
  }
};
bool Variant_nsString__ArrayBuffer__Nothing_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv) {
  return aValue.match(Variant_nsString__ArrayBuffer__Nothing_ToJSValMatcher(aCx, aRv));
}
bool AddFieldArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BaseFieldMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_nsString__ArrayBuffer__Nothing_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool AddFieldArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BaseFieldMetadata::FromJSVal(aCx, propVal, aRv.mField))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString__ArrayBuffer__Nothing_FromJSVal(aCx, propVal, &aRv.mValue))) {
    return false;
  }
  
  return true;
}
            
bool AddFieldArgs::ToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!BaseFieldMetadata::ToJSVal(aCx, aValue.mField, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "field", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  if (NS_WARN_IF(!aValue.mValue)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString__ArrayBuffer__Nothing_ToJSVal(aCx, *aValue.mValue, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool FieldValueRejectionReason::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool FieldValueRejectionReason::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mCode))) {
    return false;
  }
  
  return true;
}
            
bool FieldValueRejectionReason::ToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mCode, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "code", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool RejectFieldValueArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BaseFieldMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!FieldValueRejectionReason::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsVariant_nsString__ArrayBuffer__Nothing_(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RejectFieldValueArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!BaseFieldMetadata::FromJSVal(aCx, propVal, aRv.mField))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!FieldValueRejectionReason::FromJSVal(aCx, propVal, aRv.mReason))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString__ArrayBuffer__Nothing_FromJSVal(aCx, propVal, &aRv.mOptionalNewValue))) {
    return false;
  }
  
  return true;
}
            
bool RejectFieldValueArgs::ToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!BaseFieldMetadata::ToJSVal(aCx, aValue.mField, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "field", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!FieldValueRejectionReason::ToJSVal(aCx, aValue.mReason, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mOptionalNewValue)) {
    return false;
  }
  if (NS_WARN_IF(!Variant_nsString__ArrayBuffer__Nothing_ToJSVal(aCx, *aValue.mOptionalNewValue, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "optionalNewValue", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsVariant_nsString__ArrayBuffer_(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  bool isValid = false;
  
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }


  if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    aRv = true;
    return true;
  }

  aRv = false;
  return true;
}
bool Variant_nsString__ArrayBuffer_FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Variant<nsString, ArrayBuffer>** aRv) {
  bool isValid = false;
  if (NS_WARN_IF(!JSValIsString(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    nsString nv;
    if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, nv))) {
      return false;
    }
    *aRv = new Variant<nsString, ArrayBuffer>(nv);
    return true;
  }
if (NS_WARN_IF(!JSValIsArrayBuffer(aCx, aValue, isValid))) {
    return false;
  }
  if (isValid) {
    *aRv = new Variant<nsString, ArrayBuffer>(ArrayBuffer());
    if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, aValue, (*aRv)->as<ArrayBuffer>()))) {
      return false;
    }
    return true;
    
  }

  NS_WARN_IF(true);
  return false;
}
struct Variant_nsString__ArrayBuffer_ToJSValMatcher {
  JSContext* mCx;
  JS::MutableHandle<JS::Value> mRv;
  Variant_nsString__ArrayBuffer_ToJSValMatcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}

  bool operator()(const nsString& aVal) {
    return StringToJSVal(mCx, aVal, mRv);
  }

  bool operator()(const ArrayBuffer& aVal) {
    return ArrayBufferToJSVal(mCx, aVal, mRv);
  }
};
bool Variant_nsString__ArrayBuffer_ToJSVal(JSContext* aCx, const Variant<nsString, ArrayBuffer>& aValue, JS::MutableHandle<JS::Value> aRv) {
  return aValue.match(Variant_nsString__ArrayBuffer_ToJSValMatcher(aCx, aRv));
}

bool JSValIsEChallengeType(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EChallengeType::EChallengeType_EndGuard_)) {
    return false;
  }
  return true;
}
bool EChallengeTypeFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeType& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EChallengeType::EChallengeType_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EChallengeType>(aValue.toInt32());
  return true;
}
bool EChallengeTypeToJSVal(JSContext* aCx, const EChallengeType& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool ChallengeParameters::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  aRv = true;
  return true;
}
bool ChallengeParameters::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeParameters& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  return true;
}
            
bool ChallengeParameters::ToJSVal(JSContext* aCx, const ChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsEChallengeStatus(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EChallengeStatus::EChallengeStatus_EndGuard_)) {
    return false;
  }
  return true;
}
bool EChallengeStatusFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeStatus& aRv) {
  if (NS_WARN_IF(!aValue.isInt32())) {
    return false;
  }
  if (aValue.toInt32() >= static_cast<uint8_t>(EChallengeStatus::EChallengeStatus_EndGuard_)) {
    return false;
  }
  aRv = static_cast<EChallengeStatus>(aValue.toInt32());
  return true;
}
bool EChallengeStatusToJSVal(JSContext* aCx, const EChallengeStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setInt32(aValue);
  return true;
}
bool ChallengeMetadata::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEChallengeType(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeParameters::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsEChallengeStatus(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChallengeMetadata::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EChallengeTypeFromJSVal(aCx, propVal, aRv.mType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeParameters::FromJSVal(aCx, propVal, aRv.mParameters))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EChallengeStatusFromJSVal(aCx, propVal, aRv.mStatus))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeMetadata::ToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mId, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!EChallengeTypeToJSVal(aCx, aValue.mType, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!ChallengeParameters::ToJSVal(aCx, aValue.mParameters, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!EChallengeStatusToJSVal(aCx, aValue.mStatus, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool ApproveChallengeRequestArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ApproveChallengeRequestArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge))) {
    return false;
  }
  
  return true;
}
            
bool ApproveChallengeRequestArgs::ToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool ChallengeAbortionReason::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChallengeAbortionReason::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeAbortionReason& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mCode))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeAbortionReason::ToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mCode, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "code", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool AbortChallengeArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeAbortionReason::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool AbortChallengeArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeAbortionReason::FromJSVal(aCx, propVal, aRv.mReason))) {
    return false;
  }
  
  return true;
}
            
bool AbortChallengeArgs::ToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ChallengeAbortionReason::ToJSVal(aCx, aValue.mReason, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool CloseChallengeArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool CloseChallengeArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge))) {
    return false;
  }
  
  return true;
}
            
bool CloseChallengeArgs::ToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool ChallengePayload::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  aRv = true;
  return true;
}
bool ChallengePayload::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengePayload& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  return true;
}
            
bool ChallengePayload::ToJSVal(JSContext* aCx, const ChallengePayload& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  aRv.setObject(*obj);
  return true;
}

bool ChallengeMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!JSValIsString(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengePayload::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChallengeMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!StringFromJSVal(aCx, propVal, aRv.mName))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengePayload::FromJSVal(aCx, propVal, aRv.mPayload))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeMessage::ToJSVal(JSContext* aCx, const ChallengeMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!StringToJSVal(aCx, aValue.mName, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ChallengePayload::ToJSVal(aCx, aValue.mPayload, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool RespondToChallengeMessageArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challengeMessage", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMessage::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool RespondToChallengeMessageArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RespondToChallengeMessageArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "challengeMessage", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengeMessage::FromJSVal(aCx, propVal, aRv.mChallengeMessage))) {
    return false;
  }
  
  return true;
}
            
bool RespondToChallengeMessageArgs::ToJSVal(JSContext* aCx, const RespondToChallengeMessageArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!ChallengeMessage::ToJSVal(aCx, aValue.mChallengeMessage, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challengeMessage", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool ChallengeMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengePayload::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;
}
bool ChallengeMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ChallengeMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChallengePayload::FromJSVal(aCx, propVal, aRv.mPayload))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeMessageResponse::ToJSVal(JSContext* aCx, const ChallengeMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!ChallengePayload::ToJSVal(aCx, aValue.mPayload, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}



RefPtr<ManagerGetSigningKeyResult> AgentProxy::Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs) {
  RefPtr<ManagerGetSigningKeyResult::Private> outPromise = new ManagerGetSigningKeyResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"manager"_ns;
  nsString method = u"getSigningKey"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!PreliminaryRequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!GetSigningKeyArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    nsString out;
    if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<ManagerGetCredentialsMetadataResult> AgentProxy::Manager_GetCredentialsMetadata(PreliminaryRequestContext& aContext, GetCredentialsMetadataArgs& aArgs) {
  RefPtr<ManagerGetCredentialsMetadataResult::Private> outPromise = new ManagerGetCredentialsMetadataResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"manager"_ns;
  nsString method = u"getCredentialsMetadata"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!PreliminaryRequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!GetCredentialsMetadataArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    int32_t out;
    if (NS_WARN_IF(!Int32FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<ChannelGenerateKeyExchangeParametersResult> AgentProxy::Channel_GenerateKeyExchangeParameters(RequestContext& aContext, GenerateKeyExchangeParametersArgs& aArgs) {
  RefPtr<ChannelGenerateKeyExchangeParametersResult::Private> outPromise = new ChannelGenerateKeyExchangeParametersResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"channel"_ns;
  nsString method = u"generateKeyExchangeParameters"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!GenerateKeyExchangeParametersArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    PartialKeyExchangeParametersFromScm out;
    if (NS_WARN_IF(!PartialKeyExchangeParametersFromScm::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<ChannelEnableEndToEndEncryptionResult> AgentProxy::Channel_EnableEndToEndEncryption(RequestContext& aContext, EnableEndToEndEncryptionArgs& aArgs) {
  RefPtr<ChannelEnableEndToEndEncryptionResult::Private> outPromise = new ChannelEnableEndToEndEncryptionResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"channel"_ns;
  nsString method = u"enableEndToEndEncryption"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!EnableEndToEndEncryptionArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ArrayBuffer out;
    if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<ChannelCloseChannelResult> AgentProxy::Channel_CloseChannel(RequestContext& aContext) {
  RefPtr<ChannelCloseChannelResult::Private> outPromise = new ChannelCloseChannelResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"channel"_ns;
  nsString method = u"closeChannel"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<LoginApproveOperationResult> AgentProxy::Login_ApproveOperation(RequestContext& aContext, ApproveOperationArgs& aArgs) {
  RefPtr<LoginApproveOperationResult::Private> outPromise = new LoginApproveOperationResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"login"_ns;
  nsString method = u"approveOperation"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContext::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!ApproveOperationArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ELoginUserIntent out;
    if (NS_WARN_IF(!ELoginUserIntentFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<LoginCloseOpeationResult> AgentProxy::Login_CloseOpeation(RequestContextWithOperation& aContext) {
  RefPtr<LoginCloseOpeationResult::Private> outPromise = new LoginCloseOpeationResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"login"_ns;
  nsString method = u"closeOpeation"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<LoginGetRecordMetadataResult> AgentProxy::Login_GetRecordMetadata(RequestContextWithOperation& aContext) {
  RefPtr<LoginGetRecordMetadataResult::Private> outPromise = new LoginGetRecordMetadataResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"login"_ns;
  nsString method = u"getRecordMetadata"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    RecordMetadata out;
    if (NS_WARN_IF(!RecordMetadata::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<LoginUpdateMetadataResult> AgentProxy::Login_UpdateMetadata(RequestContextWithOperation& aContext, UpdateMetadataArgs& aArgs) {
  RefPtr<LoginUpdateMetadataResult::Private> outPromise = new LoginUpdateMetadataResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"login"_ns;
  nsString method = u"updateMetadata"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!UpdateMetadataArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountCreationApproveTransitionToAuthOpResult> AgentProxy::AccountCreation_ApproveTransitionToAuthOp(RequestContextWithOperation& aContext, ApproveTransitionToAuthOpArgs& aArgs) {
  RefPtr<AccountCreationApproveTransitionToAuthOpResult::Private> outPromise = new AccountCreationApproveTransitionToAuthOpResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountCreation"_ns;
  nsString method = u"approveTransitionToAuthOp"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!ApproveTransitionToAuthOpArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountCreationGetUserAttributesResult> AgentProxy::AccountCreation_GetUserAttributes(RequestContextWithOperation& aContext) {
  RefPtr<AccountCreationGetUserAttributesResult::Private> outPromise = new AccountCreationGetUserAttributesResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountCreation"_ns;
  nsString method = u"getUserAttributes"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    nsTArray<UserAttribute> out;
    if (NS_WARN_IF(!nsTArray_UserAttribute_FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountCreationAddFieldResult> AgentProxy::AccountCreation_AddField(RequestContextWithOperation& aContext, AddFieldArgs& aArgs) {
  RefPtr<AccountCreationAddFieldResult::Private> outPromise = new AccountCreationAddFieldResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountCreation"_ns;
  nsString method = u"addField"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!AddFieldArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountCreationRejectFieldValueResult> AgentProxy::AccountCreation_RejectFieldValue(RequestContextWithOperation& aContext, RejectFieldValueArgs& aArgs) {
  RefPtr<AccountCreationRejectFieldValueResult::Private> outPromise = new AccountCreationRejectFieldValueResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountCreation"_ns;
  nsString method = u"rejectFieldValue"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!RejectFieldValueArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Variant<nsString, ArrayBuffer> *out;
    if (NS_WARN_IF(!Variant_nsString__ArrayBuffer_FromJSVal(aCx, aValue, &out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountAuthenticationApproveChallengeRequestResult> AgentProxy::AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs) {
  RefPtr<AccountAuthenticationApproveChallengeRequestResult::Private> outPromise = new AccountAuthenticationApproveChallengeRequestResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountAuthentication"_ns;
  nsString method = u"approveChallengeRequest"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!ApproveChallengeRequestArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountAuthenticationAbortChallengeResult> AgentProxy::AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs) {
  RefPtr<AccountAuthenticationAbortChallengeResult::Private> outPromise = new AccountAuthenticationAbortChallengeResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountAuthentication"_ns;
  nsString method = u"abortChallenge"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!AbortChallengeArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountAuthenticationCloseChallengeResult> AgentProxy::AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs) {
  RefPtr<AccountAuthenticationCloseChallengeResult::Private> outPromise = new AccountAuthenticationCloseChallengeResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountAuthentication"_ns;
  nsString method = u"closeChallenge"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!CloseChallengeArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}
RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AgentProxy::AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, RespondToChallengeMessageArgs& aArgs) {
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult::Private> outPromise = new AccountAuthenticationRespondToChallengeMessageResult::Private(__func__);
  ErrorResult rv;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  mozilla::dom::AutoJSAPI jsapi;
  if (NS_WARN_IF(!jsapi.Init(mGlobal))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JSContext* cx = jsapi.cx();

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(cx, "BerytusAgentTarget"_ns, rv);
  if (NS_WARN_IF(rv.Failed())) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSObject*> rMsgData(cx, JS_NewPlainObject(cx));
  if (NS_WARN_IF(!rMsgData)) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> rManagerId(
    cx, JS_NewUCStringCopyN(cx, mManagerId.get(), mManagerId.Length()));
  JS::Rooted<JS::Value> vManagerId(cx, StringValue(rManagerId));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "managerId", vManagerId))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  nsString group = u"accountAuthentication"_ns;
  nsString method = u"respondToChallengeMessage"_ns;

  JS::Rooted<JSString*> rGroup(
    cx, JS_NewUCStringCopyN(cx, group.get(), group.Length()));
  JS::Rooted<JS::Value> vGroup(cx, StringValue(rGroup));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "group", vGroup))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JSString*> jMethod(
    cx, JS_NewUCStringCopyN(cx, method.get(), method.Length()));
  JS::Rooted<JS::Value> vMethod(cx, StringValue(jMethod));
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "method", vMethod))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vReqCx(cx);
  if (NS_WARN_IF(!RequestContextWithOperation::ToJSVal(cx, aContext, &vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestContext", vReqCx))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  JS::Rooted<JS::Value> vReqArgs(cx);
  if (NS_WARN_IF(!RespondToChallengeMessageArgs::ToJSVal(cx, aArgs, &vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  if (NS_WARN_IF(!JS_SetProperty(cx, rMsgData, "requestArgs", vReqArgs))) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }

  JS::Rooted<JS::Value> vMsgData(cx, JS::ObjectValue(*rMsgData));

  RefPtr<mozilla::dom::Promise> prom = actor->SendQuery(cx, u"BerytusAgentTarget:invokeRequestHandler"_ns, vMsgData, rv);
  if (rv.Failed()) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ChallengeMessageResponse out;
    if (NS_WARN_IF(!ChallengeMessageResponse::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(NS_ERROR_FAILURE, __func__);
      return;
    }
    
    outPromise->Resolve(std::move(out), __func__);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    outPromise->Reject(NS_ERROR_FAILURE, __func__);
  };
  prom->AddCallbacksWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  return outPromise;
}

}  // namespace mozilla::berytus