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
#include "mozilla/dom/PromiseNativeHandler.h"

static mozilla::LazyLogModule sLogger("berytus_agent");

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

bool AgentProxy::IsDisabled() const {
  return mDisabled;
}

template <typename W1, typename W2>
already_AddRefed<dom::Promise> AgentProxy::CallSendQuery(JSContext *aCx,
                                                         const nsAString & aGroup,
                                                         const nsAString &aMethod,
                                                         const W1& aReqCx,
                                                         const W2* aReqArgs,
                                                         ErrorResult& aRv) {
  JS::Rooted<JS::Value> reqArgsJS(aCx, JS::UndefinedValue());
  if (aReqArgs) {
    if (NS_WARN_IF(!ToJSVal(aCx, *aReqArgs, &reqArgsJS))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
  }
  return CallSendQuery(aCx, aGroup, aMethod, aReqCx, reqArgsJS, aRv);
}

template <typename W1>
already_AddRefed<dom::Promise> AgentProxy::CallSendQuery(JSContext *aCx,
                                                         const nsAString & aGroup,
                                                         const nsAString &aMethod,
                                                         const W1& aReqCx,
                                                         JS::Handle<JS::Value> aReqArgsJs,
                                                         ErrorResult& aRv) {
  MOZ_LOG(sLogger, LogLevel::Info, ("SendQuery %s:%s", NS_ConvertUTF16toUTF8(aGroup).get(), NS_ConvertUTF16toUTF8(aMethod).get()));
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
  if (NS_WARN_IF(!ToJSVal(aCx, aReqCx, &reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestContext", reqCxJS))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  if (!aReqArgsJs.isUndefined()) {
    JS::Rooted<JS::Value> wrappedReqArgs(aCx, aReqArgsJs);
    if (NS_WARN_IF(!JS_WrapValue(aCx, &wrappedReqArgs))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
    if (NS_WARN_IF(!JS_SetProperty(aCx, msgData, "requestArgs", wrappedReqArgs))) {
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

NS_IMPL_ADDREF_INHERITED(OwnedAgentProxy, AgentProxy)
NS_IMPL_RELEASE_INHERITED(OwnedAgentProxy, AgentProxy)
NS_INTERFACE_MAP_BEGIN(OwnedAgentProxy)
NS_INTERFACE_MAP_END_INHERITING(AgentProxy)

OwnedAgentProxy::OwnedAgentProxy(
    nsIGlobalObject* aGlobal, const nsAString& aManagerId)
    : AgentProxy(aGlobal, aManagerId) {}

OwnedAgentProxy::~OwnedAgentProxy() {}

void OwnedAgentProxy::Disable() {
  mDisabled = true;
}

template<>
bool JSValIs<double>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isNumber() || aValue.isNumber();
  return true;
}
template<>
bool FromJSVal<double>(JSContext* aCx, JS::Handle<JS::Value> aValue, double& aRv) {
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
template<>
bool ToJSVal<double>(JSContext* aCx, const double& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setNumber(aValue);
  return true;
}
template<>
bool JSValIs<nsString>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isString();
  return true;
}
template<>
bool FromJSVal<nsString>(JSContext* aCx, JS::Handle<JS::Value> aValue, nsString& aRv) {
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
template<>
bool ToJSVal<nsString>(JSContext* aCx, const nsString& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.get(), aValue.Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<UriParams>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<UriParams>(JSContext* aCx, JS::Handle<JS::Value> aValue, UriParams& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mUri)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mScheme)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mHostname)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mPort)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mPath)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<UriParams>(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mUri, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "uri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mScheme, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scheme", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mHostname, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hostname", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mPort, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "port", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mPath, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "path", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<DocumentMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<UriParams>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<DocumentMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, DocumentMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<UriParams>(aCx, propVal, aRv.mUri)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<DocumentMetadata>(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<UriParams>(aCx, aValue.mUri, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "uri", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<PreliminaryRequestContext>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<DocumentMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<PreliminaryRequestContext>(JSContext* aCx, JS::Handle<JS::Value> aValue, PreliminaryRequestContext& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<DocumentMetadata>(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<PreliminaryRequestContext>(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<DocumentMetadata>(aCx, aValue.mDocument, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<CryptoActor>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<CryptoActor>(JSContext* aCx, JS::Handle<JS::Value> aValue, CryptoActor& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mEd25519Key)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<CryptoActor>(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mEd25519Key, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "ed25519Key", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<OriginActor>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<UriParams>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<UriParams>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<OriginActor>(JSContext* aCx, JS::Handle<JS::Value> aValue, OriginActor& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<UriParams>(aCx, propVal, aRv.mOriginalUri)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<UriParams>(aCx, propVal, aRv.mCurrentUri)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<OriginActor>(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<UriParams>(aCx, aValue.mOriginalUri, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "originalUri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<UriParams>(aCx, aValue.mCurrentUri, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "currentUri", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<CryptoActor, OriginActor>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<CryptoActor>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<OriginActor>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<CryptoActor, OriginActor>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<CryptoActor, OriginActor>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<CryptoActor>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      CryptoActor nv;
      if (NS_WARN_IF(!(FromJSVal<CryptoActor>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<OriginActor>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      OriginActor nv;
      if (NS_WARN_IF(!(FromJSVal<OriginActor>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<CryptoActor, OriginActor>>(JSContext* aCx, const SafeVariant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const CryptoActor& aVal) {
      return ToJSVal<CryptoActor>(mCx, aVal, mRv);
    }

    bool operator()(const OriginActor& aVal) {
      return ToJSVal<OriginActor>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<GetSigningKeyArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<GetSigningKeyArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, GetSigningKeyArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<GetSigningKeyArgs>(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, aValue.mWebAppActor, &memberVal0)))) {
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
template<>
bool FromJSVal<Failure>(JSContext* aCx, JS::Handle<JS::Value> aValue, Failure& aRv) {
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
    if (NS_WARN_IF(!FromJSVal(aCx, messageVal, twoByteMsg))) {
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

template<>
bool JSValIs<nsTArray<nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  // TODO(berytus): What about the values inside the array?
  return true;
}
template<>
bool FromJSVal<nsTArray<nsString>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<nsString>& aRv) {
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
    if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
template<>
bool ToJSVal<nsTArray<nsString>>(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const nsString& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
template<>
bool JSValIs<Maybe<nsTArray<nsString>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<nsTArray<nsString>>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<nsTArray<nsString>>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<nsString>>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<nsTArray<nsString>>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<nsTArray<nsString>>>(JSContext* aCx, const Maybe<nsTArray<nsString>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<nsTArray<nsString>>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<bool>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isBoolean();
  return true;
}
template<>
bool FromJSVal<bool>(JSContext* aCx, JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isBoolean())) {
    return false;
  }
  aRv = aValue.toBoolean();
  return true;
}
template<>
bool ToJSVal<bool>(JSContext* aCx, const bool& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setBoolean(aValue);
  return true;
}
template<>
bool JSValIs<Maybe<nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<nsString>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<nsString>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsString>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<nsString>>(JSContext* aCx, const Maybe<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<nsString>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<Maybe<double>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<double>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<double>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<double>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<double>>(JSContext* aCx, const Maybe<double>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<double>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<PartialAccountIdentity>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<PartialAccountIdentity>(JSContext* aCx, JS::Handle<JS::Value> aValue, PartialAccountIdentity& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mFieldValue)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<PartialAccountIdentity>(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mFieldValue, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldValue", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<nsTArray<PartialAccountIdentity>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  // TODO(berytus): What about the values inside the array?
  return true;
}
template<>
bool FromJSVal<nsTArray<PartialAccountIdentity>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<PartialAccountIdentity>& aRv) {
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
    if (NS_WARN_IF(!(FromJSVal<PartialAccountIdentity>(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
template<>
bool ToJSVal<nsTArray<PartialAccountIdentity>>(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const PartialAccountIdentity& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(ToJSVal<PartialAccountIdentity>(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
template<>
bool JSValIs<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<nsTArray<PartialAccountIdentity>>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<nsTArray<PartialAccountIdentity>>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<nsTArray<PartialAccountIdentity>>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(JSContext* aCx, const Maybe<nsTArray<PartialAccountIdentity>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<nsTArray<PartialAccountIdentity>>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<AccountConstraints>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<double>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsTArray<PartialAccountIdentity>>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<AccountConstraints>(JSContext* aCx, JS::Handle<JS::Value> aValue, AccountConstraints& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsString>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<double>>(aCx, propVal, aRv.mSchemaVersion)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(aCx, propVal, aRv.mIdentity)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<AccountConstraints>(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsString>>(aCx, aValue.mCategory, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<double>>(aCx, aValue.mSchemaVersion, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "schemaVersion", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsTArray<PartialAccountIdentity>>>(aCx, aValue.mIdentity, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "identity", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<Maybe<AccountConstraints>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<AccountConstraints>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<AccountConstraints>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<AccountConstraints>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<AccountConstraints>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<AccountConstraints>>(JSContext* aCx, const Maybe<AccountConstraints>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<AccountConstraints>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<ChannelConstraints>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsTArray<nsString>>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<AccountConstraints>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<ChannelConstraints>(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelConstraints& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsTArray<nsString>>>(aCx, propVal, aRv.mSecretManagerPublicKey)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mEnableEndToEndEncryption)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<AccountConstraints>>(aCx, propVal, aRv.mAccount)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<ChannelConstraints>(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsTArray<nsString>>>(aCx, aValue.mSecretManagerPublicKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "secretManagerPublicKey", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mEnableEndToEndEncryption, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "enableEndToEndEncryption", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<AccountConstraints>>(aCx, aValue.mAccount, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "account", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<GetCredentialsMetadataArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelConstraints>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<AccountConstraints>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<GetCredentialsMetadataArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, GetCredentialsMetadataArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelConstraints>(aCx, propVal, aRv.mChannelConstraints)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<AccountConstraints>(aCx, propVal, aRv.mAccountConstraints)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<GetCredentialsMetadataArgs>(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, aValue.mWebAppActor, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelConstraints>(aCx, aValue.mChannelConstraints, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelConstraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<AccountConstraints>(aCx, aValue.mAccountConstraints, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "accountConstraints", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<ChannelMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelConstraints>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<CryptoActor>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "e2eeEnabled", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<ChannelMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, ChannelMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelConstraints>(aCx, propVal, aRv.mConstraints)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<CryptoActor>(aCx, propVal, aRv.mScmActor)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "e2eeEnabled", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mE2eeEnabled)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<ChannelMetadata>(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelConstraints>(aCx, aValue.mConstraints, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "constraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<CryptoActor, OriginActor>>(aCx, aValue.mWebAppActor, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<CryptoActor>(aCx, aValue.mScmActor, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmActor", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mE2eeEnabled, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "e2eeEnabled", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<CreateChannelArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<CreateChannelArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, CreateChannelArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelMetadata>(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<CreateChannelArgs>(JSContext* aCx, const CreateChannelArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelMetadata>(aCx, aValue.mChannel, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<RequestContext>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<DocumentMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RequestContext>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContext& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelMetadata>(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<DocumentMetadata>(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RequestContext>(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelMetadata>(aCx, aValue.mChannel, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<DocumentMetadata>(aCx, aValue.mDocument, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<GenerateX25519KeyResult>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "public", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<GenerateX25519KeyResult>(JSContext* aCx, JS::Handle<JS::Value> aValue, GenerateX25519KeyResult& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "public", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mPublic)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<GenerateX25519KeyResult>(JSContext* aCx, const GenerateX25519KeyResult& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mPublic, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "public", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<SignKeyAgreementParametersArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "canonicalJson", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<SignKeyAgreementParametersArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, SignKeyAgreementParametersArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "canonicalJson", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mCanonicalJson)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<SignKeyAgreementParametersArgs>(JSContext* aCx, const SignKeyAgreementParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mCanonicalJson, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "canonicalJson", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<ArrayBuffer>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS::IsArrayBufferObject(obj);
  return true;
}
template<>
bool FromJSVal<ArrayBuffer>(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBuffer& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  if (NS_WARN_IF(!JS::IsArrayBufferObject(obj))) {
    return false;
  }
  size_t length = 0u;
  uint8_t* bytes = nullptr;
  JSObject* buff = JS::GetObjectAsArrayBuffer(obj, &length, &bytes);
  if (NS_WARN_IF(!buff)) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  // TODO(berytus): Check if we need to pass obj or buff
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<ArrayBuffer>(JSContext* aCx, const ArrayBuffer& aValue, JS::MutableHandle<JS::Value> aRv) {
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
  aRv.setObject(*aValue.Obj());
  return true;
}
template<>
bool JSValIs<SignKeyAgreementParametersResult>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<SignKeyAgreementParametersResult>(JSContext* aCx, JS::Handle<JS::Value> aValue, SignKeyAgreementParametersResult& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, propVal, aRv.mScmSignature)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<SignKeyAgreementParametersResult>(JSContext* aCx, const SignKeyAgreementParametersResult& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ArrayBuffer>(aCx, aValue.mScmSignature, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmSignature", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<VerifySignedKeyExchangeParametersArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "canonicalJson", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<VerifySignedKeyExchangeParametersArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, VerifySignedKeyExchangeParametersArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "canonicalJson", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mCanonicalJson)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, propVal, aRv.mWebAppSignature)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<VerifySignedKeyExchangeParametersArgs>(JSContext* aCx, const VerifySignedKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mCanonicalJson, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "canonicalJson", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ArrayBuffer>(aCx, aValue.mWebAppSignature, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppSignature", memberVal1))) {
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
bool ELoginUserIntent::IsPendingDeclaration() const {
  return mVal == uint8_t(0);
}
bool ELoginUserIntent::IsAuthenticate() const {
  return mVal == uint8_t(1);
}
bool ELoginUserIntent::IsRegister() const {
  return mVal == uint8_t(2);
}
void ELoginUserIntent::SetAsPendingDeclaration() {
  mVal = uint8_t(0);
}
void ELoginUserIntent::SetAsAuthenticate() {
  mVal = uint8_t(1);
}
void ELoginUserIntent::SetAsRegister() {
  mVal = uint8_t(2);
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
template<>
bool JSValIs<ELoginUserIntent>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  ELoginUserIntent e;
  aRv = ELoginUserIntent::FromString(strVal, e);
  return true;
}
template<>
bool FromJSVal<ELoginUserIntent>(JSContext* aCx, JS::Handle<JS::Value> aValue, ELoginUserIntent& aRv)
 {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!ELoginUserIntent::ELoginUserIntent::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<ELoginUserIntent>(JSContext* aCx, const ELoginUserIntent& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool JSValIs<StaticString0>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString0::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString0>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString0& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString0>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString0>(JSContext* aCx, const StaticString0& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString1>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString1::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString1>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString1& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString1>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString1>(JSContext* aCx, const StaticString1& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString2>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString2::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString2>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString2& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString2>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString2>(JSContext* aCx, const StaticString2& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString3>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString3::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString3>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString3& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString3>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString3>(JSContext* aCx, const StaticString3& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString4>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString4::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString4>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString4& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString4>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString4>(JSContext* aCx, const StaticString4& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString5>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString5::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString5>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString5& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString5>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString5>(JSContext* aCx, const StaticString5& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString6>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString6::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString6>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString6& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString6>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString6>(JSContext* aCx, const StaticString6& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString7>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString7::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString7>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString7& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString7>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString7>(JSContext* aCx, const StaticString7& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString8>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString8::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString8>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString8& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString8>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString8>(JSContext* aCx, const StaticString8& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString9>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString9::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString9>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString9& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString9>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString9>(JSContext* aCx, const StaticString9& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString10>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString10::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString10>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString10& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString10>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString10>(JSContext* aCx, const StaticString10& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString11>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString11::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString11>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString11& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString11>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString11>(JSContext* aCx, const StaticString11& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString12>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString12::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString12>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString12& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString12>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString12>(JSContext* aCx, const StaticString12& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString0>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString1>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString2>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString3>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString4>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString5>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString6>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString7>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString8>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString9>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString10>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString11>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString12>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString0>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString0 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString0>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString1>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString1 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString1>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString2>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString2 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString2>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString3>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString3 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString3>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString4>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString4 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString4>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString5>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString5 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString5>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString6>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString6 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString6>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString7>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString7 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString7>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString8>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString8 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString8>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<8>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString9>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString9 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString9>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<9>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString10>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString10 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString10>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<10>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString11>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString11 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString11>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<11>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString12>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString12 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString12>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<12>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<13>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(JSContext* aCx, const SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const StaticString0& aVal) {
      return ToJSVal<StaticString0>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString1& aVal) {
      return ToJSVal<StaticString1>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString2& aVal) {
      return ToJSVal<StaticString2>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString3& aVal) {
      return ToJSVal<StaticString3>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString4& aVal) {
      return ToJSVal<StaticString4>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString5& aVal) {
      return ToJSVal<StaticString5>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString6& aVal) {
      return ToJSVal<StaticString6>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString7& aVal) {
      return ToJSVal<StaticString7>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString8& aVal) {
      return ToJSVal<StaticString8>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString9& aVal) {
      return ToJSVal<StaticString9>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString10& aVal) {
      return ToJSVal<StaticString10>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString11& aVal) {
      return ToJSVal<StaticString11>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString12& aVal) {
      return ToJSVal<StaticString12>(mCx, aVal, mRv);
    }

    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<RequestedUserAttribute>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RequestedUserAttribute>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestedUserAttribute& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mRequired)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RequestedUserAttribute>(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mId.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mRequired, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "required", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<nsTArray<RequestedUserAttribute>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  // TODO(berytus): What about the values inside the array?
  return true;
}
template<>
bool FromJSVal<nsTArray<RequestedUserAttribute>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<RequestedUserAttribute>& aRv) {
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
    if (NS_WARN_IF(!(FromJSVal<RequestedUserAttribute>(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
template<>
bool ToJSVal<nsTArray<RequestedUserAttribute>>(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const RequestedUserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(ToJSVal<RequestedUserAttribute>(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}
void EBerytusFieldType::ToString(nsString& aRetVal) const {
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
    aRetVal.Assign(u"ConsumablePassword"_ns);
    return;
  }
  if (mVal == 5) {
    aRetVal.Assign(u"Key"_ns);
    return;
  }
  if (mVal == 6) {
    aRetVal.Assign(u"SharedKey"_ns);
    return;
  }
  if (mVal == 7) {
    aRetVal.Assign(u"Custom"_ns);
    return;
  }
}
EBerytusFieldType EBerytusFieldType::Identity() {
  return EBerytusFieldType(uint8_t(0));
}
EBerytusFieldType EBerytusFieldType::ForeignIdentity() {
  return EBerytusFieldType(uint8_t(1));
}
EBerytusFieldType EBerytusFieldType::Password() {
  return EBerytusFieldType(uint8_t(2));
}
EBerytusFieldType EBerytusFieldType::SecurePassword() {
  return EBerytusFieldType(uint8_t(3));
}
EBerytusFieldType EBerytusFieldType::ConsumablePassword() {
  return EBerytusFieldType(uint8_t(4));
}
EBerytusFieldType EBerytusFieldType::Key() {
  return EBerytusFieldType(uint8_t(5));
}
EBerytusFieldType EBerytusFieldType::SharedKey() {
  return EBerytusFieldType(uint8_t(6));
}
EBerytusFieldType EBerytusFieldType::Custom() {
  return EBerytusFieldType(uint8_t(7));
}
bool EBerytusFieldType::IsIdentity() const {
  return mVal == uint8_t(0);
}
bool EBerytusFieldType::IsForeignIdentity() const {
  return mVal == uint8_t(1);
}
bool EBerytusFieldType::IsPassword() const {
  return mVal == uint8_t(2);
}
bool EBerytusFieldType::IsSecurePassword() const {
  return mVal == uint8_t(3);
}
bool EBerytusFieldType::IsConsumablePassword() const {
  return mVal == uint8_t(4);
}
bool EBerytusFieldType::IsKey() const {
  return mVal == uint8_t(5);
}
bool EBerytusFieldType::IsSharedKey() const {
  return mVal == uint8_t(6);
}
bool EBerytusFieldType::IsCustom() const {
  return mVal == uint8_t(7);
}
void EBerytusFieldType::SetAsIdentity() {
  mVal = uint8_t(0);
}
void EBerytusFieldType::SetAsForeignIdentity() {
  mVal = uint8_t(1);
}
void EBerytusFieldType::SetAsPassword() {
  mVal = uint8_t(2);
}
void EBerytusFieldType::SetAsSecurePassword() {
  mVal = uint8_t(3);
}
void EBerytusFieldType::SetAsConsumablePassword() {
  mVal = uint8_t(4);
}
void EBerytusFieldType::SetAsKey() {
  mVal = uint8_t(5);
}
void EBerytusFieldType::SetAsSharedKey() {
  mVal = uint8_t(6);
}
void EBerytusFieldType::SetAsCustom() {
  mVal = uint8_t(7);
}
bool EBerytusFieldType::FromString(const nsString& aVal, EBerytusFieldType& aRetVal) {
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
if (aVal.Equals(u"ConsumablePassword"_ns)) {
    aRetVal.mVal = 4;
    return true;
  }
if (aVal.Equals(u"Key"_ns)) {
    aRetVal.mVal = 5;
    return true;
  }
if (aVal.Equals(u"SharedKey"_ns)) {
    aRetVal.mVal = 6;
    return true;
  }
if (aVal.Equals(u"Custom"_ns)) {
    aRetVal.mVal = 7;
    return true;
  }
  return false;
}
template<>
bool JSValIs<EBerytusFieldType>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EBerytusFieldType e;
  aRv = EBerytusFieldType::FromString(strVal, e);
  return true;
}
template<>
bool FromJSVal<EBerytusFieldType>(JSContext* aCx, JS::Handle<JS::Value> aValue, EBerytusFieldType& aRv)
 {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EBerytusFieldType::EBerytusFieldType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<EBerytusFieldType>(JSContext* aCx, const EBerytusFieldType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool JSValIs<BerytusFieldCategoryOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "categoryId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "position", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<double>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusFieldCategoryOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusFieldCategoryOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "categoryId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mCategoryId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "position", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<double>>(aCx, propVal, aRv.mPosition)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusFieldCategoryOptions>(JSContext* aCx, const BerytusFieldCategoryOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mCategoryId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "categoryId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<double>>(aCx, aValue.mPosition, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "position", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<Maybe<BerytusFieldCategoryOptions>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return JSValIs<BerytusFieldCategoryOptions>(aCx, aValue, aRv);
}
template<>
bool FromJSVal<Maybe<BerytusFieldCategoryOptions>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<BerytusFieldCategoryOptions>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(FromJSVal<BerytusFieldCategoryOptions>(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<Maybe<BerytusFieldCategoryOptions>>(JSContext* aCx, const Maybe<BerytusFieldCategoryOptions>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return ToJSVal<BerytusFieldCategoryOptions>(aCx, aValue.ref(), aRv);
}
template<>
bool JSValIs<BerytusForeignIdentityFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "kind", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusForeignIdentityFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mPrivate)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "kind", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mKind)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusForeignIdentityFieldOptions>(JSContext* aCx, const BerytusForeignIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mPrivate, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "private", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mKind, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "kind", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusIdentityFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "humanReadable", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<bool>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "maxLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "allowedCharacters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusIdentityFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "humanReadable", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mHumanReadable)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<bool>(aCx, propVal, aRv.mPrivate)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "maxLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mMaxLength)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "allowedCharacters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsString>>(aCx, propVal, aRv.mAllowedCharacters)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusIdentityFieldOptions>(JSContext* aCx, const BerytusIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mHumanReadable, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "humanReadable", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<bool>(aCx, aValue.mPrivate, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "private", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mMaxLength, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "maxLength", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsString>>(aCx, aValue.mAllowedCharacters, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "allowedCharacters", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusKeyFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusKeyFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mAlg)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusKeyFieldOptions>(JSContext* aCx, const BerytusKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mAlg, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "alg", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusPasswordFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "passwordRules", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusPasswordFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "passwordRules", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsString>>(aCx, propVal, aRv.mPasswordRules)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusPasswordFieldOptions>(JSContext* aCx, const BerytusPasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsString>>(aCx, aValue.mPasswordRules, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "passwordRules", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusSecurePasswordFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identityFieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSecurePasswordFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identityFieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mIdentityFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSecurePasswordFieldOptions>(JSContext* aCx, const BerytusSecurePasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mIdentityFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "identityFieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusSharedKeyFieldOptions>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSharedKeyFieldOptions>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mAlg)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSharedKeyFieldOptions>(JSContext* aCx, const BerytusSharedKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mAlg, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "alg", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<BerytusFieldCategoryOptions>>(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusForeignIdentityFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentityFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusForeignIdentityFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusForeignIdentityFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusForeignIdentityFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentityFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusIdentityFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusIdentityFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusPasswordFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusPasswordFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldOptions>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldOptions nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldOptions>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(JSContext* aCx, const SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusForeignIdentityFieldOptions& aVal) {
      return ToJSVal<BerytusForeignIdentityFieldOptions>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusIdentityFieldOptions& aVal) {
      return ToJSVal<BerytusIdentityFieldOptions>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldOptions& aVal) {
      return ToJSVal<BerytusKeyFieldOptions>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusPasswordFieldOptions& aVal) {
      return ToJSVal<BerytusPasswordFieldOptions>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldOptions& aVal) {
      return ToJSVal<BerytusSecurePasswordFieldOptions>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldOptions& aVal) {
      return ToJSVal<BerytusSharedKeyFieldOptions>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<FieldInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EBerytusFieldType>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<FieldInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EBerytusFieldType>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<FieldInfo>(JSContext* aCx, const FieldInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EBerytusFieldType>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mOptions.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<BerytusForeignIdentityFieldOptions, BerytusIdentityFieldOptions, BerytusKeyFieldOptions, BerytusPasswordFieldOptions, BerytusSecurePasswordFieldOptions, BerytusSharedKeyFieldOptions>>(aCx, aValue.mOptions, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<Record<nsString, FieldInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  if (aValue.isNull()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    aRv = false;
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    if (!prop.isString()) {
      aRv = false;
      return true;
    }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      aRv = false;
      return false;
    }
    bool isValid;
    if (NS_WARN_IF((!JSValIs<FieldInfo>(aCx, val, isValid)))) {
      aRv = false;
      return false;
    }
    if (!isValid) {
      aRv = false;
      return true;
    }
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<Record<nsString, FieldInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, FieldInfo>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(aValue.isNull())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    Record<nsString, FieldInfo>::EntryType entry;
    if (NS_WARN_IF(!prop.isString())) {
      return false;
    }
    nsAutoJSString propName;
    if (NS_WARN_IF(!propName.init(aCx, prop))) {
      return false;
    }
    entry.mKey.Assign(propName);

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      return false;
    }
    if (NS_WARN_IF((!FromJSVal<FieldInfo>(aCx, val, entry.mValue)))) {
      return false;
    }
    aRv.Entries().AppendElement(std::move(entry));
  }
  return true;
};
template<>
bool ToJSVal<Record<nsString, FieldInfo>>(JSContext* aCx, const Record<nsString, FieldInfo>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  for (const auto& entry : aValue.Entries()) {
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF((!ToJSVal<FieldInfo>(aCx, entry.mValue, &val)))) {
      return false;
    }
    nsCString propName = NS_ConvertUTF16toUTF8(entry.mKey);
    if (NS_WARN_IF(!JS_SetProperty(aCx, obj, propName.BeginReading(), val))) {
      return false;
    }
  }
  aRv.setObjectOrNull(obj);
  return true;
}
template<>
bool JSValIs<StaticString13>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString13::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString13>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString13& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString13>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString13>(JSContext* aCx, const StaticString13& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<JSNull>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isNull();
  return true;
}
template<>
bool FromJSVal<JSNull>(JSContext* aCx, JS::Handle<JS::Value> aValue, JSNull& aRv) {
  if (NS_WARN_IF(!aValue.isNull())) {
    return false;
  }
  // Nothing to do...
  return true;
}
template<>
bool ToJSVal<JSNull>(JSContext* aCx, const JSNull& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setNull();
  return true;
}
template<>
bool JSValIs<BerytusIdentificationChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString13>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusIdentificationChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentificationChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString13>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusIdentificationChallengeInfo>(JSContext* aCx, const BerytusIdentificationChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString13>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<JSNull>(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString14>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString14::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString14>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString14& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString14>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString14>(JSContext* aCx, const StaticString14& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusPasswordChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString14>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusPasswordChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString14>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusPasswordChallengeInfo>(JSContext* aCx, const BerytusPasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString14>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<JSNull>(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString15>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString15::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString15>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString15& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString15>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString15>(JSContext* aCx, const StaticString15& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusDigitalSignatureChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString15>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusDigitalSignatureChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusDigitalSignatureChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString15>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusDigitalSignatureChallengeInfo>(JSContext* aCx, const BerytusDigitalSignatureChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString15>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<JSNull>(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString16>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString16::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString16>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString16& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString16>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString16>(JSContext* aCx, const StaticString16& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString17>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString17::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString17>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString17& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString17>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString17>(JSContext* aCx, const StaticString17& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString18>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString18::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString18>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString18& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString18>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString18>(JSContext* aCx, const StaticString18& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<Nothing>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isUndefined();
  return true;
}
template<>
bool FromJSVal<Nothing>(JSContext* aCx, JS::Handle<JS::Value> aValue, Nothing& aRv) {
  if (NS_WARN_IF(!aValue.isUndefined())) {
    return false;
  }
  // Nothing to do...
  return true;
}
template<>
bool ToJSVal<Nothing>(JSContext* aCx, const Nothing& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setUndefined();
  return true;
}
template<>
bool JSValIs<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString17>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString18>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<Nothing>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString17, StaticString18, Nothing>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString17>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString17 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString17>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString18>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString18 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString18>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<Nothing>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      Nothing nv;
      if (NS_WARN_IF(!(FromJSVal<Nothing>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(JSContext* aCx, const SafeVariant<StaticString17, StaticString18, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const StaticString17& aVal) {
      return ToJSVal<StaticString17>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString18& aVal) {
      return ToJSVal<StaticString18>(mCx, aVal, mRv);
    }

    bool operator()(const Nothing& aVal) {
      return ToJSVal<Nothing>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusSecureRemotePasswordChallengeParameters>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "encoding", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<StaticString17, StaticString18, Nothing>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSecureRemotePasswordChallengeParameters>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeParameters& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "encoding", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(aCx, propVal, aRv.mEncoding)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSecureRemotePasswordChallengeParameters>(JSContext* aCx, const BerytusSecureRemotePasswordChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mEncoding.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<StaticString17, StaticString18, Nothing>>(aCx, aValue.mEncoding, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "encoding", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusSecureRemotePasswordChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString16>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeParameters>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString16>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeParameters>(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(JSContext* aCx, const BerytusSecureRemotePasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString16>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecureRemotePasswordChallengeParameters>(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString19>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString19::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString19>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString19& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString19>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString19>(JSContext* aCx, const StaticString19& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusOffChannelOtpChallengeInfo>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString19>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusOffChannelOtpChallengeInfo>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusOffChannelOtpChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString19>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusOffChannelOtpChallengeInfo>(JSContext* aCx, const BerytusOffChannelOtpChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString19>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<JSNull>(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentificationChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusDigitalSignatureChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusOffChannelOtpChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentificationChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusIdentificationChallengeInfo nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusIdentificationChallengeInfo>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusPasswordChallengeInfo nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusPasswordChallengeInfo>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusDigitalSignatureChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusDigitalSignatureChallengeInfo nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusDigitalSignatureChallengeInfo>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecureRemotePasswordChallengeInfo nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusOffChannelOtpChallengeInfo>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusOffChannelOtpChallengeInfo nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusOffChannelOtpChallengeInfo>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(JSContext* aCx, const SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusIdentificationChallengeInfo& aVal) {
      return ToJSVal<BerytusIdentificationChallengeInfo>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusPasswordChallengeInfo& aVal) {
      return ToJSVal<BerytusPasswordChallengeInfo>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusDigitalSignatureChallengeInfo& aVal) {
      return ToJSVal<BerytusDigitalSignatureChallengeInfo>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecureRemotePasswordChallengeInfo& aVal) {
      return ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusOffChannelOtpChallengeInfo& aVal) {
      return ToJSVal<BerytusOffChannelOtpChallengeInfo>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  if (aValue.isNull()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    aRv = false;
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    if (!prop.isString()) {
      aRv = false;
      return true;
    }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      aRv = false;
      return false;
    }
    bool isValid;
    if (NS_WARN_IF((!JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, val, isValid)))) {
      aRv = false;
      return false;
    }
    if (!isValid) {
      aRv = false;
      return true;
    }
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(aValue.isNull())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>::EntryType entry;
    if (NS_WARN_IF(!prop.isString())) {
      return false;
    }
    nsAutoJSString propName;
    if (NS_WARN_IF(!propName.init(aCx, prop))) {
      return false;
    }
    entry.mKey.Assign(propName);

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      return false;
    }
    if (NS_WARN_IF((!FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, val, entry.mValue)))) {
      return false;
    }
    aRv.Entries().AppendElement(std::move(entry));
  }
  return true;
};
template<>
bool ToJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(JSContext* aCx, const Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  for (const auto& entry : aValue.Entries()) {
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF((!ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, entry.mValue, &val)))) {
      return false;
    }
    nsCString propName = NS_ConvertUTF16toUTF8(entry.mKey);
    if (NS_WARN_IF(!JS_SetProperty(aCx, obj, propName.BeginReading(), val))) {
      return false;
    }
  }
  aRv.setObjectOrNull(obj);
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
bool EOperationType::IsPendingDeclaration() const {
  return mVal == uint8_t(0);
}
bool EOperationType::IsRegistration() const {
  return mVal == uint8_t(1);
}
bool EOperationType::IsAuthentication() const {
  return mVal == uint8_t(2);
}
void EOperationType::SetAsPendingDeclaration() {
  mVal = uint8_t(0);
}
void EOperationType::SetAsRegistration() {
  mVal = uint8_t(1);
}
void EOperationType::SetAsAuthentication() {
  mVal = uint8_t(2);
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
template<>
bool JSValIs<EOperationType>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EOperationType e;
  aRv = EOperationType::FromString(strVal, e);
  return true;
}
template<>
bool FromJSVal<EOperationType>(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationType& aRv)
 {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationType::EOperationType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<EOperationType>(JSContext* aCx, const EOperationType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
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
bool EOperationStatus::IsPending() const {
  return mVal == uint8_t(0);
}
bool EOperationStatus::IsCreated() const {
  return mVal == uint8_t(1);
}
bool EOperationStatus::IsAborted() const {
  return mVal == uint8_t(2);
}
bool EOperationStatus::IsFinished() const {
  return mVal == uint8_t(3);
}
void EOperationStatus::SetAsPending() {
  mVal = uint8_t(0);
}
void EOperationStatus::SetAsCreated() {
  mVal = uint8_t(1);
}
void EOperationStatus::SetAsAborted() {
  mVal = uint8_t(2);
}
void EOperationStatus::SetAsFinished() {
  mVal = uint8_t(3);
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
template<>
bool JSValIs<EOperationStatus>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EOperationStatus e;
  aRv = EOperationStatus::FromString(strVal, e);
  return true;
}
template<>
bool FromJSVal<EOperationStatus>(JSContext* aCx, JS::Handle<JS::Value> aValue, EOperationStatus& aRv)
 {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EOperationStatus::EOperationStatus::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<EOperationStatus>(JSContext* aCx, const EOperationStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool JSValIs<OperationState>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;


}
template<>
bool FromJSVal<OperationState>(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationState& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  return true;
}
            
template<>
bool ToJSVal<OperationState>(JSContext* aCx, const OperationState& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<LoginOperationMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ELoginUserIntent>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsTArray<RequestedUserAttribute>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fields", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Record<nsString, FieldInfo>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenges", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EOperationType>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EOperationStatus>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<OperationState>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<LoginOperationMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, LoginOperationMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ELoginUserIntent>(aCx, propVal, aRv.mIntent)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsTArray<RequestedUserAttribute>>(aCx, propVal, aRv.mRequestedUserAttributes)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fields", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Record<nsString, FieldInfo>>(aCx, propVal, aRv.mFields)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenges", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(aCx, propVal, aRv.mChallenges)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EOperationType>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EOperationStatus>(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<OperationState>(aCx, propVal, aRv.mState)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<LoginOperationMetadata>(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ELoginUserIntent>(aCx, aValue.mIntent, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "intent", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsTArray<RequestedUserAttribute>>(aCx, aValue.mRequestedUserAttributes, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "requestedUserAttributes", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Record<nsString, FieldInfo>>(aCx, aValue.mFields, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fields", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Record<nsString, SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>>(aCx, aValue.mChallenges, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenges", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal4))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal5(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EOperationType>(aCx, aValue.mType, &memberVal5)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal5))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal6(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EOperationStatus>(aCx, aValue.mStatus, &memberVal6)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal6))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal7(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<OperationState>(aCx, aValue.mState, &memberVal7)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "state", memberVal7))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<ApproveOperationArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<LoginOperationMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<ApproveOperationArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveOperationArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<LoginOperationMetadata>(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<ApproveOperationArgs>(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<LoginOperationMetadata>(aCx, aValue.mOperation, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<OperationMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EOperationType>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EOperationStatus>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<OperationState>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<OperationMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, OperationMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EOperationType>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EOperationStatus>(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<OperationState>(aCx, propVal, aRv.mState)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<OperationMetadata>(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EOperationType>(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EOperationStatus>(aCx, aValue.mStatus, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<OperationState>(aCx, aValue.mState, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "state", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<RequestContextWithOperation>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<OperationMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<DocumentMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RequestContextWithOperation>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithOperation& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<OperationMetadata>(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelMetadata>(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<DocumentMetadata>(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RequestContextWithOperation>(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<OperationMetadata>(aCx, aValue.mOperation, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelMetadata>(aCx, aValue.mChannel, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<DocumentMetadata>(aCx, aValue.mDocument, &memberVal2)))) {
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
bool EMetadataStatus::IsPending() const {
  return mVal == uint8_t(0);
}
bool EMetadataStatus::IsCreated() const {
  return mVal == uint8_t(1);
}
bool EMetadataStatus::IsRetired() const {
  return mVal == uint8_t(2);
}
void EMetadataStatus::SetAsPending() {
  mVal = uint8_t(0);
}
void EMetadataStatus::SetAsCreated() {
  mVal = uint8_t(1);
}
void EMetadataStatus::SetAsRetired() {
  mVal = uint8_t(2);
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
template<>
bool JSValIs<EMetadataStatus>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EMetadataStatus e;
  aRv = EMetadataStatus::FromString(strVal, e);
  return true;
}
template<>
bool FromJSVal<EMetadataStatus>(JSContext* aCx, JS::Handle<JS::Value> aValue, EMetadataStatus& aRv)
 {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!FromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EMetadataStatus::EMetadataStatus::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<EMetadataStatus>(JSContext* aCx, const EMetadataStatus& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!ToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
template<>
bool JSValIs<RecordMetadata>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<double>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<EMetadataStatus>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RecordMetadata>(JSContext* aCx, JS::Handle<JS::Value> aValue, RecordMetadata& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<double>(aCx, propVal, aRv.mVersion)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<EMetadataStatus>(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mChangePassUrl)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RecordMetadata>(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<double>(aCx, aValue.mVersion, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "version", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<EMetadataStatus>(aCx, aValue.mStatus, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mCategory, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mChangePassUrl, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "changePassUrl", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<UpdateMetadataArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<RecordMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<UpdateMetadataArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateMetadataArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<RecordMetadata>(aCx, propVal, aRv.mMetadata)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<UpdateMetadataArgs>(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<RecordMetadata>(aCx, aValue.mMetadata, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "metadata", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<ApproveTransitionToAuthOpArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<LoginOperationMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<ApproveTransitionToAuthOpArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveTransitionToAuthOpArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<LoginOperationMetadata>(aCx, propVal, aRv.mNewAuthOp)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<ApproveTransitionToAuthOpArgs>(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<LoginOperationMetadata>(aCx, aValue.mNewAuthOp, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "newAuthOp", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<RequestContextWithLoginOperation>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<LoginOperationMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<ChannelMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<DocumentMetadata>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RequestContextWithLoginOperation>(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithLoginOperation& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<LoginOperationMetadata>(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<ChannelMetadata>(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<DocumentMetadata>(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RequestContextWithLoginOperation>(JSContext* aCx, const RequestContextWithLoginOperation& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<LoginOperationMetadata>(aCx, aValue.mOperation, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<ChannelMetadata>(aCx, aValue.mChannel, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<DocumentMetadata>(aCx, aValue.mDocument, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<ArrayBufferView>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS_IsArrayBufferViewObject(obj);
  return true;
}
template<>
bool FromJSVal<ArrayBufferView>(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBufferView& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  if (NS_WARN_IF(!JS_IsArrayBufferViewObject(obj))) {
    return false;
  }
  bool isShared;
  JSObject* buff = JS_GetArrayBufferViewBuffer(aCx, obj, &isShared);
  if (NS_WARN_IF(!buff)) {
    return false;
  }
  if (NS_WARN_IF(aRv.inited())) {
    return false;
  }
  // TODO(berytus): Test if we need to pass the obj or buff
  if (NS_WARN_IF(!aRv.Init(obj))) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<ArrayBufferView>(JSContext* aCx, const ArrayBufferView& aValue, JS::MutableHandle<JS::Value> aRv) {
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
  aRv.setObject(*aValue.Obj());
  return true;
}
template<>
bool JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(FromJSVal<ArrayBufferView>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView>>(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ToJSVal<ArrayBuffer>(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ToJSVal<ArrayBufferView>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<StaticString20>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString20::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString20>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString20& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString20>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString20>(JSContext* aCx, const StaticString20& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusEncryptedPacket>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString20>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusEncryptedPacket>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusEncryptedPacket& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString20>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusEncryptedPacket>(JSContext* aCx, const BerytusEncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString20>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mValue, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<1>(), ArrayBuffer());
      if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(FromJSVal<ArrayBufferView>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBuffer& aVal) {
      return ToJSVal<ArrayBuffer>(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ToJSVal<ArrayBufferView>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<UserAttribute>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "info", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Maybe<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<UserAttribute>(JSContext* aCx, JS::Handle<JS::Value> aValue, UserAttribute& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "info", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsString>>(aCx, propVal, aRv.mInfo)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Maybe<nsString>>(aCx, propVal, aRv.mMimeType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<UserAttribute>(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mId.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<StaticString0, StaticString1, StaticString2, StaticString3, StaticString4, StaticString5, StaticString6, StaticString7, StaticString8, StaticString9, StaticString10, StaticString11, StaticString12, nsString>>(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsString>>(aCx, aValue.mInfo, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "info", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Maybe<nsString>>(aCx, aValue.mMimeType, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "mimeType", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, aValue.mValue, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<nsTArray<UserAttribute>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  // TODO(berytus): What about the values inside the array?
  return true;
}
template<>
bool FromJSVal<nsTArray<UserAttribute>>(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<UserAttribute>& aRv) {
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
    if (NS_WARN_IF(!(FromJSVal<UserAttribute>(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
template<>
bool ToJSVal<nsTArray<UserAttribute>>(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const UserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(ToJSVal<UserAttribute>(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}

template<>
bool JSValIs<UpdateUserAttributesArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "userAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsTArray<UserAttribute>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<UpdateUserAttributesArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateUserAttributesArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "userAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsTArray<UserAttribute>>(aCx, propVal, aRv.mUserAttributes)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<UpdateUserAttributesArgs>(JSContext* aCx, const UpdateUserAttributesArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsTArray<UserAttribute>>(aCx, aValue.mUserAttributes, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "userAttributes", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<StaticString21>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString21::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString21>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString21& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString21>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString21>(JSContext* aCx, const StaticString21& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return ToJSVal<JSNull>(mCx, aVal, mRv);
    }

    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusForeignIdentityField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString21>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusForeignIdentityFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusForeignIdentityField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString21>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusForeignIdentityFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusForeignIdentityField>(JSContext* aCx, const BerytusForeignIdentityField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString21>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusForeignIdentityFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString22>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString22::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString22>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString22& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString22>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString22>(JSContext* aCx, const StaticString22& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusIdentityField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString22>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusIdentityFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusIdentityField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString22>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusIdentityFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusIdentityField>(JSContext* aCx, const BerytusIdentityField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString22>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusIdentityFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString23>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString23::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString23>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString23& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString23>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString23>(JSContext* aCx, const StaticString23& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ToJSVal<ArrayBuffer>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusKeyFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "publicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusKeyFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "publicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPublicKey)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusKeyFieldValue>(JSContext* aCx, const BerytusKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mPublicKey.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mPublicKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "publicKey", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return ToJSVal<JSNull>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return ToJSVal<BerytusKeyFieldValue>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusKeyField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString23>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, BerytusKeyFieldValue>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusKeyField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString23>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusKeyField>(JSContext* aCx, const BerytusKeyField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString23>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusKeyFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, BerytusKeyFieldValue>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusPasswordField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString14>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusPasswordFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusPasswordField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString14>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusPasswordFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusPasswordField>(JSContext* aCx, const BerytusPasswordField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString14>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusPasswordFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString24>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString24::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString24>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString24& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString24>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString24>(JSContext* aCx, const StaticString24& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSecurePasswordFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "salt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "verifier", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSecurePasswordFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "salt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mSalt)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "verifier", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mVerifier)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSecurePasswordFieldValue>(JSContext* aCx, const BerytusSecurePasswordFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mSalt.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mSalt, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "salt", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  if (NS_WARN_IF(!aValue.mVerifier.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mVerifier, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "verifier", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return ToJSVal<JSNull>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return ToJSVal<BerytusSecurePasswordFieldValue>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusSecurePasswordField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString24>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSecurePasswordField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString24>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSecurePasswordField>(JSContext* aCx, const BerytusSecurePasswordField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString24>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecurePasswordFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, BerytusSecurePasswordFieldValue>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString25>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString25::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString25>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString25& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString25>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString25>(JSContext* aCx, const StaticString25& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSharedKeyFieldValue>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "privateKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSharedKeyFieldValue>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "privateKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPrivateKey)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSharedKeyFieldValue>(JSContext* aCx, const BerytusSharedKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mPrivateKey.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mPrivateKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "privateKey", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return ToJSVal<JSNull>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return ToJSVal<BerytusSharedKeyFieldValue>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusSharedKeyField>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString25>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldOptions>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSharedKeyField>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString25>(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldOptions>(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSharedKeyField>(JSContext* aCx, const BerytusSharedKeyField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString25>(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSharedKeyFieldOptions>(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<JSNull, BerytusSharedKeyFieldValue>>(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusForeignIdentityField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentityField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusForeignIdentityField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusForeignIdentityField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusForeignIdentityField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusIdentityField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusIdentityField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusIdentityField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusPasswordField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusPasswordField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusPasswordField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyField>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyField nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyField>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(JSContext* aCx, const SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusForeignIdentityField& aVal) {
      return ToJSVal<BerytusForeignIdentityField>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusIdentityField& aVal) {
      return ToJSVal<BerytusIdentityField>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyField& aVal) {
      return ToJSVal<BerytusKeyField>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusPasswordField& aVal) {
      return ToJSVal<BerytusPasswordField>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordField& aVal) {
      return ToJSVal<BerytusSecurePasswordField>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyField& aVal) {
      return ToJSVal<BerytusSharedKeyField>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<AddFieldArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<AddFieldArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, AddFieldArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(aCx, propVal, aRv.mField)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<AddFieldArgs>(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mField.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>>(aCx, aValue.mField, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "field", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return ToJSVal<BerytusKeyFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return ToJSVal<BerytusSecurePasswordFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return ToJSVal<BerytusSharedKeyFieldValue>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<JSNull>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(FromJSVal<JSNull>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return ToJSVal<JSNull>(mCx, aVal, mRv);
    }

    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return ToJSVal<BerytusKeyFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return ToJSVal<BerytusSecurePasswordFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return ToJSVal<BerytusSharedKeyFieldValue>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}

template<>
bool JSValIs<FieldValueRejectionReason>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<FieldValueRejectionReason>(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldValueRejectionReason& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mCode)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<FieldValueRejectionReason>(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mCode, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "code", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<Nothing>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSecurePasswordFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSecurePasswordFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSharedKeyFieldValue>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSharedKeyFieldValue>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<Nothing>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      Nothing nv;
      if (NS_WARN_IF(!(FromJSVal<Nothing>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return ToJSVal<BerytusKeyFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return ToJSVal<BerytusSecurePasswordFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return ToJSVal<BerytusSharedKeyFieldValue>(mCx, aVal, mRv);
    }

    bool operator()(const Nothing& aVal) {
      return ToJSVal<Nothing>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<RejectFieldValueArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<FieldValueRejectionReason>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<RejectFieldValueArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, RejectFieldValueArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<FieldValueRejectionReason>(aCx, propVal, aRv.mReason)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(aCx, propVal, aRv.mOptionalNewValue)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<RejectFieldValueArgs>(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<FieldValueRejectionReason>(aCx, aValue.mReason, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mOptionalNewValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>>(aCx, aValue.mOptionalNewValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "optionalNewValue", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<ApproveChallengeRequestArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<ApproveChallengeRequestArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, ApproveChallengeRequestArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<ApproveChallengeRequestArgs>(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mChallenge.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<StaticString26>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString26::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString26>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString26& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString26>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString26>(JSContext* aCx, const StaticString26& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString27>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString27::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString27>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString27& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString27>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString27>(JSContext* aCx, const StaticString27& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString28>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString28::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString28>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString28& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString28>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString28>(JSContext* aCx, const StaticString28& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString29>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString29::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString29>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString29& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString29>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString29>(JSContext* aCx, const StaticString29& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString30>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString30::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString30>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString30& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString30>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString30>(JSContext* aCx, const StaticString30& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString31>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString31::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString31>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString31& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString31>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString31>(JSContext* aCx, const StaticString31& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString32>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString32::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString32>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString32& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString32>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString32>(JSContext* aCx, const StaticString32& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<StaticString33>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString33::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString33>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString33& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString33>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString33>(JSContext* aCx, const StaticString33& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString26>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString27>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString28>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString29>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString30>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString31>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString32>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString33>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString26>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString26 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString26>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString27>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString27 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString27>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString28>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString28 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString28>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString29>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString29 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString29>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString30>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString30 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString30>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString31>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString31 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString31>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString32>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString32 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString32>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<StaticString33>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      StaticString33 nv;
      if (NS_WARN_IF(!(FromJSVal<StaticString33>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(JSContext* aCx, const SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const StaticString26& aVal) {
      return ToJSVal<StaticString26>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString27& aVal) {
      return ToJSVal<StaticString27>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString28& aVal) {
      return ToJSVal<StaticString28>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString29& aVal) {
      return ToJSVal<StaticString29>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString30& aVal) {
      return ToJSVal<StaticString30>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString31& aVal) {
      return ToJSVal<StaticString31>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString32& aVal) {
      return ToJSVal<StaticString32>(mCx, aVal, mRv);
    }

    bool operator()(const StaticString33& aVal) {
      return ToJSVal<StaticString33>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<AbortChallengeArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<AbortChallengeArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, AbortChallengeArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(aCx, propVal, aRv.mReason)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<AbortChallengeArgs>(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mChallenge.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  if (NS_WARN_IF(!aValue.mReason.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<StaticString26, StaticString27, StaticString28, StaticString29, StaticString30, StaticString31, StaticString32, StaticString33>>(aCx, aValue.mReason, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<CloseChallengeArgs>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<CloseChallengeArgs>(JSContext* aCx, JS::Handle<JS::Value> aValue, CloseChallengeArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<CloseChallengeArgs>(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mChallenge.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<BerytusIdentificationChallengeInfo, BerytusPasswordChallengeInfo, BerytusDigitalSignatureChallengeInfo, BerytusSecureRemotePasswordChallengeInfo, BerytusOffChannelOtpChallengeInfo>>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


template<>
bool JSValIs<StaticString34>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString34::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString34>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString34& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString34>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString34>(JSContext* aCx, const StaticString34& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendGetIdentityFieldsMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusIdentificationChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString34>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsTArray<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendGetIdentityFieldsMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetIdentityFieldsMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusIdentificationChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString34>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsTArray<nsString>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendGetIdentityFieldsMessage>(JSContext* aCx, const BerytusSendGetIdentityFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusIdentificationChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString34>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsTArray<nsString>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString35>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString35::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString35>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString35& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString35>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString35>(JSContext* aCx, const StaticString35& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendGetPasswordFieldsMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusPasswordChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString35>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsTArray<nsString>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendGetPasswordFieldsMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetPasswordFieldsMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusPasswordChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString35>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsTArray<nsString>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendGetPasswordFieldsMessage>(JSContext* aCx, const BerytusSendGetPasswordFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusPasswordChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString35>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsTArray<nsString>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString36>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString36::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString36>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString36& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString36>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString36>(JSContext* aCx, const StaticString36& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendSelectKeyMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusDigitalSignatureChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString36>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendSelectKeyMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectKeyMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusDigitalSignatureChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString36>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendSelectKeyMessage>(JSContext* aCx, const BerytusSendSelectKeyMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusDigitalSignatureChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString36>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString37>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString37::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString37>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString37& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString37>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString37>(JSContext* aCx, const StaticString37& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBufferView>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(FromJSVal<ArrayBufferView>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ToJSVal<ArrayBuffer>(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ToJSVal<ArrayBufferView>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusSendSignNonceMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusDigitalSignatureChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString37>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendSignNonceMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSignNonceMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusDigitalSignatureChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString37>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendSignNonceMessage>(JSContext* aCx, const BerytusSendSignNonceMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusDigitalSignatureChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString37>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString38>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString38::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString38>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString38& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString38>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString38>(JSContext* aCx, const StaticString38& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendSelectSecurePasswordMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString38>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendSelectSecurePasswordMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectSecurePasswordMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString38>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendSelectSecurePasswordMessage>(JSContext* aCx, const BerytusSendSelectSecurePasswordMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString38>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString39>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString39::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString39>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString39& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString39>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString39>(JSContext* aCx, const StaticString39& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendExchangePublicKeysMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString39>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendExchangePublicKeysMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendExchangePublicKeysMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString39>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendExchangePublicKeysMessage>(JSContext* aCx, const BerytusSendExchangePublicKeysMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString39>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString40>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString40::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString40>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString40& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString40>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString40>(JSContext* aCx, const StaticString40& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendComputeClientProofMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString40>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendComputeClientProofMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendComputeClientProofMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString40>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendComputeClientProofMessage>(JSContext* aCx, const BerytusSendComputeClientProofMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString40>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString41>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString41::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString41>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString41& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString41>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString41>(JSContext* aCx, const StaticString41& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendVerifyServerProofMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString41>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendVerifyServerProofMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendVerifyServerProofMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString41>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendVerifyServerProofMessage>(JSContext* aCx, const BerytusSendVerifyServerProofMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusSecureRemotePasswordChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString41>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<StaticString42>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isString()) {
    aRv = false;
    return true;
  }
  nsString val;
  if (NS_WARN_IF((!FromJSVal<nsString>(aCx, aValue, val)))) {
    return false;
  }
  if (!val.Equals(StaticString42::mLiteral)) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<StaticString42>(JSContext* aCx, JS::Handle<JS::Value> aValue, StaticString42& aRv) {
  bool rv;
  if (NS_WARN_IF(!(JSValIs<StaticString42>(aCx, aValue, rv)))) {
    return false;
  }
  if (NS_WARN_IF(!rv)) {
    return false;
  }
  return true;
}
template<>
bool ToJSVal<StaticString42>(JSContext* aCx, const StaticString42& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSString*> rStr(aCx, JS_NewUCStringCopyN(aCx, aValue.GetString().BeginReading(), aValue.GetString().Length()));
  aRv.setString(rStr);
  return true;
}
template<>
bool JSValIs<BerytusSendGetOtpMessage>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusOffChannelOtpChallengeInfo>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<StaticString42>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<nsString>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusSendGetOtpMessage>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetOtpMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusOffChannelOtpChallengeInfo>(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<StaticString42>(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusSendGetOtpMessage>(JSContext* aCx, const BerytusSendGetOtpMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusOffChannelOtpChallengeInfo>(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<StaticString42>(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<nsString>(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetIdentityFieldsMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetPasswordFieldsMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSelectKeyMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSignNonceMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSelectSecurePasswordMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendExchangePublicKeysMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendComputeClientProofMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendVerifyServerProofMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetOtpMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetIdentityFieldsMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendGetIdentityFieldsMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendGetIdentityFieldsMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetPasswordFieldsMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendGetPasswordFieldsMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendGetPasswordFieldsMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSelectKeyMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSelectKeyMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendSelectKeyMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSignNonceMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSignNonceMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendSignNonceMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendSelectSecurePasswordMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSelectSecurePasswordMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendSelectSecurePasswordMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendExchangePublicKeysMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendExchangePublicKeysMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendExchangePublicKeysMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendComputeClientProofMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendComputeClientProofMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendComputeClientProofMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendVerifyServerProofMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendVerifyServerProofMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendVerifyServerProofMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusSendGetOtpMessage>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendGetOtpMessage nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusSendGetOtpMessage>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<8>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>>(JSContext* aCx, const SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusSendGetIdentityFieldsMessage& aVal) {
      return ToJSVal<BerytusSendGetIdentityFieldsMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendGetPasswordFieldsMessage& aVal) {
      return ToJSVal<BerytusSendGetPasswordFieldsMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSelectKeyMessage& aVal) {
      return ToJSVal<BerytusSendSelectKeyMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSignNonceMessage& aVal) {
      return ToJSVal<BerytusSendSignNonceMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSelectSecurePasswordMessage& aVal) {
      return ToJSVal<BerytusSendSelectSecurePasswordMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendExchangePublicKeysMessage& aVal) {
      return ToJSVal<BerytusSendExchangePublicKeysMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendComputeClientProofMessage& aVal) {
      return ToJSVal<BerytusSendComputeClientProofMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendVerifyServerProofMessage& aVal) {
      return ToJSVal<BerytusSendVerifyServerProofMessage>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendGetOtpMessage& aVal) {
      return ToJSVal<BerytusSendGetOtpMessage>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  if (aValue.isNull()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    aRv = false;
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    if (!prop.isString()) {
      aRv = false;
      return true;
    }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      aRv = false;
      return false;
    }
    bool isValid;
    if (NS_WARN_IF((!JSValIs<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, val, isValid)))) {
      aRv = false;
      return false;
    }
    if (!isValid) {
      aRv = false;
      return true;
    }
  }
  aRv = true;
  return true;
}
template<>
bool FromJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(aValue.isNull())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObjectOrNull());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>::EntryType entry;
    if (NS_WARN_IF(!prop.isString())) {
      return false;
    }
    nsAutoJSString propName;
    if (NS_WARN_IF(!propName.init(aCx, prop))) {
      return false;
    }
    entry.mKey.Assign(propName);

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      return false;
    }
    if (NS_WARN_IF((!FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, val, entry.mValue)))) {
      return false;
    }
    aRv.Entries().AppendElement(std::move(entry));
  }
  return true;
};
template<>
bool ToJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(JSContext* aCx, const Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  for (const auto& entry : aValue.Entries()) {
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF((!ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, entry.mValue, &val)))) {
      return false;
    }
    nsCString propName = NS_ConvertUTF16toUTF8(entry.mKey);
    if (NS_WARN_IF(!JS_SetProperty(aCx, obj, propName.BeginReading(), val))) {
      return false;
    }
  }
  aRv.setObjectOrNull(obj);
  return true;
}
template<>
bool JSValIs<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetIdentityFieldsMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(JSContext* aCx, const BerytusChallengeGetIdentityFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetPasswordFieldsMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(JSContext* aCx, const BerytusChallengeGetPasswordFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<Record<nsString, SafeVariant<nsString, BerytusEncryptedPacket>>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeSelectKeyMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<BerytusKeyFieldValue>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeSelectKeyMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectKeyMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<BerytusKeyFieldValue>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeSelectKeyMessageResponse>(JSContext* aCx, const BerytusChallengeSelectKeyMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ToJSVal<BerytusKeyFieldValue>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeSignNonceMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeSignNonceMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSignNonceMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeSignNonceMessageResponse>(JSContext* aCx, const BerytusChallengeSignNonceMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectSecurePasswordMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(JSContext* aCx, const BerytusChallengeSelectSecurePasswordMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<nsString>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<ArrayBuffer>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<1>(), ArrayBuffer());
      if (NS_WARN_IF(!(FromJSVal<ArrayBuffer>(aCx, aValue, (aRv.InternalValue())->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusEncryptedPacket>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusEncryptedPacket>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return ToJSVal<nsString>(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBuffer& aVal) {
      return ToJSVal<ArrayBuffer>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return ToJSVal<BerytusEncryptedPacket>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template<>
bool JSValIs<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeExchangePublicKeysMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(JSContext* aCx, const BerytusChallengeExchangePublicKeysMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeComputeClientProofMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeComputeClientProofMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeComputeClientProofMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeComputeClientProofMessageResponse>(JSContext* aCx, const BerytusChallengeComputeClientProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeVerifyServerProofMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeVerifyServerProofMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeVerifyServerProofMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeVerifyServerProofMessageResponse>(JSContext* aCx, const BerytusChallengeVerifyServerProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<BerytusChallengeGetOtpMessageResponse>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIs<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
template<>
bool FromJSVal<BerytusChallengeGetOtpMessageResponse>(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetOtpMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FromJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
template<>
bool ToJSVal<BerytusChallengeGetOtpMessageResponse>(JSContext* aCx, const BerytusChallengeGetOtpMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(ToJSVal<SafeVariant<nsString, BerytusEncryptedPacket>>(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

template<>
bool JSValIs<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetIdentityFieldsMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetPasswordFieldsMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSelectKeyMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSignNonceMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSelectSecurePasswordMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeExchangePublicKeysMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeComputeClientProofMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeVerifyServerProofMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetOtpMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);

  aRv = false;
  return true;
}
template<>
bool FromJSVal<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetIdentityFieldsMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeGetIdentityFieldsMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetPasswordFieldsMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeGetPasswordFieldsMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSelectKeyMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSelectKeyMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeSelectKeyMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSignNonceMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSignNonceMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeSignNonceMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeSelectSecurePasswordMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSelectSecurePasswordMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeExchangePublicKeysMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeExchangePublicKeysMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeComputeClientProofMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeComputeClientProofMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeComputeClientProofMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeVerifyServerProofMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeVerifyServerProofMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeVerifyServerProofMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIs<BerytusChallengeGetOtpMessageResponse>(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeGetOtpMessageResponse nv;
      if (NS_WARN_IF(!(FromJSVal<BerytusChallengeGetOtpMessageResponse>(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<8>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
template<>
bool ToJSVal<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(JSContext* aCx, const SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusChallengeGetIdentityFieldsMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeGetIdentityFieldsMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeGetPasswordFieldsMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeGetPasswordFieldsMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSelectKeyMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeSelectKeyMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSignNonceMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeSignNonceMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSelectSecurePasswordMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeSelectSecurePasswordMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeExchangePublicKeysMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeExchangePublicKeysMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeComputeClientProofMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeComputeClientProofMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeVerifyServerProofMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeVerifyServerProofMessageResponse>(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeGetOtpMessageResponse& aVal) {
      return ToJSVal<BerytusChallengeGetOtpMessageResponse>(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}


RefPtr<ManagerGetSigningKeyResult> AgentProxy::Manager_GetSigningKey(const PreliminaryRequestContext& aContext, const GetSigningKeyArgs& aArgs) {
  RefPtr<ManagerGetSigningKeyResult::Private> outPromise = new ManagerGetSigningKeyResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Manager_GetSigningKey:onResolve()"));
    nsString out;
    if (NS_WARN_IF(!(FromJSVal<nsString>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Manager_GetSigningKey:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ManagerGetCredentialsMetadataResult> AgentProxy::Manager_GetCredentialsMetadata(const PreliminaryRequestContext& aContext, const GetCredentialsMetadataArgs& aArgs) {
  RefPtr<ManagerGetCredentialsMetadataResult::Private> outPromise = new ManagerGetCredentialsMetadataResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Manager_GetCredentialsMetadata:onResolve()"));
    double out;
    if (NS_WARN_IF(!(FromJSVal<double>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Manager_GetCredentialsMetadata:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelCreateChannelResult> AgentProxy::Channel_CreateChannel(const PreliminaryRequestContext& aContext, const CreateChannelArgs& aArgs) {
  RefPtr<ChannelCreateChannelResult::Private> outPromise = new ChannelCreateChannelResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"createChannel"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_CreateChannel:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_CreateChannel:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelGenerateX25519KeyResult> AgentProxy::Channel_GenerateX25519Key(const RequestContext& aContext) {
  RefPtr<ChannelGenerateX25519KeyResult::Private> outPromise = new ChannelGenerateX25519KeyResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"generateX25519Key"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_GenerateX25519Key:onResolve()"));
    GenerateX25519KeyResult out;
    if (NS_WARN_IF(!(FromJSVal<GenerateX25519KeyResult>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_GenerateX25519Key:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelSignKeyExchangeParametersResult> AgentProxy::Channel_SignKeyExchangeParameters(const RequestContext& aContext, const SignKeyAgreementParametersArgs& aArgs) {
  RefPtr<ChannelSignKeyExchangeParametersResult::Private> outPromise = new ChannelSignKeyExchangeParametersResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"signKeyExchangeParameters"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_SignKeyExchangeParameters:onResolve()"));
    SignKeyAgreementParametersResult out;
    if (NS_WARN_IF(!(FromJSVal<SignKeyAgreementParametersResult>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_SignKeyExchangeParameters:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelVerifySignedKeyExchangeParametersResult> AgentProxy::Channel_VerifySignedKeyExchangeParameters(const RequestContext& aContext, const VerifySignedKeyExchangeParametersArgs& aArgs) {
  RefPtr<ChannelVerifySignedKeyExchangeParametersResult::Private> outPromise = new ChannelVerifySignedKeyExchangeParametersResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"verifySignedKeyExchangeParameters"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_VerifySignedKeyExchangeParameters:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_VerifySignedKeyExchangeParameters:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelEnableEndToEndEncryptionResult> AgentProxy::Channel_EnableEndToEndEncryption(const RequestContext& aContext) {
  RefPtr<ChannelEnableEndToEndEncryptionResult::Private> outPromise = new ChannelEnableEndToEndEncryptionResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"channel"_ns,
                                            u"enableEndToEndEncryption"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_EnableEndToEndEncryption:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_EnableEndToEndEncryption:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<ChannelCloseChannelResult> AgentProxy::Channel_CloseChannel(const RequestContext& aContext) {
  RefPtr<ChannelCloseChannelResult::Private> outPromise = new ChannelCloseChannelResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_CloseChannel:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Channel_CloseChannel:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<LoginApproveOperationResult> AgentProxy::Login_ApproveOperation(const RequestContext& aContext, const ApproveOperationArgs& aArgs) {
  RefPtr<LoginApproveOperationResult::Private> outPromise = new LoginApproveOperationResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_ApproveOperation:onResolve()"));
    ELoginUserIntent out;
    if (NS_WARN_IF(!(FromJSVal<ELoginUserIntent>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_ApproveOperation:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<LoginCloseOperationResult> AgentProxy::Login_CloseOperation(const RequestContextWithOperation& aContext) {
  RefPtr<LoginCloseOperationResult::Private> outPromise = new LoginCloseOperationResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"login"_ns,
                                            u"closeOperation"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_CloseOperation:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_CloseOperation:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<LoginGetRecordMetadataResult> AgentProxy::Login_GetRecordMetadata(const RequestContextWithOperation& aContext) {
  RefPtr<LoginGetRecordMetadataResult::Private> outPromise = new LoginGetRecordMetadataResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_GetRecordMetadata:onResolve()"));
    RecordMetadata out;
    if (NS_WARN_IF(!(FromJSVal<RecordMetadata>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_GetRecordMetadata:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<LoginUpdateMetadataResult> AgentProxy::Login_UpdateMetadata(const RequestContextWithOperation& aContext, const UpdateMetadataArgs& aArgs) {
  RefPtr<LoginUpdateMetadataResult::Private> outPromise = new LoginUpdateMetadataResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_UpdateMetadata:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("Login_UpdateMetadata:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountCreationApproveTransitionToAuthOpResult> AgentProxy::AccountCreation_ApproveTransitionToAuthOp(const RequestContextWithOperation& aContext, const ApproveTransitionToAuthOpArgs& aArgs) {
  RefPtr<AccountCreationApproveTransitionToAuthOpResult::Private> outPromise = new AccountCreationApproveTransitionToAuthOpResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_ApproveTransitionToAuthOp:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_ApproveTransitionToAuthOp:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountCreationGetUserAttributesResult> AgentProxy::AccountCreation_GetUserAttributes(const RequestContextWithLoginOperation& aContext) {
  RefPtr<AccountCreationGetUserAttributesResult::Private> outPromise = new AccountCreationGetUserAttributesResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_GetUserAttributes:onResolve()"));
    nsTArray<UserAttribute> out;
    if (NS_WARN_IF(!(FromJSVal<nsTArray<UserAttribute>>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_GetUserAttributes:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountCreationUpdateUserAttributesResult> AgentProxy::AccountCreation_UpdateUserAttributes(const RequestContextWithOperation& aContext, const UpdateUserAttributesArgs& aArgs) {
  RefPtr<AccountCreationUpdateUserAttributesResult::Private> outPromise = new AccountCreationUpdateUserAttributesResult::Private(__func__);
  dom::AutoEntryScript aes(mGlobal, "AgentProxy messaging interface");
  JSContext* cx = aes.cx();

  ErrorResult err;
  RefPtr<dom::Promise> prom = CallSendQuery(cx,
                                            u"accountCreation"_ns,
                                            u"updateUserAttributes"_ns,
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_UpdateUserAttributes:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_UpdateUserAttributes:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountCreationAddFieldResult> AgentProxy::AccountCreation_AddField(const RequestContextWithLoginOperation& aContext, const AddFieldArgs& aArgs) {
  RefPtr<AccountCreationAddFieldResult::Private> outPromise = new AccountCreationAddFieldResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_AddField:onResolve()"));
    SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> out;
    if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_AddField:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountCreationRejectFieldValueResult> AgentProxy::AccountCreation_RejectFieldValue(const RequestContextWithLoginOperation& aContext, const RejectFieldValueArgs& aArgs) {
  RefPtr<AccountCreationRejectFieldValueResult::Private> outPromise = new AccountCreationRejectFieldValueResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_RejectFieldValue:onResolve()"));
    SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> out;
    if (NS_WARN_IF(!(FromJSVal<SafeVariant<JSNull, nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountCreation_RejectFieldValue:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountAuthenticationApproveChallengeRequestResult> AgentProxy::AccountAuthentication_ApproveChallengeRequest(const RequestContextWithOperation& aContext, const ApproveChallengeRequestArgs& aArgs) {
  RefPtr<AccountAuthenticationApproveChallengeRequestResult::Private> outPromise = new AccountAuthenticationApproveChallengeRequestResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_ApproveChallengeRequest:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_ApproveChallengeRequest:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountAuthenticationAbortChallengeResult> AgentProxy::AccountAuthentication_AbortChallenge(const RequestContextWithOperation& aContext, const AbortChallengeArgs& aArgs) {
  RefPtr<AccountAuthenticationAbortChallengeResult::Private> outPromise = new AccountAuthenticationAbortChallengeResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_AbortChallenge:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_AbortChallenge:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountAuthenticationCloseChallengeResult> AgentProxy::AccountAuthentication_CloseChallenge(const RequestContextWithOperation& aContext, const CloseChallengeArgs& aArgs) {
  RefPtr<AccountAuthenticationCloseChallengeResult::Private> outPromise = new AccountAuthenticationCloseChallengeResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_CloseChallenge:onResolve()"));
    void* out = nullptr;
    outPromise->Resolve(out, __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_CloseChallenge:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}
RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AgentProxy::AccountAuthentication_RespondToChallengeMessage(const RequestContextWithLoginOperation& aContext, const SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage, BerytusSendGetOtpMessage>& aArgs) {
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult::Private> outPromise = new AccountAuthenticationRespondToChallengeMessageResult::Private(__func__);
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
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_RespondToChallengeMessage:onResolve()"));
    SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse> out;
    if (NS_WARN_IF(!(FromJSVal<SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse, BerytusChallengeGetOtpMessageResponse>>(aCx, aValue, out)))) {
      outPromise->Reject(Failure(), __func__);
    } else {
      outPromise->Resolve(std::move(out), __func__);
    }
    
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  auto onReject = [outPromise](JSContext* aCx, JS::Handle<JS::Value> aValue,
                     ErrorResult& aRv,
                     const nsCOMPtr<nsIGlobalObject>& aGlobal) {
    MOZ_LOG(sLogger, LogLevel::Debug, ("AccountAuthentication_RespondToChallengeMessage:onReject()"));
    Failure fr;
    FromJSVal(aCx, aValue, fr);
    outPromise->Reject(std::move(fr), __func__);
    return dom::Promise::CreateResolvedWithUndefined(aGlobal, aRv);
  };
  Result<RefPtr<dom::Promise>, nsresult> thenRes =
    prom->ThenCatchWithCycleCollectedArgs(std::move(onResolve), std::move(onReject), nsCOMPtr{mGlobal});
  if (NS_WARN_IF(thenRes.isErr())) {
    outPromise->Reject(Failure(), __func__);
  } else {
    MOZ_ASSERT(thenRes.unwrap());
    prom->AppendNativeHandler(new MozPromiseRejectWithBerytusFailureOnDestruction(outPromise, __func__));
  }
  return outPromise;
}

}  // namespace mozilla::berytus