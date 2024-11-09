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
    : mGlobal(aGlobal), mManagerId(aManagerId), mDisabled(false) {}

AgentProxy::~AgentProxy() {}

void AgentProxy::Disable() {
  mDisabled = true;
}
bool AgentProxy::IsDisabled() const {
  return mDisabled;
}

template <typename W1, typename W2, typename, typename>
already_AddRefed<dom::Promise> AgentProxy::CallSendQuery(JSContext *aCx,
                                                         const nsAString & aGroup,
                                                         const nsAString &aMethod,
                                                         const W1& aReqCx,
                                                         const W2* aReqArgs,
                                                         ErrorResult& aRv) const {
  MOZ_ASSERT(!aRv.Failed());
  if (mDisabled) {
    aRv.ThrowInvalidStateError("Agent is disabled");
    return nullptr;
  }
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  RefPtr<mozilla::dom::JSWindowActorChild> actor =
    wgc->GetActor(aCx, "BerytusAgentTarget"_ns, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }

  // MOZ_ASSERT(actor->GetWrapper());
  JS::Rooted<JSObject*> actorJsImpl(aCx, actor->GetWrapper());
  JSAutoRealm ar(aCx, actorJsImpl);

  JS::Rooted<JS::Value> sendQuery(aCx);
  if (NS_WARN_IF(!JS_GetProperty(aCx, actorJsImpl, "sendQuery", &sendQuery))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!sendQuery.isObject())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!JS::IsCallable(&sendQuery.toObject()))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> msgName(aCx, JS::StringValue(JS_NewUCStringCopyZ(aCx, u"BerytusAgentTarget:invokeRequestHandler")));
  JS::Rooted<JS::Value> promiseVal(aCx);
  JS::RootedVector<JS::Value> args(aCx);
  if (NS_WARN_IF(!args.append(msgName))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JSObject*> msgData(aCx, JS_NewPlainObject(aCx));
  if (NS_WARN_IF(!msgData)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  JS::Rooted<JS::Value> managerId(aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, mManagerId.get(), mManagerId.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "managerId", managerId))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  const char16_t* groupBuf;
  aGroup.GetData(&groupBuf);
  JS::Rooted<JS::Value> group(
    aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, groupBuf, aGroup.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "group", group))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  const char16_t* methodBuf;
  aMethod.GetData(&methodBuf);
  JS::Rooted<JS::Value> method(
    aCx, JS::StringValue(JS_NewUCStringCopyN(aCx, methodBuf, aMethod.Length())));
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "method", method))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }

  JS::Rooted<JS::Value> reqCxJS(aCx);
  if (NS_WARN_IF(!aReqCx.ToJSVal(aCx, aReqCx, &reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestContext", reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (aReqArgs) {
    JS::Rooted<JS::Value> reqArgsJS(aCx);
    if (NS_WARN_IF(!aReqArgs->ToJSVal(aCx, *aReqArgs, &reqArgsJS))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
    if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestArgs", reqArgsJS))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
  }
  if (NS_WARN_IF(!args.append(JS::ObjectValue(*msgData)))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!JS_CallFunctionValue(aCx, actorJsImpl, sendQuery, JS::HandleValueArray(args), //JS::HandleValueArray::empty(), //JS::HandleValueArray(aData),
                &promiseVal)) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!promiseVal.isObject())) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  RefPtr<dom::Promise> promise = dom::Promise::Create(mGlobal, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  promise->MaybeResolve(promiseVal);
  return promise.forget();
}

bool JSValIsNumber(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isNumber() || aValue.isNumber();
  return true;
}
bool NumberFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv) {
  if (aValue.isInt32()) {
    aRv = static_cast<double>(aValue.toInt32());
    return true;
  }
  if (NS_WARN_IF(!aValue.isNumber())) {
    return false;
  }
  aRv = aValue.toNumber();
  return true;
}
bool NumberToJSVal(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setNumber(aValue);
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
  if (NS_WARN_IF(!JSValIsNumber(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!NumberFromJSVal(aCx, propVal, aRv.mId))) {
    return false;
  }
  
  return true;
}
            
bool DocumentMetadata::ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!NumberToJSVal(aCx, aValue.mId, &memberVal0))) {
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
  JS::AutoCheckCannotGC nogc;
  JSString* str = aValue.toString();
  size_t len;
  if (!JS::StringHasLatin1Chars(str)) {
    const char16_t* buf =
      JS_GetTwoByteStringCharsAndLength(aCx, nogc, str, &len);
    len = JS::GetStringLength(str);
    aRv.Assign(buf, len);
    return true;
  }
  // Latin1/OneByte JSString.
  const JS::Latin1Char* buf =
    JS_GetLatin1StringCharsAndLength(aCx, nogc, str, &len);
  CopyASCIItoUTF16(Span(reinterpret_cast<const char*>(buf), len), aRv);
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
  if (NS_WARN_IF(!JSValIsNumber(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!NumberFromJSVal(aCx, propVal, aRv.mPort))) {
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
  
  if (NS_WARN_IF(!NumberToJSVal(aCx, aValue.mPort, &memberVal3))) {
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

ErrorResult Failure::ToErrorResult() const {
  ErrorResult rv;
  nsresult result = mException->GetResult();
    const nsCString& msg = mException->GetMessageMoz();

  if (result == NS_ERROR_ABORT) {
    rv.ThrowAbortError(
      msg.Length() == 0
        ? "Request aborted"_ns
        : msg
    );
  } else {
    rv.ThrowInvalidStateError(
      msg.Length() == 0
        ? "Request failed"_ns
        : msg
    );
  }
  return rv;
}
bool Failure::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, Failure& aRv) {
  nsresult result = NS_ERROR_FAILURE;
  nsIStackFrame* stackFrame = nullptr;
  nsISupports* data = nullptr;
  nsCString msg = BERYTUS_AGENT_DEFAULT_EXCEPTION_MESSAGE;
  nsCString excpName = BERYTUS_AGENT_DEFAULT_EXCEPTION_NAME;
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(js::IsCrossCompartmentWrapper(&aValue.toObject()))) {
    return false;
  }
  JS::RootedObject errorObj(aCx, &aValue.toObject());
  JS::RootedValue messageVal(aCx);
  JS::RootedValue resultVal(aCx);
  if (JS_GetProperty(aCx, errorObj, "result", &resultVal)) {
    if (resultVal.isInt32()) {
      result = nsresult(resultVal.toInt32());
    } else if (resultVal.isDouble()) {
      result = nsresult(resultVal.toDouble());
    }
  }
  if (JS_GetProperty(aCx, errorObj, "message", &messageVal)) {
    nsString twoByteMsg;
    if (NS_WARN_IF(!StringFromJSVal(aCx, messageVal, twoByteMsg))) {
      return false;
    }
    // additional copy... TODO(berytus): possibly optimise this.
    msg = NS_ConvertUTF16toUTF8(twoByteMsg);
  }
  aRv.mException =
      new mozilla::dom::Exception(msg,
                                  result,
                                  excpName,
                                  stackFrame,
                                  data);
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
bool JSValIsMaybe_double_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIsNumber(aCx, aValue, aRv);
}
bool Maybe_double_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<double>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  
  aRv.emplace();
  if (NS_WARN_IF(!NumberFromJSVal(aCx, aValue, *aRv))) {
    return false;
  }
  return true;
}
bool Maybe_double_ToJSVal(JSContext* aCx, const Maybe<double>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return NumberToJSVal(aCx, aValue.ref(), aRv);
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
  if (NS_WARN_IF(!JSValIsMaybe_double_(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!Maybe_double_FromJSVal(aCx, propVal, aRv.mSchemaVersion))) {
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
  
  if (NS_WARN_IF(!Maybe_double_ToJSVal(aCx, aValue.mSchemaVersion, &memberVal1))) {
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
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelConstraints::IsJSValueValid(aCx, propVal, isValid))) {
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
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!ChannelConstraints::FromJSVal(aCx, propVal, aRv.mChannelConstraints))) {
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
  
  if (NS_WARN_IF(!ChannelConstraints::ToJSVal(aCx, aValue.mChannelConstraints, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelConstraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!AccountConstraints::ToJSVal(aCx, aValue.mAccountConstraints, &memberVal2))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "accountConstraints", memberVal2))) {
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
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
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
  if (NS_WARN_IF(!JSValIsNumber(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!NumberFromJSVal(aCx, propVal, aRv.mAesKeyLength))) {
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
  
  if (NS_WARN_IF(!NumberToJSVal(aCx, aValue.mAesKeyLength, &memberVal4))) {
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
  if (NS_WARN_IF(!JSValIsNumber(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!NumberFromJSVal(aCx, propVal, aRv.mAesKeyLength))) {
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
  
  if (NS_WARN_IF(!NumberToJSVal(aCx, aValue.mAesKeyLength, &memberVal7))) {
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



void ELoginUserIntent::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"PendingDeclaration"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"Authenticate"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Register"_ns);
    return;
  }
}
ELoginUserIntent ELoginUserIntent::PendingDeclaration() {
  return ELoginUserIntent(uint8_t(0));
}
ELoginUserIntent ELoginUserIntent::Authenticate() {
  return ELoginUserIntent(uint8_t(1));
}
ELoginUserIntent ELoginUserIntent::Register() {
  return ELoginUserIntent(uint8_t(2));
}
bool ELoginUserIntent::FromString(const nsString& aVal, ELoginUserIntent& aRetVal) {
  if (aVal.Equals(u"PendingDeclaration"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"Authenticate"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Register"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
  return false;
}
bool ELoginUserIntent::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  ELoginUserIntent e;
  aRv = ELoginUserIntent::FromString(strVal, e);
  return true;
}
bool ELoginUserIntent::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!ELoginUserIntent::ELoginUserIntent::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool ELoginUserIntent::ToJSVal(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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
void EOperationType::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"PendingDeclaration"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"Registration"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Authentication"_ns);
    return;
  }
}
EOperationType EOperationType::PendingDeclaration() {
  return EOperationType(uint8_t(0));
}
EOperationType EOperationType::Registration() {
  return EOperationType(uint8_t(1));
}
EOperationType EOperationType::Authentication() {
  return EOperationType(uint8_t(2));
}
bool EOperationType::FromString(const nsString& aVal, EOperationType& aRetVal) {
  if (aVal.Equals(u"PendingDeclaration"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"Registration"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Authentication"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
  return false;
}
bool EOperationType::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EOperationType e;
  aRv = EOperationType::FromString(strVal, e);
  return true;
}
bool EOperationType::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationType& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationType::EOperationType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EOperationType::ToJSVal(JSContext* aCx, const EOperationType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
void EOperationStatus::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"Pending"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"Created"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Aborted"_ns);
    return;
  }
  if (mVal == 3) {
    aRetVal.Assign(u"Finished"_ns);
    return;
  }
}
EOperationStatus EOperationStatus::Pending() {
  return EOperationStatus(uint8_t(0));
}
EOperationStatus EOperationStatus::Created() {
  return EOperationStatus(uint8_t(1));
}
EOperationStatus EOperationStatus::Aborted() {
  return EOperationStatus(uint8_t(2));
}
EOperationStatus EOperationStatus::Finished() {
  return EOperationStatus(uint8_t(3));
}
bool EOperationStatus::FromString(const nsString& aVal, EOperationStatus& aRetVal) {
  if (aVal.Equals(u"Pending"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"Created"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Aborted"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
if (aVal.Equals(u"Finished"_ns)) {
    aRetVal.mVal = 3;
    return true;
  }
  return false;
}
bool EOperationStatus::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EOperationStatus e;
  aRv = EOperationStatus::FromString(strVal, e);
  return true;
}
bool EOperationStatus::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::EOperationStatus::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EOperationStatus::ToJSVal(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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
  if (NS_WARN_IF(!ELoginUserIntent::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!EOperationType::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!ELoginUserIntent::FromJSVal(aCx, propVal, aRv.mIntent))) {
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
  if (NS_WARN_IF(!EOperationType::FromJSVal(aCx, propVal, aRv.mType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::FromJSVal(aCx, propVal, aRv.mStatus))) {
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
  
  if (NS_WARN_IF(!ELoginUserIntent::ToJSVal(aCx, aValue.mIntent, &memberVal0))) {
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
  
  if (NS_WARN_IF(!EOperationType::ToJSVal(aCx, aValue.mType, &memberVal3))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!EOperationStatus::ToJSVal(aCx, aValue.mStatus, &memberVal4))) {
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
  if (NS_WARN_IF(!EOperationType::IsJSValueValid(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!EOperationType::FromJSVal(aCx, propVal, aRv.mType))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::FromJSVal(aCx, propVal, aRv.mStatus))) {
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
  
  if (NS_WARN_IF(!EOperationType::ToJSVal(aCx, aValue.mType, &memberVal1))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!EOperationStatus::ToJSVal(aCx, aValue.mStatus, &memberVal2))) {
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


void EMetadataStatus::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"Pending"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"Created"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Retired"_ns);
    return;
  }
}
EMetadataStatus EMetadataStatus::Pending() {
  return EMetadataStatus(uint8_t(0));
}
EMetadataStatus EMetadataStatus::Created() {
  return EMetadataStatus(uint8_t(1));
}
EMetadataStatus EMetadataStatus::Retired() {
  return EMetadataStatus(uint8_t(2));
}
bool EMetadataStatus::FromString(const nsString& aVal, EMetadataStatus& aRetVal) {
  if (aVal.Equals(u"Pending"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"Created"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Retired"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
  return false;
}
bool EMetadataStatus::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EMetadataStatus e;
  aRv = EMetadataStatus::FromString(strVal, e);
  return true;
}
bool EMetadataStatus::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EMetadataStatus::EMetadataStatus::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EMetadataStatus::ToJSVal(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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
  if (NS_WARN_IF(!JSValIsNumber(aCx, propVal, isValid))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EMetadataStatus::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!NumberFromJSVal(aCx, propVal, aRv.mVersion))) {
    return false;
  }
  

  if(NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!EMetadataStatus::FromJSVal(aCx, propVal, aRv.mStatus))) {
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
  
  if (NS_WARN_IF(!NumberToJSVal(aCx, aValue.mVersion, &memberVal0))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "version", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!EMetadataStatus::ToJSVal(aCx, aValue.mStatus, &memberVal1))) {
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

void EFieldType::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"Identity"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"ForeignIdentity"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Password"_ns);
    return;
  }
  if (mVal == 3) {
    aRetVal.Assign(u"SecurePassword"_ns);
    return;
  }
  if (mVal == 4) {
    aRetVal.Assign(u"Key"_ns);
    return;
  }
}
EFieldType EFieldType::Identity() {
  return EFieldType(uint8_t(0));
}
EFieldType EFieldType::ForeignIdentity() {
  return EFieldType(uint8_t(1));
}
EFieldType EFieldType::Password() {
  return EFieldType(uint8_t(2));
}
EFieldType EFieldType::SecurePassword() {
  return EFieldType(uint8_t(3));
}
EFieldType EFieldType::Key() {
  return EFieldType(uint8_t(4));
}
bool EFieldType::FromString(const nsString& aVal, EFieldType& aRetVal) {
  if (aVal.Equals(u"Identity"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"ForeignIdentity"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Password"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
if (aVal.Equals(u"SecurePassword"_ns)) {
    aRetVal.mVal = 3;
    return true;
  }
if (aVal.Equals(u"Key"_ns)) {
    aRetVal.mVal = 4;
    return true;
  }
  return false;
}
bool EFieldType::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EFieldType e;
  aRv = EFieldType::FromString(strVal, e);
  return true;
}
bool EFieldType::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EFieldType& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EFieldType::EFieldType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EFieldType::ToJSVal(JSContext* aCx, const EFieldType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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
  if (NS_WARN_IF(!EFieldType::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!EFieldType::FromJSVal(aCx, propVal, aRv.mFieldType))) {
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
  
  if (NS_WARN_IF(!EFieldType::ToJSVal(aCx, aValue.mFieldType, &memberVal0))) {
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

void EChallengeType::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"Identification"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"DigitalSignature"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Password"_ns);
    return;
  }
  if (mVal == 3) {
    aRetVal.Assign(u"SecureRemotePassword"_ns);
    return;
  }
  if (mVal == 4) {
    aRetVal.Assign(u"ForeignIdentityOtp"_ns);
    return;
  }
}
EChallengeType EChallengeType::Identification() {
  return EChallengeType(uint8_t(0));
}
EChallengeType EChallengeType::DigitalSignature() {
  return EChallengeType(uint8_t(1));
}
EChallengeType EChallengeType::Password() {
  return EChallengeType(uint8_t(2));
}
EChallengeType EChallengeType::SecureRemotePassword() {
  return EChallengeType(uint8_t(3));
}
EChallengeType EChallengeType::ForeignIdentityOtp() {
  return EChallengeType(uint8_t(4));
}
bool EChallengeType::FromString(const nsString& aVal, EChallengeType& aRetVal) {
  if (aVal.Equals(u"Identification"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"DigitalSignature"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Password"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
if (aVal.Equals(u"SecureRemotePassword"_ns)) {
    aRetVal.mVal = 3;
    return true;
  }
if (aVal.Equals(u"ForeignIdentityOtp"_ns)) {
    aRetVal.mVal = 4;
    return true;
  }
  return false;
}
bool EChallengeType::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EChallengeType e;
  aRv = EChallengeType::FromString(strVal, e);
  return true;
}
bool EChallengeType::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeType& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EChallengeType::EChallengeType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EChallengeType::ToJSVal(JSContext* aCx, const EChallengeType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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

void EChallengeStatus::ToString(nsString& aRetVal) const {
  if (mVal == 0) {
    aRetVal.Assign(u"Invalid"_ns);
    return;
  }
  if (mVal == 1) {
    aRetVal.Assign(u"Pending"_ns);
    return;
  }
  if (mVal == 2) {
    aRetVal.Assign(u"Active"_ns);
    return;
  }
  if (mVal == 3) {
    aRetVal.Assign(u"Aborted"_ns);
    return;
  }
  if (mVal == 4) {
    aRetVal.Assign(u"Sealed"_ns);
    return;
  }
}
EChallengeStatus EChallengeStatus::Invalid() {
  return EChallengeStatus(uint8_t(0));
}
EChallengeStatus EChallengeStatus::Pending() {
  return EChallengeStatus(uint8_t(1));
}
EChallengeStatus EChallengeStatus::Active() {
  return EChallengeStatus(uint8_t(2));
}
EChallengeStatus EChallengeStatus::Aborted() {
  return EChallengeStatus(uint8_t(3));
}
EChallengeStatus EChallengeStatus::Sealed() {
  return EChallengeStatus(uint8_t(4));
}
bool EChallengeStatus::FromString(const nsString& aVal, EChallengeStatus& aRetVal) {
  if (aVal.Equals(u"Invalid"_ns)) {
    aRetVal.mVal = 0;
    return true;
  }
if (aVal.Equals(u"Pending"_ns)) {
    aRetVal.mVal = 1;
    return true;
  }
if (aVal.Equals(u"Active"_ns)) {
    aRetVal.mVal = 2;
    return true;
  }
if (aVal.Equals(u"Aborted"_ns)) {
    aRetVal.mVal = 3;
    return true;
  }
if (aVal.Equals(u"Sealed"_ns)) {
    aRetVal.mVal = 4;
    return true;
  }
  return false;
}
bool EChallengeStatus::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EChallengeStatus e;
  aRv = EChallengeStatus::FromString(strVal, e);
  return true;
}
bool EChallengeStatus::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EChallengeStatus& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EChallengeStatus::EChallengeStatus::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EChallengeStatus::ToJSVal(JSContext* aCx, const EChallengeStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
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
  if (NS_WARN_IF(!EChallengeType::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!EChallengeStatus::IsJSValueValid(aCx, propVal, isValid))) {
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
  if (NS_WARN_IF(!EChallengeType::FromJSVal(aCx, propVal, aRv.mType))) {
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
  if (NS_WARN_IF(!EChallengeStatus::FromJSVal(aCx, propVal, aRv.mStatus))) {
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
  
  if (NS_WARN_IF(!EChallengeType::ToJSVal(aCx, aValue.mType, &memberVal1))) {
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
  
  if (NS_WARN_IF(!EChallengeStatus::ToJSVal(aCx, aValue.mStatus, &memberVal3))) {
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



RefPtr<ManagerGetSigningKeyResult> AgentProxy::Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs) const {
  RefPtr<ManagerGetSigningKeyResult::Private> outPromise = new ManagerGetSigningKeyResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"manager"_ns,
                                            u"getSigningKey"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    nsString out;
    if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<ManagerGetCredentialsMetadataResult> AgentProxy::Manager_GetCredentialsMetadata(PreliminaryRequestContext& aContext, GetCredentialsMetadataArgs& aArgs) const {
  RefPtr<ManagerGetCredentialsMetadataResult::Private> outPromise = new ManagerGetCredentialsMetadataResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"manager"_ns,
                                            u"getCredentialsMetadata"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    double out;
    if (NS_WARN_IF(!NumberFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<ChannelGenerateKeyExchangeParametersResult> AgentProxy::Channel_GenerateKeyExchangeParameters(RequestContext& aContext, GenerateKeyExchangeParametersArgs& aArgs) const {
  RefPtr<ChannelGenerateKeyExchangeParametersResult::Private> outPromise = new ChannelGenerateKeyExchangeParametersResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"generateKeyExchangeParameters"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    PartialKeyExchangeParametersFromScm out;
    if (NS_WARN_IF(!PartialKeyExchangeParametersFromScm::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<ChannelEnableEndToEndEncryptionResult> AgentProxy::Channel_EnableEndToEndEncryption(RequestContext& aContext, EnableEndToEndEncryptionArgs& aArgs) const {
  RefPtr<ChannelEnableEndToEndEncryptionResult::Private> outPromise = new ChannelEnableEndToEndEncryptionResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"enableEndToEndEncryption"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ArrayBuffer out;
    if (NS_WARN_IF(!ArrayBufferFromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<ChannelCloseChannelResult> AgentProxy::Channel_CloseChannel(RequestContext& aContext) const {
  RefPtr<ChannelCloseChannelResult::Private> outPromise = new ChannelCloseChannelResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"closeChannel"_ns,
                                            aContext,
                                            static_cast<PreliminaryRequestContext*>(nullptr),
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<LoginApproveOperationResult> AgentProxy::Login_ApproveOperation(RequestContext& aContext, ApproveOperationArgs& aArgs) const {
  RefPtr<LoginApproveOperationResult::Private> outPromise = new LoginApproveOperationResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"login"_ns,
                                            u"approveOperation"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ELoginUserIntent out;
    if (NS_WARN_IF(!ELoginUserIntent::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<LoginCloseOpeationResult> AgentProxy::Login_CloseOpeation(RequestContextWithOperation& aContext) const {
  RefPtr<LoginCloseOpeationResult::Private> outPromise = new LoginCloseOpeationResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"login"_ns,
                                            u"closeOpeation"_ns,
                                            aContext,
                                            static_cast<PreliminaryRequestContext*>(nullptr),
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<LoginGetRecordMetadataResult> AgentProxy::Login_GetRecordMetadata(RequestContextWithOperation& aContext) const {
  RefPtr<LoginGetRecordMetadataResult::Private> outPromise = new LoginGetRecordMetadataResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"login"_ns,
                                            u"getRecordMetadata"_ns,
                                            aContext,
                                            static_cast<PreliminaryRequestContext*>(nullptr),
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    RecordMetadata out;
    if (NS_WARN_IF(!RecordMetadata::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<LoginUpdateMetadataResult> AgentProxy::Login_UpdateMetadata(RequestContextWithOperation& aContext, UpdateMetadataArgs& aArgs) const {
  RefPtr<LoginUpdateMetadataResult::Private> outPromise = new LoginUpdateMetadataResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"login"_ns,
                                            u"updateMetadata"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountCreationApproveTransitionToAuthOpResult> AgentProxy::AccountCreation_ApproveTransitionToAuthOp(RequestContextWithOperation& aContext, ApproveTransitionToAuthOpArgs& aArgs) const {
  RefPtr<AccountCreationApproveTransitionToAuthOpResult::Private> outPromise = new AccountCreationApproveTransitionToAuthOpResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountCreation"_ns,
                                            u"approveTransitionToAuthOp"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountCreationGetUserAttributesResult> AgentProxy::AccountCreation_GetUserAttributes(RequestContextWithOperation& aContext) const {
  RefPtr<AccountCreationGetUserAttributesResult::Private> outPromise = new AccountCreationGetUserAttributesResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountCreation"_ns,
                                            u"getUserAttributes"_ns,
                                            aContext,
                                            static_cast<PreliminaryRequestContext*>(nullptr),
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    nsTArray<UserAttribute> out;
    if (NS_WARN_IF(!nsTArray_UserAttribute_FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountCreationAddFieldResult> AgentProxy::AccountCreation_AddField(RequestContextWithOperation& aContext, AddFieldArgs& aArgs) const {
  RefPtr<AccountCreationAddFieldResult::Private> outPromise = new AccountCreationAddFieldResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountCreation"_ns,
                                            u"addField"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountCreationRejectFieldValueResult> AgentProxy::AccountCreation_RejectFieldValue(RequestContextWithOperation& aContext, RejectFieldValueArgs& aArgs) const {
  RefPtr<AccountCreationRejectFieldValueResult::Private> outPromise = new AccountCreationRejectFieldValueResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountCreation"_ns,
                                            u"rejectFieldValue"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Variant<nsString, ArrayBuffer> *out;
    if (NS_WARN_IF(!Variant_nsString__ArrayBuffer_FromJSVal(aCx, aValue, &out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountAuthenticationApproveChallengeRequestResult> AgentProxy::AccountAuthentication_ApproveChallengeRequest(RequestContextWithOperation& aContext, ApproveChallengeRequestArgs& aArgs) const {
  RefPtr<AccountAuthenticationApproveChallengeRequestResult::Private> outPromise = new AccountAuthenticationApproveChallengeRequestResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountAuthentication"_ns,
                                            u"approveChallengeRequest"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountAuthenticationAbortChallengeResult> AgentProxy::AccountAuthentication_AbortChallenge(RequestContextWithOperation& aContext, AbortChallengeArgs& aArgs) const {
  RefPtr<AccountAuthenticationAbortChallengeResult::Private> outPromise = new AccountAuthenticationAbortChallengeResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountAuthentication"_ns,
                                            u"abortChallenge"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountAuthenticationCloseChallengeResult> AgentProxy::AccountAuthentication_CloseChallenge(RequestContextWithOperation& aContext, CloseChallengeArgs& aArgs) const {
  RefPtr<AccountAuthenticationCloseChallengeResult::Private> outPromise = new AccountAuthenticationCloseChallengeResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountAuthentication"_ns,
                                            u"closeChallenge"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}
RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AgentProxy::AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, RespondToChallengeMessageArgs& aArgs) const {
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult::Private> outPromise = new AccountAuthenticationRespondToChallengeMessageResult::Private(__func__);
  nsresult res;
  nsPIDOMWindowInner* inner = mGlobal->GetAsInnerWindow();
  if (NS_WARN_IF(!inner)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  mozilla::dom::WindowGlobalChild* wgc = inner->GetWindowGlobalChild();
  if (NS_WARN_IF(!wgc)) {
    outPromise->Reject(Failure(), __func__);
    return outPromise;
  }

  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountAuthentication"_ns,
                                            u"respondToChallengeMessage"_ns,
                                            aContext,
                                            &aArgs,
                                            err);
  if (NS_WARN_IF(err.Failed())) {
    outPromise->Reject(Failure(err.StealNSResult()), __func__);
    return outPromise;
  }
  auto onResolve = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                      ErrorResult& aRv,
                      const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    ChallengeMessageResponse out;
    if (NS_WARN_IF(!ChallengeMessageResponse::FromJSVal(aCx, aValue, out))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    Failure fr;
    Failure::FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  }
  return outPromise;
}

}  // namespace mozilla::berytus