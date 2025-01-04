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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool DocumentMetadata::ToJSVal(JSContext* aCx, const DocumentMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mId, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
bool PreliminaryRequestContext::ToJSVal(JSContext* aCx, const PreliminaryRequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ed25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mEd25519Key)))) {
    return false;
  }
  
  return true;
}
            
bool CryptoActor::ToJSVal(JSContext* aCx, const CryptoActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mEd25519Key, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "uri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mUri)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scheme", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mScheme)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hostname", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mHostname)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "port", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mPort)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "path", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mPath)))) {
    return false;
  }
  
  return true;
}
            
bool UriParams::ToJSVal(JSContext* aCx, const UriParams& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mUri, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "uri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mScheme, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scheme", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mHostname, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hostname", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mPort, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "port", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mPath, &memberVal4)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(UriParams::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(UriParams::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "originalUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(UriParams::FromJSVal(aCx, propVal, aRv.mOriginalUri)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "currentUri", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(UriParams::FromJSVal(aCx, propVal, aRv.mCurrentUri)))) {
    return false;
  }
  
  return true;
}
            
bool OriginActor::ToJSVal(JSContext* aCx, const OriginActor& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(UriParams::ToJSVal(aCx, aValue.mOriginalUri, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "originalUri", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(UriParams::ToJSVal(aCx, aValue.mCurrentUri, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "currentUri", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<CryptoActor, OriginActor>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(CryptoActor::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(OriginActor::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<CryptoActor, OriginActor>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<CryptoActor, OriginActor>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(CryptoActor::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      CryptoActor nv;
      if (NS_WARN_IF(!(CryptoActor::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(OriginActor::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      OriginActor nv;
      if (NS_WARN_IF(!(OriginActor::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<CryptoActor, OriginActor>::ToJSVal(JSContext* aCx, const SafeVariant<CryptoActor, OriginActor>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const CryptoActor& aVal) {
      return CryptoActor::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const OriginActor& aVal) {
      return OriginActor::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool GetSigningKeyArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::FromJSVal(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  
  return true;
}
            
bool GetSigningKeyArgs::ToJSVal(JSContext* aCx, const GetSigningKeyArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::ToJSVal(aCx, aValue.mWebAppActor, &memberVal0)))) {
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
    if (NS_WARN_IF(!(StringFromJSVal(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
bool nsTArray_nsString_ToJSVal(JSContext* aCx, const nsTArray<nsString>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const nsString& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(StringToJSVal(aCx, item, &value)))) {
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
  if (NS_WARN_IF(!(nsTArray_nsString_FromJSVal(aCx, aValue, *aRv)))) {
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
  if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, *aRv)))) {
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
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, aValue, *aRv)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mFieldValue)))) {
    return false;
  }
  
  return true;
}
            
bool PartialAccountIdentity::ToJSVal(JSContext* aCx, const PartialAccountIdentity& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mFieldValue, &memberVal1)))) {
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
    if (NS_WARN_IF(!(PartialAccountIdentity::FromJSVal(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
bool nsTArray_PartialAccountIdentity_ToJSVal(JSContext* aCx, const nsTArray<PartialAccountIdentity>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const PartialAccountIdentity& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(PartialAccountIdentity::ToJSVal(aCx, item, &value)))) {
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
  if (NS_WARN_IF(!(nsTArray_PartialAccountIdentity_FromJSVal(aCx, aValue, *aRv)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_double_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsTArray_PartialAccountIdentity__(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsString_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "schemaVersion", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_double_FromJSVal(aCx, propVal, aRv.mSchemaVersion)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identity", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsTArray_PartialAccountIdentity__FromJSVal(aCx, propVal, aRv.mIdentity)))) {
    return false;
  }
  
  return true;
}
            
bool AccountConstraints::ToJSVal(JSContext* aCx, const AccountConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsString_ToJSVal(aCx, aValue.mCategory, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_double_ToJSVal(aCx, aValue.mSchemaVersion, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "schemaVersion", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsTArray_PartialAccountIdentity__ToJSVal(aCx, aValue.mIdentity, &memberVal2)))) {
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
  if (NS_WARN_IF(!(AccountConstraints::FromJSVal(aCx, aValue, *aRv)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsTArray_nsString__(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsBool(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_AccountConstraints_(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "secretManagerPublicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsTArray_nsString__FromJSVal(aCx, propVal, aRv.mSecretManagerPublicKey)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "enableEndToEndEncryption", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BoolFromJSVal(aCx, propVal, aRv.mEnableEndToEndEncryption)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "account", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_AccountConstraints_FromJSVal(aCx, propVal, aRv.mAccount)))) {
    return false;
  }
  
  return true;
}
            
bool ChannelConstraints::ToJSVal(JSContext* aCx, const ChannelConstraints& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsTArray_nsString__ToJSVal(aCx, aValue.mSecretManagerPublicKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "secretManagerPublicKey", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BoolToJSVal(aCx, aValue.mEnableEndToEndEncryption, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "enableEndToEndEncryption", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(Maybe_AccountConstraints_ToJSVal(aCx, aValue.mAccount, &memberVal2)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelConstraints::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(AccountConstraints::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::FromJSVal(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelConstraints::FromJSVal(aCx, propVal, aRv.mChannelConstraints)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "accountConstraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(AccountConstraints::FromJSVal(aCx, propVal, aRv.mAccountConstraints)))) {
    return false;
  }
  
  return true;
}
            
bool GetCredentialsMetadataArgs::ToJSVal(JSContext* aCx, const GetCredentialsMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::ToJSVal(aCx, aValue.mWebAppActor, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ChannelConstraints::ToJSVal(aCx, aValue.mChannelConstraints, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelConstraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(AccountConstraints::ToJSVal(aCx, aValue.mAccountConstraints, &memberVal2)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelConstraints::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(CryptoActor::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "constraints", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelConstraints::FromJSVal(aCx, propVal, aRv.mConstraints)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::FromJSVal(aCx, propVal, aRv.mWebAppActor)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmActor", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(CryptoActor::FromJSVal(aCx, propVal, aRv.mScmActor)))) {
    return false;
  }
  
  return true;
}
            
bool ChannelMetadata::ToJSVal(JSContext* aCx, const ChannelMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ChannelConstraints::ToJSVal(aCx, aValue.mConstraints, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "constraints", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mWebAppActor.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<CryptoActor, OriginActor>::ToJSVal(aCx, aValue.mWebAppActor, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppActor", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(CryptoActor::ToJSVal(aCx, aValue.mScmActor, &memberVal3)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::FromJSVal(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
bool RequestContext::ToJSVal(JSContext* aCx, const RequestContext& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ChannelMetadata::ToJSVal(aCx, aValue.mChannel, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal1)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mChannelId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mWebAppX25519Key)))) {
    return false;
  }
  
  return true;
}
            
bool InitialKeyExchangeParametersDraft::ToJSVal(JSContext* aCx, const InitialKeyExchangeParametersDraft& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mChannelId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mWebAppX25519Key, &memberVal1)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "paramsDraft", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(InitialKeyExchangeParametersDraft::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "paramsDraft", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(InitialKeyExchangeParametersDraft::FromJSVal(aCx, propVal, aRv.mParamsDraft)))) {
    return false;
  }
  
  return true;
}
            
bool GenerateKeyExchangeParametersArgs::ToJSVal(JSContext* aCx, const GenerateKeyExchangeParametersArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(InitialKeyExchangeParametersDraft::ToJSVal(aCx, aValue.mParamsDraft, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mScmX25519Key)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mHkdfHash)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfSalt)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfInfo)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mAesKeyLength)))) {
    return false;
  }
  
  return true;
}
            
bool PartialKeyExchangeParametersFromScm::ToJSVal(JSContext* aCx, const PartialKeyExchangeParametersFromScm& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mScmX25519Key, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmX25519Key", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mHkdfHash, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfHash", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mHkdfSalt, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfSalt", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mHkdfInfo, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfInfo", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mAesKeyLength, &memberVal4)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "packet", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "packet", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mPacket)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channelId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mChannelId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mWebAppX25519Key)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "scmX25519Key", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mScmX25519Key)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfHash", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mHkdfHash)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfSalt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfSalt)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "hkdfInfo", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mHkdfInfo)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "aesKeyLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mAesKeyLength)))) {
    return false;
  }
  
  return true;
}
            
bool KeyExchangeParameters::ToJSVal(JSContext* aCx, const KeyExchangeParameters& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mPacket, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "packet", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mChannelId, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channelId", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mWebAppX25519Key, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "webAppX25519Key", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mScmX25519Key, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "scmX25519Key", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mHkdfHash, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfHash", memberVal4))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal5(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mHkdfSalt, &memberVal5)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfSalt", memberVal5))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal6(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mHkdfInfo, &memberVal6)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "hkdfInfo", memberVal6))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal7(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mAesKeyLength, &memberVal7)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "params", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(KeyExchangeParameters::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppPacketSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "params", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(KeyExchangeParameters::FromJSVal(aCx, propVal, aRv.mParams)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "webAppPacketSignature", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mWebAppPacketSignature)))) {
    return false;
  }
  
  return true;
}
            
bool EnableEndToEndEncryptionArgs::ToJSVal(JSContext* aCx, const EnableEndToEndEncryptionArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(KeyExchangeParameters::ToJSVal(aCx, aValue.mParams, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "params", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mWebAppPacketSignature, &memberVal1)))) {
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
bool RequestedUserAttribute::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsBool(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "required", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BoolFromJSVal(aCx, propVal, aRv.mRequired)))) {
    return false;
  }
  
  return true;
}
            
bool RequestedUserAttribute::ToJSVal(JSContext* aCx, const RequestedUserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BoolToJSVal(aCx, aValue.mRequired, &memberVal1)))) {
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
    if (NS_WARN_IF(!(RequestedUserAttribute::FromJSVal(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
bool nsTArray_RequestedUserAttribute_ToJSVal(JSContext* aCx, const nsTArray<RequestedUserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const RequestedUserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(RequestedUserAttribute::ToJSVal(aCx, item, &value)))) {
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
bool EBerytusFieldType::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  EBerytusFieldType e;
  aRv = EBerytusFieldType::FromString(strVal, e);
  return true;
}
bool EBerytusFieldType::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, EBerytusFieldType& aRv) {
  if (NS_WARN_IF(!aValue.isString())) {
    return false;
  }
  nsString strVal;
  if (NS_WARN_IF(!StringFromJSVal(aCx, aValue, strVal))) {
    return false;
  }
  if (NS_WARN_IF(!EBerytusFieldType::EBerytusFieldType::FromString(strVal, aRv))) {
    return false;
  }
  return true;
}
bool EBerytusFieldType::ToJSVal(JSContext* aCx, const EBerytusFieldType& aValue, JS::MutableHandle<JS::Value> aRv) {
  nsString strVal;
  aValue.ToString(strVal);
  if (NS_WARN_IF(!StringToJSVal(aCx, strVal, aRv))) {
    return false;
  }
  return true;
}
bool FieldInfo::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EBerytusFieldType::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool FieldInfo::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, FieldInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EBerytusFieldType::FromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  
  return true;
}
            
bool FieldInfo::ToJSVal(JSContext* aCx, const FieldInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(EBerytusFieldType::ToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsnsTArray_FieldInfo_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  if (NS_WARN_IF(!JS::IsArrayObject(aCx, aValue, &aRv))) {
    return false;
  }
  return true;
}
bool nsTArray_FieldInfo_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, nsTArray<FieldInfo>& aRv) {
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

    FieldInfo item;
    if (NS_WARN_IF(!(FieldInfo::FromJSVal(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
bool nsTArray_FieldInfo_ToJSVal(JSContext* aCx, const nsTArray<FieldInfo>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const FieldInfo& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(FieldInfo::ToJSVal(aCx, item, &value)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ELoginUserIntent::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsnsTArray_RequestedUserAttribute_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fields", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsnsTArray_FieldInfo_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationType::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationStatus::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationState::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "intent", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ELoginUserIntent::FromJSVal(aCx, propVal, aRv.mIntent)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "requestedUserAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(nsTArray_RequestedUserAttribute_FromJSVal(aCx, propVal, aRv.mRequestedUserAttributes)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fields", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(nsTArray_FieldInfo_FromJSVal(aCx, propVal, aRv.mFields)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationType::FromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationStatus::FromJSVal(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationState::FromJSVal(aCx, propVal, aRv.mState)))) {
    return false;
  }
  
  return true;
}
            
bool LoginOperationMetadata::ToJSVal(JSContext* aCx, const LoginOperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ELoginUserIntent::ToJSVal(aCx, aValue.mIntent, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "intent", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(nsTArray_RequestedUserAttribute_ToJSVal(aCx, aValue.mRequestedUserAttributes, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "requestedUserAttributes", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(nsTArray_FieldInfo_ToJSVal(aCx, aValue.mFields, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fields", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(EOperationType::ToJSVal(aCx, aValue.mType, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal4))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal5(aCx);
  
  if (NS_WARN_IF(!(EOperationStatus::ToJSVal(aCx, aValue.mStatus, &memberVal5)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal5))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal6(aCx);
  
  if (NS_WARN_IF(!(OperationState::ToJSVal(aCx, aValue.mState, &memberVal6)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "state", memberVal6))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(LoginOperationMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(LoginOperationMetadata::FromJSVal(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  
  return true;
}
            
bool ApproveOperationArgs::ToJSVal(JSContext* aCx, const ApproveOperationArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(LoginOperationMetadata::ToJSVal(aCx, aValue.mOperation, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationType::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationStatus::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationState::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationType::FromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EOperationStatus::FromJSVal(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "state", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationState::FromJSVal(aCx, propVal, aRv.mState)))) {
    return false;
  }
  
  return true;
}
            
bool OperationMetadata::ToJSVal(JSContext* aCx, const OperationMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(EOperationType::ToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(EOperationStatus::ToJSVal(aCx, aValue.mStatus, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(OperationState::ToJSVal(aCx, aValue.mState, &memberVal3)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(OperationMetadata::FromJSVal(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::FromJSVal(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
bool RequestContextWithOperation::ToJSVal(JSContext* aCx, const RequestContextWithOperation& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(OperationMetadata::ToJSVal(aCx, aValue.mOperation, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ChannelMetadata::ToJSVal(aCx, aValue.mChannel, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal2)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EMetadataStatus::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "version", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mVersion)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EMetadataStatus::FromJSVal(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "changePassUrl", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mChangePassUrl)))) {
    return false;
  }
  
  return true;
}
            
bool RecordMetadata::ToJSVal(JSContext* aCx, const RecordMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mVersion, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "version", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(EMetadataStatus::ToJSVal(aCx, aValue.mStatus, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "status", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mCategory, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mChangePassUrl, &memberVal3)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(RecordMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "metadata", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(RecordMetadata::FromJSVal(aCx, propVal, aRv.mMetadata)))) {
    return false;
  }
  
  return true;
}
            
bool UpdateMetadataArgs::ToJSVal(JSContext* aCx, const UpdateMetadataArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(RecordMetadata::ToJSVal(aCx, aValue.mMetadata, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(LoginOperationMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "newAuthOp", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(LoginOperationMetadata::FromJSVal(aCx, propVal, aRv.mNewAuthOp)))) {
    return false;
  }
  
  return true;
}
            
bool ApproveTransitionToAuthOpArgs::ToJSVal(JSContext* aCx, const ApproveTransitionToAuthOpArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(LoginOperationMetadata::ToJSVal(aCx, aValue.mNewAuthOp, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "newAuthOp", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool JSValIsArrayBufferView(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  aRv = JS_IsArrayBufferViewObject(obj);
  return true;
}
bool ArrayBufferViewFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, ArrayBufferView& aRv) {
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
bool ArrayBufferViewToJSVal(JSContext* aCx, const ArrayBufferView& aValue, JS::MutableHandle<JS::Value> aRv) {
  MOZ_ASSERT(aValue.Obj()); // TODO(berytus): Remove or keep this.
  aRv.setObject(*aValue.Obj());
  return true;
}
bool SafeVariant<ArrayBuffer, ArrayBufferView>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
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
bool SafeVariant<ArrayBuffer, ArrayBufferView>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, (aRv.mVariant)->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(ArrayBufferViewFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<ArrayBuffer, ArrayBufferView>::ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ArrayBufferToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ArrayBufferViewToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
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
bool SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNothing(aCx, aValue, isValid)))) {
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
bool SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, (aRv.mVariant)->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(ArrayBufferViewFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNothing(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      Nothing nv;
      if (NS_WARN_IF(!(NothingFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ArrayBufferToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ArrayBufferViewToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const Nothing& aVal) {
      return NothingToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool AesGcmParams::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "iv", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "additionalData", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "tagLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_double_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool AesGcmParams::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, AesGcmParams& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "iv", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::FromJSVal(aCx, propVal, aRv.mIv)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "additionalData", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::FromJSVal(aCx, propVal, aRv.mAdditionalData)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "tagLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_double_FromJSVal(aCx, propVal, aRv.mTagLength)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  
  return true;
}
            
bool AesGcmParams::ToJSVal(JSContext* aCx, const AesGcmParams& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mIv.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::ToJSVal(aCx, aValue.mIv, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "iv", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  if (NS_WARN_IF(!aValue.mAdditionalData.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView, Nothing>::ToJSVal(aCx, aValue.mAdditionalData, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "additionalData", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(Maybe_double_ToJSVal(aCx, aValue.mTagLength, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "tagLength", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusEncryptedPacket::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  bool isValid = false;
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(AesGcmParams::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ciphertext", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusEncryptedPacket::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusEncryptedPacket& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(AesGcmParams::FromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "ciphertext", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mCiphertext)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusEncryptedPacket::ToJSVal(JSContext* aCx, const BerytusEncryptedPacket& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(AesGcmParams::ToJSVal(aCx, aValue.mParameters, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mCiphertext, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "ciphertext", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<1>(), ArrayBuffer());
      if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, (aRv.mVariant)->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBufferView(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      ArrayBufferView nv;
      if (NS_WARN_IF(!(ArrayBufferViewFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::ToJSVal(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBuffer& aVal) {
      return ArrayBufferToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBufferView& aVal) {
      return ArrayBufferViewToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool UserAttribute::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "info", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "info", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsString_FromJSVal(aCx, propVal, aRv.mInfo)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "mimeType", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsString_FromJSVal(aCx, propVal, aRv.mMimeType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  
  return true;
}
            
bool UserAttribute::ToJSVal(JSContext* aCx, const UserAttribute& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsString_ToJSVal(aCx, aValue.mInfo, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "info", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsString_ToJSVal(aCx, aValue.mMimeType, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "mimeType", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mValue, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal3))) {
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
    if (NS_WARN_IF(!(UserAttribute::FromJSVal(aCx, value, item)))) {
      return false;
    }
    aRv.AppendElement(std::move(item));
  }
  return true;
}
bool nsTArray_UserAttribute_ToJSVal(JSContext* aCx, const nsTArray<UserAttribute>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> array(aCx, JS::NewArrayObject(aCx, 0));

  for (uint32_t i = 0; i < aValue.Length(); i++) {
    const UserAttribute& item = aValue.ElementAt(i);

    JS::Rooted<JS::Value> value(aCx);
    if (NS_WARN_IF(!(UserAttribute::ToJSVal(aCx, item, &value)))) {
      return false;
    }
    if (NS_WARN_IF(!JS_DefineElement(aCx, array, i, value, JSPROP_ENUMERATE))) {
      return false;
    }
  }
  aRv.setObject(*array);
  return true;
}

bool UpdateUserAttributesArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsnsTArray_UserAttribute_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool UpdateUserAttributesArgs::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, UpdateUserAttributesArgs& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "userAttributes", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(nsTArray_UserAttribute_FromJSVal(aCx, propVal, aRv.mUserAttributes)))) {
    return false;
  }
  
  return true;
}
            
bool UpdateUserAttributesArgs::ToJSVal(JSContext* aCx, const UpdateUserAttributesArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(nsTArray_UserAttribute_ToJSVal(aCx, aValue.mUserAttributes, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "userAttributes", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool RequestContextWithLoginOperation::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(LoginOperationMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool RequestContextWithLoginOperation::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, RequestContextWithLoginOperation& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "operation", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(LoginOperationMetadata::FromJSVal(aCx, propVal, aRv.mOperation)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "channel", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChannelMetadata::FromJSVal(aCx, propVal, aRv.mChannel)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "document", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(DocumentMetadata::FromJSVal(aCx, propVal, aRv.mDocument)))) {
    return false;
  }
  
  return true;
}
            
bool RequestContextWithLoginOperation::ToJSVal(JSContext* aCx, const RequestContextWithLoginOperation& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(LoginOperationMetadata::ToJSVal(aCx, aValue.mOperation, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "operation", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ChannelMetadata::ToJSVal(aCx, aValue.mChannel, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "channel", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(DocumentMetadata::ToJSVal(aCx, aValue.mDocument, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "document", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusFieldCategoryOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "position", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_double_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusFieldCategoryOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusFieldCategoryOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "categoryId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mCategoryId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "position", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_double_FromJSVal(aCx, propVal, aRv.mPosition)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusFieldCategoryOptions::ToJSVal(JSContext* aCx, const BerytusFieldCategoryOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mCategoryId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "categoryId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_double_ToJSVal(aCx, aValue.mPosition, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "position", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsMaybe_BerytusFieldCategoryOptions_(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (aValue.isUndefined()) {
    aRv = true;
    return true;
  }
  return BerytusFieldCategoryOptions::IsJSValueValid(aCx, aValue, aRv);
}
bool Maybe_BerytusFieldCategoryOptions_FromJSVal(JSContext* aCx, const JS::Handle<JS::Value> aValue, Maybe<BerytusFieldCategoryOptions>& aRv) {
  if (aValue.isUndefined()) {
    aRv.reset();
    return true;
  }
  aRv.emplace();
  if (NS_WARN_IF(!(BerytusFieldCategoryOptions::FromJSVal(aCx, aValue, *aRv)))) {
    return false;
  }
  return true;
}
bool Maybe_BerytusFieldCategoryOptions_ToJSVal(JSContext* aCx, const Maybe<BerytusFieldCategoryOptions>& aValue, JS::MutableHandle<JS::Value> aRv) {
  if (!aValue) {
    aRv.setUndefined();
    return true;
  }

  return BerytusFieldCategoryOptions::ToJSVal(aCx, aValue.ref(), aRv);
}
bool BerytusForeignIdentityFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsBool(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "kind", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusForeignIdentityFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BoolFromJSVal(aCx, propVal, aRv.mPrivate)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "kind", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mKind)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusForeignIdentityFieldOptions::ToJSVal(JSContext* aCx, const BerytusForeignIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BoolToJSVal(aCx, aValue.mPrivate, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "private", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mKind, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "kind", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool JSValIsNull(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  aRv = aValue.isNull();
  return true;
}
bool NullFromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, JSNull& aRv) {
  if (NS_WARN_IF(!aValue.isNull())) {
    return false;
  }
  // Nothing to do...
  return true;
}
bool NullToJSVal(JSContext* aCx, const JSNull& aValue, JS::MutableHandle<JS::Value> aRv) {
  aRv.setNull();
  return true;
}
bool SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(NullFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::ToJSVal(JSContext* aCx, const SafeVariant<JSNull, nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return NullToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusForeignIdentityField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusForeignIdentityFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusForeignIdentityField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusForeignIdentityField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusForeignIdentityFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusForeignIdentityField::ToJSVal(JSContext* aCx, const BerytusForeignIdentityField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusForeignIdentityFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusIdentityFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsBool(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsBool(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "maxLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "allowedCharacters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusIdentityFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "humanReadable", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BoolFromJSVal(aCx, propVal, aRv.mHumanReadable)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "private", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BoolFromJSVal(aCx, propVal, aRv.mPrivate)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "maxLength", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mMaxLength)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "allowedCharacters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsString_FromJSVal(aCx, propVal, aRv.mAllowedCharacters)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusIdentityFieldOptions::ToJSVal(JSContext* aCx, const BerytusIdentityFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BoolToJSVal(aCx, aValue.mHumanReadable, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "humanReadable", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BoolToJSVal(aCx, aValue.mPrivate, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "private", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mMaxLength, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "maxLength", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsString_ToJSVal(aCx, aValue.mAllowedCharacters, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "allowedCharacters", memberVal3))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal4(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal4)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal4))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusIdentityField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusIdentityFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusIdentityField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentityField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusIdentityFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusIdentityField::ToJSVal(JSContext* aCx, const BerytusIdentityField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusIdentityFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusKeyFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusKeyFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mAlg)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusKeyFieldOptions::ToJSVal(JSContext* aCx, const BerytusKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mAlg, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "alg", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<0>(), ArrayBuffer());
      if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, (aRv.mVariant)->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(JSContext* aCx, const SafeVariant<ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const ArrayBuffer& aVal) {
      return ArrayBufferToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusKeyFieldValue::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusKeyFieldValue::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "publicKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mPublicKey)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusKeyFieldValue::ToJSVal(JSContext* aCx, const BerytusKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mPublicKey.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mPublicKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "publicKey", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<JSNull, BerytusKeyFieldValue>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<JSNull, BerytusKeyFieldValue>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(NullFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<JSNull, BerytusKeyFieldValue>::ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return NullToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return BerytusKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusKeyField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusKeyFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusKeyFieldValue>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusKeyField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusKeyField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusKeyFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusKeyFieldValue>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusKeyField::ToJSVal(JSContext* aCx, const BerytusKeyField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusKeyFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusKeyFieldValue>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusPasswordFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsMaybe_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusPasswordFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "passwordRules", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_nsString_FromJSVal(aCx, propVal, aRv.mPasswordRules)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusPasswordFieldOptions::ToJSVal(JSContext* aCx, const BerytusPasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(Maybe_nsString_ToJSVal(aCx, aValue.mPasswordRules, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "passwordRules", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusPasswordField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusPasswordFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusPasswordField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusPasswordFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusPasswordField::ToJSVal(JSContext* aCx, const BerytusPasswordField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusPasswordFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, nsString, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSecurePasswordFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSecurePasswordFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "identityFieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mIdentityFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSecurePasswordFieldOptions::ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mIdentityFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "identityFieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSecurePasswordFieldValue::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "verifier", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSecurePasswordFieldValue::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "salt", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mSalt)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "verifier", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mVerifier)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSecurePasswordFieldValue::ToJSVal(JSContext* aCx, const BerytusSecurePasswordFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mSalt.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mSalt, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "salt", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  if (NS_WARN_IF(!aValue.mVerifier.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mVerifier, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "verifier", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(NullFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusSecurePasswordFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return NullToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return BerytusSecurePasswordFieldValue::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusSecurePasswordField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecurePasswordFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSecurePasswordField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecurePasswordField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecurePasswordFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSecurePasswordField::ToJSVal(JSContext* aCx, const BerytusSecurePasswordField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusSecurePasswordFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSecurePasswordFieldValue>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSharedKeyFieldOptions::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsNumber(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsMaybe_BerytusFieldCategoryOptions_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSharedKeyFieldOptions::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldOptions& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "alg", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NumberFromJSVal(aCx, propVal, aRv.mAlg)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "category", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_FromJSVal(aCx, propVal, aRv.mCategory)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSharedKeyFieldOptions::ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldOptions& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(NumberToJSVal(aCx, aValue.mAlg, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "alg", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(Maybe_BerytusFieldCategoryOptions_ToJSVal(aCx, aValue.mCategory, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "category", memberVal1))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSharedKeyFieldValue::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSharedKeyFieldValue::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyFieldValue& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "privateKey", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mPrivateKey)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSharedKeyFieldValue::ToJSVal(JSContext* aCx, const BerytusSharedKeyFieldValue& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mPrivateKey.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mPrivateKey, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "privateKey", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<JSNull, BerytusSharedKeyFieldValue>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<JSNull, BerytusSharedKeyFieldValue>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNull(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      JSNull nv;
      if (NS_WARN_IF(!(NullFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<JSNull, BerytusSharedKeyFieldValue>::ToJSVal(JSContext* aCx, const SafeVariant<JSNull, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const JSNull& aVal) {
      return NullToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return BerytusSharedKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusSharedKeyField::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSharedKeyFieldOptions::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSharedKeyFieldValue>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSharedKeyField::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSharedKeyField& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "options", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSharedKeyFieldOptions::FromJSVal(aCx, propVal, aRv.mOptions)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "value", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSharedKeyFieldValue>::FromJSVal(aCx, propVal, aRv.mValue)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSharedKeyField::ToJSVal(JSContext* aCx, const BerytusSharedKeyField& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(BerytusSharedKeyFieldOptions::ToJSVal(aCx, aValue.mOptions, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "options", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<JSNull, BerytusSharedKeyFieldValue>::ToJSVal(aCx, aValue.mValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "value", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal3)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal3))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusForeignIdentityField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusIdentityField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusPasswordField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyField::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusForeignIdentityField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusForeignIdentityField nv;
      if (NS_WARN_IF(!(BerytusForeignIdentityField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusIdentityField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusIdentityField nv;
      if (NS_WARN_IF(!(BerytusIdentityField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyField nv;
      if (NS_WARN_IF(!(BerytusKeyField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusPasswordField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusPasswordField nv;
      if (NS_WARN_IF(!(BerytusPasswordField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordField nv;
      if (NS_WARN_IF(!(BerytusSecurePasswordField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyField::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyField nv;
      if (NS_WARN_IF(!(BerytusSharedKeyField::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::ToJSVal(JSContext* aCx, const SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusForeignIdentityField& aVal) {
      return BerytusForeignIdentityField::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusIdentityField& aVal) {
      return BerytusIdentityField::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyField& aVal) {
      return BerytusKeyField::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusPasswordField& aVal) {
      return BerytusPasswordField::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordField& aVal) {
      return BerytusSecurePasswordField::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyField& aVal) {
      return BerytusSharedKeyField::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool AddFieldArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "field", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::FromJSVal(aCx, propVal, aRv.mField)))) {
    return false;
  }
  
  return true;
}
            
bool AddFieldArgs::ToJSVal(JSContext* aCx, const AddFieldArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mField.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<BerytusForeignIdentityField, BerytusIdentityField, BerytusKeyField, BerytusPasswordField, BerytusSecurePasswordField, BerytusSharedKeyField>::ToJSVal(aCx, aValue.mField, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "field", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>::ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return BerytusKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return BerytusSecurePasswordFieldValue::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return BerytusSharedKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}

bool FieldValueRejectionReason::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mCode)))) {
    return false;
  }
  
  return true;
}
            
bool FieldValueRejectionReason::ToJSVal(JSContext* aCx, const FieldValueRejectionReason& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mCode, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "code", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNothing(aCx, aValue, isValid)))) {
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
bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSecurePasswordFieldValue nv;
      if (NS_WARN_IF(!(BerytusSecurePasswordFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSharedKeyFieldValue nv;
      if (NS_WARN_IF(!(BerytusSharedKeyFieldValue::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsNothing(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      Nothing nv;
      if (NS_WARN_IF(!(NothingFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusKeyFieldValue& aVal) {
      return BerytusKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSecurePasswordFieldValue& aVal) {
      return BerytusSecurePasswordFieldValue::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSharedKeyFieldValue& aVal) {
      return BerytusSharedKeyFieldValue::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const Nothing& aVal) {
      return NothingToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool RejectFieldValueArgs::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FieldValueRejectionReason::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "fieldId", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mFieldId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(FieldValueRejectionReason::FromJSVal(aCx, propVal, aRv.mReason)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "optionalNewValue", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::FromJSVal(aCx, propVal, aRv.mOptionalNewValue)))) {
    return false;
  }
  
  return true;
}
            
bool RejectFieldValueArgs::ToJSVal(JSContext* aCx, const RejectFieldValueArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mFieldId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "fieldId", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(FieldValueRejectionReason::ToJSVal(aCx, aValue.mReason, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "reason", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mOptionalNewValue.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue, Nothing>::ToJSVal(aCx, aValue.mOptionalNewValue, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "optionalNewValue", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
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
bool EChallengeType::IsIdentification() const {
  return mVal == uint8_t(0);
}
bool EChallengeType::IsDigitalSignature() const {
  return mVal == uint8_t(1);
}
bool EChallengeType::IsPassword() const {
  return mVal == uint8_t(2);
}
bool EChallengeType::IsSecureRemotePassword() const {
  return mVal == uint8_t(3);
}
bool EChallengeType::IsForeignIdentityOtp() const {
  return mVal == uint8_t(4);
}
void EChallengeType::SetAsIdentification() {
  mVal = uint8_t(0);
}
void EChallengeType::SetAsDigitalSignature() {
  mVal = uint8_t(1);
}
void EChallengeType::SetAsPassword() {
  mVal = uint8_t(2);
}
void EChallengeType::SetAsSecureRemotePassword() {
  mVal = uint8_t(3);
}
void EChallengeType::SetAsForeignIdentityOtp() {
  mVal = uint8_t(4);
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
bool EChallengeStatus::IsInvalid() const {
  return mVal == uint8_t(0);
}
bool EChallengeStatus::IsPending() const {
  return mVal == uint8_t(1);
}
bool EChallengeStatus::IsActive() const {
  return mVal == uint8_t(2);
}
bool EChallengeStatus::IsAborted() const {
  return mVal == uint8_t(3);
}
bool EChallengeStatus::IsSealed() const {
  return mVal == uint8_t(4);
}
void EChallengeStatus::SetAsInvalid() {
  mVal = uint8_t(0);
}
void EChallengeStatus::SetAsPending() {
  mVal = uint8_t(1);
}
void EChallengeStatus::SetAsActive() {
  mVal = uint8_t(2);
}
void EChallengeStatus::SetAsAborted() {
  mVal = uint8_t(3);
}
void EChallengeStatus::SetAsSealed() {
  mVal = uint8_t(4);
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EChallengeType::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeParameters::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EChallengeStatus::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EChallengeType::FromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeParameters::FromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "status", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(EChallengeStatus::FromJSVal(aCx, propVal, aRv.mStatus)))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeMetadata::ToJSVal(JSContext* aCx, const ChallengeMetadata& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(EChallengeType::ToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(ChallengeParameters::ToJSVal(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal3(aCx);
  
  if (NS_WARN_IF(!(EChallengeStatus::ToJSVal(aCx, aValue.mStatus, &memberVal3)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  
  return true;
}
            
bool ApproveChallengeRequestArgs::ToJSVal(JSContext* aCx, const ApproveChallengeRequestArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "code", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mCode)))) {
    return false;
  }
  
  return true;
}
            
bool ChallengeAbortionReason::ToJSVal(JSContext* aCx, const ChallengeAbortionReason& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mCode, &memberVal0)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeAbortionReason::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "reason", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeAbortionReason::FromJSVal(aCx, propVal, aRv.mReason)))) {
    return false;
  }
  
  return true;
}
            
bool AbortChallengeArgs::ToJSVal(JSContext* aCx, const AbortChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(ChallengeAbortionReason::ToJSVal(aCx, aValue.mReason, &memberVal1)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::IsJSValueValid(aCx, propVal, isValid)))) {
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
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ChallengeMetadata::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  
  return true;
}
            
bool CloseChallengeArgs::ToJSVal(JSContext* aCx, const CloseChallengeArgs& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ChallengeMetadata::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}


bool BerytusIdentificationChallengeInfo::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNull(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusIdentificationChallengeInfo::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusIdentificationChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NullFromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusIdentificationChallengeInfo::ToJSVal(JSContext* aCx, const BerytusIdentificationChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(NullToJSVal(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendGetIdentityFieldsMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusIdentificationChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsnsTArray_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendGetIdentityFieldsMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetIdentityFieldsMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusIdentificationChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(nsTArray_nsString_FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendGetIdentityFieldsMessage::ToJSVal(JSContext* aCx, const BerytusSendGetIdentityFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusIdentificationChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(nsTArray_nsString_ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusPasswordChallengeInfo::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNull(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusPasswordChallengeInfo::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusPasswordChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NullFromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusPasswordChallengeInfo::ToJSVal(JSContext* aCx, const BerytusPasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(NullToJSVal(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendGetPasswordFieldsMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusPasswordChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsnsTArray_nsString_(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendGetPasswordFieldsMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendGetPasswordFieldsMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusPasswordChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(nsTArray_nsString_FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendGetPasswordFieldsMessage::ToJSVal(JSContext* aCx, const BerytusSendGetPasswordFieldsMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusPasswordChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(nsTArray_nsString_ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusDigitalSignatureChallengeInfo::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsNull(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusDigitalSignatureChallengeInfo::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusDigitalSignatureChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NullFromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusDigitalSignatureChallengeInfo::ToJSVal(JSContext* aCx, const BerytusDigitalSignatureChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(NullToJSVal(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendSelectKeyMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendSelectKeyMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectKeyMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendSelectKeyMessage::ToJSVal(JSContext* aCx, const BerytusSendSelectKeyMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendSignNonceMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendSignNonceMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSignNonceMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendSignNonceMessage::ToJSVal(JSContext* aCx, const BerytusSendSignNonceMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusDigitalSignatureChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<ArrayBuffer, ArrayBufferView>::ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSecureRemotePasswordChallengeParameters::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSecureRemotePasswordChallengeParameters::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeParameters& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "encoding", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mEncoding)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSecureRemotePasswordChallengeParameters::ToJSVal(JSContext* aCx, const BerytusSecureRemotePasswordChallengeParameters& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mEncoding, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "encoding", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSecureRemotePasswordChallengeInfo::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeParameters::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSecureRemotePasswordChallengeInfo::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSecureRemotePasswordChallengeInfo& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "id", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mId)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "type", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mType)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "parameters", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeParameters::FromJSVal(aCx, propVal, aRv.mParameters)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSecureRemotePasswordChallengeInfo::ToJSVal(JSContext* aCx, const BerytusSecureRemotePasswordChallengeInfo& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mId, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "id", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mType, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "type", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeParameters::ToJSVal(aCx, aValue.mParameters, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "parameters", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendSelectSecurePasswordMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendSelectSecurePasswordMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendSelectSecurePasswordMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendSelectSecurePasswordMessage::ToJSVal(JSContext* aCx, const BerytusSendSelectSecurePasswordMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendExchangePublicKeysMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendExchangePublicKeysMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendExchangePublicKeysMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendExchangePublicKeysMessage::ToJSVal(JSContext* aCx, const BerytusSendExchangePublicKeysMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendComputeClientProofMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendComputeClientProofMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendComputeClientProofMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendComputeClientProofMessage::ToJSVal(JSContext* aCx, const BerytusSendComputeClientProofMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusSendVerifyServerProofMessage::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(JSValIsString(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusSendVerifyServerProofMessage::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusSendVerifyServerProofMessage& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "challenge", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::FromJSVal(aCx, propVal, aRv.mChallenge)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "name", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(StringFromJSVal(aCx, propVal, aRv.mName)))) {
    return false;
  }
  

  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "payload", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mPayload)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusSendVerifyServerProofMessage::ToJSVal(JSContext* aCx, const BerytusSendVerifyServerProofMessage& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusSecureRemotePasswordChallengeInfo::ToJSVal(aCx, aValue.mChallenge, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "challenge", memberVal0))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal1(aCx);
  
  if (NS_WARN_IF(!(StringToJSVal(aCx, aValue.mName, &memberVal1)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "name", memberVal1))) {
    return false;
  }
  

  JS::Rooted<JS::Value> memberVal2(aCx);
  if (NS_WARN_IF(!aValue.mPayload.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, ArrayBufferView, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mPayload, &memberVal2)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "payload", memberVal2))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendGetIdentityFieldsMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendGetPasswordFieldsMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSelectKeyMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSignNonceMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSelectSecurePasswordMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendExchangePublicKeysMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendComputeClientProofMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendVerifyServerProofMessage::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendGetIdentityFieldsMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendGetIdentityFieldsMessage nv;
      if (NS_WARN_IF(!(BerytusSendGetIdentityFieldsMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendGetPasswordFieldsMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendGetPasswordFieldsMessage nv;
      if (NS_WARN_IF(!(BerytusSendGetPasswordFieldsMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSelectKeyMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSelectKeyMessage nv;
      if (NS_WARN_IF(!(BerytusSendSelectKeyMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSignNonceMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSignNonceMessage nv;
      if (NS_WARN_IF(!(BerytusSendSignNonceMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendSelectSecurePasswordMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendSelectSecurePasswordMessage nv;
      if (NS_WARN_IF(!(BerytusSendSelectSecurePasswordMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendExchangePublicKeysMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendExchangePublicKeysMessage nv;
      if (NS_WARN_IF(!(BerytusSendExchangePublicKeysMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendComputeClientProofMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendComputeClientProofMessage nv;
      if (NS_WARN_IF(!(BerytusSendComputeClientProofMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusSendVerifyServerProofMessage::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusSendVerifyServerProofMessage nv;
      if (NS_WARN_IF(!(BerytusSendVerifyServerProofMessage::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>::ToJSVal(JSContext* aCx, const SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusSendGetIdentityFieldsMessage& aVal) {
      return BerytusSendGetIdentityFieldsMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendGetPasswordFieldsMessage& aVal) {
      return BerytusSendGetPasswordFieldsMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSelectKeyMessage& aVal) {
      return BerytusSendSelectKeyMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSignNonceMessage& aVal) {
      return BerytusSendSignNonceMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendSelectSecurePasswordMessage& aVal) {
      return BerytusSendSelectSecurePasswordMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendExchangePublicKeysMessage& aVal) {
      return BerytusSendExchangePublicKeysMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendComputeClientProofMessage& aVal) {
      return BerytusSendComputeClientProofMessage::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusSendVerifyServerProofMessage& aVal) {
      return BerytusSendVerifyServerProofMessage::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
template <typename K, typename V, typename Q, typename T>
bool RecordWord<K, V, Q, T>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  if (!aValue.isObject()) {
    aRv = false;
    return true;
  }
  if (aValue.isNull()) {
    aRv = false;
    return true;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObject());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    aRv = false;
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    if constexpr (std::is_same_v<K, nsString>) {
      if (!prop.isString()) {
        aRv = false;
        return true;
      }
    } else {
      if (!prop.isInt()) {
        aRv = false;
        return true;
      }
    }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      aRv = false;
      return false;
    }
    bool isValid;
    if (NS_WARN_IF(!V::IsJSValueValid(aCx, val, isValid))) {
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
template <typename K, typename V, typename Q, typename T>
bool RecordWord<K, V, Q, T>::FromJSVal(JSContext *aCx, const JS::Handle<JS::Value> aValue, RecordWord<K, V>& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  if (NS_WARN_IF(aValue.isNull())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, aValue.toObject());
  JS::Rooted<JS::IdVector> ids(aCx, JS::IdVector(aCx));
  if (NS_WARN_IF(!JS_Enumerate(aCx, obj, &ids))) {
    return false;
  }
  JS::Rooted<JS::PropertyKey> prop(aCx);
  JS::Rooted<JS::Value> val(aCx);
  for (size_t i = 0, n = ids.length(); i < n; i++) {
    prop = ids[i];
    typename dom::Record<K, V>::EntryType entry;
    if constexpr (std::is_same_v<K, nsString>) {
      if (NS_WARN_IF(!prop.isString())) {
        return false;
      }
      // TODO(berytus): change to String not CString.
      nsAutoJSCString propName;
      if (NS_WARN_IF(!propName.init(aCx, prop))) {
        return false;
      }
      entry.mKey.Assign(propName);
    } else {
      if (NS_WARN_IF(!prop.isInt())) {
        return false;
      }
      entry.mKey = prop.toInt();
    }

    if (NS_WARN_IF(!JS_GetPropertyById(aCx, obj, prop, &val))) {
      return false;
    }
    V nativeValue;
    if (NS_WARN_IF(!V::FromJSVal(aCx, val, nativeValue))) {
      return false;
    }
    entry.mValue = std::move(nativeValue);
    aRv.Entries().AppendElement(std::move(entry))
  }
  return true;
};
template <typename K, typename V, typename Q, typename T>
bool RecordWord<K, V, Q, T>::ToJSVal(JSContext* aCx, const RecordWord<K, V>& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));
  JS::Rooted<JS::PropertyKey> prop(aCx);
  for (const auto& entry : aValue.Entries()) {
    if constexpr (std::is_same_v<K, nsString>) {
      JS::Rooted<JS::Value> propName(aCx);
      if (NS_WARN_IF(!StringToJSVal(aCx, entry.mKey, propName))) {
        return false;
      }
      prop.set(JS::PropertyKey::fromPinnedString(propName.toString()));
    } else {
      prop.set(JS::PropertyKey::Int(entry.mKey));
    }
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF(!V::ToJSVal(aCx, entry.mValue, &val))) {
      return false;
    }
    if (NS_WARN_IF(!JS_SetPropertyById(aCx, obj, prop, val))) {
      return false;
    }
  }
  aRv.setObjectOrNull(obj);
  return true;
}
bool SafeVariant<RecordWord<nsString, nsString>>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(RecordWord<nsString, nsString>::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<RecordWord<nsString, nsString>>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<RecordWord<nsString, nsString>>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(RecordWord<nsString, nsString>::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      RecordWord<nsString, nsString> nv;
      if (NS_WARN_IF(!(RecordWord<nsString, nsString>::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<RecordWord<nsString, nsString>>::ToJSVal(JSContext* aCx, const SafeVariant<RecordWord<nsString, nsString>>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const RecordWord<nsString, nsString>& aVal) {
      return RecordWord<nsString, nsString>::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusChallengeGetIdentityFieldsMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeGetIdentityFieldsMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetIdentityFieldsMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeGetIdentityFieldsMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeGetIdentityFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusChallengeGetPasswordFieldsMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeGetPasswordFieldsMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeGetPasswordFieldsMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeGetPasswordFieldsMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeGetPasswordFieldsMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<RecordWord<nsString, nsString>>::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusChallengeSelectKeyMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(BerytusKeyFieldValue::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeSelectKeyMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectKeyMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(BerytusKeyFieldValue::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeSelectKeyMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeSelectKeyMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(BerytusKeyFieldValue::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusChallengeSignNonceMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeSignNonceMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSignNonceMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeSignNonceMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeSignNonceMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(ArrayBufferToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<nsString, BerytusEncryptedPacket>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<nsString, BerytusEncryptedPacket>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<nsString, BerytusEncryptedPacket>::ToJSVal(JSContext* aCx, const SafeVariant<nsString, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusChallengeSelectSecurePasswordMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeSelectSecurePasswordMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeSelectSecurePasswordMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeSelectSecurePasswordMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeSelectSecurePasswordMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsString(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      nsString nv;
      if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(JSValIsArrayBuffer(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv.Init(VariantIndex<1>(), ArrayBuffer());
      if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, (aRv.mVariant)->as<ArrayBuffer>())))) {
        return false;
      }
      return true;
      
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusEncryptedPacket::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusEncryptedPacket nv;
      if (NS_WARN_IF(!(BerytusEncryptedPacket::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(JSContext* aCx, const SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const nsString& aVal) {
      return StringToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const ArrayBuffer& aVal) {
      return ArrayBufferToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusEncryptedPacket& aVal) {
      return BerytusEncryptedPacket::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}
bool BerytusChallengeExchangePublicKeysMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeExchangePublicKeysMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeExchangePublicKeysMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeExchangePublicKeysMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeExchangePublicKeysMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusChallengeComputeClientProofMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::IsJSValueValid(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeComputeClientProofMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeComputeClientProofMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::FromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeComputeClientProofMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeComputeClientProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  if (NS_WARN_IF(!aValue.mResponse.Inited())) {
    return false;
  }
  if (NS_WARN_IF(!(SafeVariant<nsString, ArrayBuffer, BerytusEncryptedPacket>::ToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool BerytusChallengeVerifyServerProofMessageResponse::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
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
  if (NS_WARN_IF(!(JSValIsNothing(aCx, propVal, isValid)))) {
    return false;
  }
  if (!isValid) {
    aRv = false;
    return true;
  }
  
  aRv = true;
  return true;


}
bool BerytusChallengeVerifyServerProofMessageResponse::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, BerytusChallengeVerifyServerProofMessageResponse& aRv) {
  if (NS_WARN_IF(!aValue.isObject())) {
    return false;
  }
  JS::Rooted<JSObject*> obj(aCx, &aValue.toObject());
  JS::Rooted<JS::Value> propVal(aCx);
  
  if (NS_WARN_IF(!JS_GetProperty(aCx, obj, "response", &propVal))) {
    return false;
  }
  if (NS_WARN_IF(!(NothingFromJSVal(aCx, propVal, aRv.mResponse)))) {
    return false;
  }
  
  return true;
}
            
bool BerytusChallengeVerifyServerProofMessageResponse::ToJSVal(JSContext* aCx, const BerytusChallengeVerifyServerProofMessageResponse& aValue, JS::MutableHandle<JS::Value> aRv) {
  JS::Rooted<JSObject*> obj(aCx, JS_NewPlainObject(aCx));

  
  JS::Rooted<JS::Value> memberVal0(aCx);
  
  if (NS_WARN_IF(!(NothingToJSVal(aCx, aValue.mResponse, &memberVal0)))) {
    return false;
  }
  if (NS_WARN_IF(!JS_SetProperty(aCx, obj, "response", memberVal0))) {
    return false;
  }
  
  aRv.setObject(*obj);
  return true;
}

bool SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>::IsJSValueValid(JSContext *aCx, const JS::Handle<JS::Value> aValue, bool& aRv) {
  
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeGetIdentityFieldsMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeGetPasswordFieldsMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSelectKeyMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSignNonceMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSelectSecurePasswordMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeExchangePublicKeysMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeComputeClientProofMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      aRv = true;
      return true;
    }
  } while (false);


  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeVerifyServerProofMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
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
bool SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>::FromJSVal(JSContext* aCx, JS::Handle<JS::Value> aValue, SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>& aRv) {
  do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeGetIdentityFieldsMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeGetIdentityFieldsMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeGetIdentityFieldsMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<0>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeGetPasswordFieldsMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeGetPasswordFieldsMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeGetPasswordFieldsMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<1>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSelectKeyMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSelectKeyMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeSelectKeyMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<2>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSignNonceMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSignNonceMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeSignNonceMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<3>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeSelectSecurePasswordMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeSelectSecurePasswordMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeSelectSecurePasswordMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<4>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeExchangePublicKeysMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeExchangePublicKeysMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeExchangePublicKeysMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<5>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeComputeClientProofMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeComputeClientProofMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeComputeClientProofMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<6>(), std::move(nv));
      return true;
    }
  } while (false);
  
do {
    bool isValid = false;
    if (NS_WARN_IF(!(BerytusChallengeVerifyServerProofMessageResponse::IsJSValueValid(aCx, aValue, isValid)))) {
      return false;
    }
    if (isValid) {
      BerytusChallengeVerifyServerProofMessageResponse nv;
      if (NS_WARN_IF(!(BerytusChallengeVerifyServerProofMessageResponse::FromJSVal(aCx, aValue, nv)))) {
        return false;
      }
      aRv.Init(VariantIndex<7>(), std::move(nv));
      return true;
    }
  } while (false);
  

  NS_WARNING_ASSERTION(true, "None of the subtypes returned a truthful IsValid()");
  return false;
}
bool SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>::ToJSVal(JSContext* aCx, const SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>& aValue, JS::MutableHandle<JS::Value> aRv) {
  struct Matcher {
    JSContext* mCx;
    JS::MutableHandle<JS::Value> mRv;
    Matcher(JSContext* aCx, JS::MutableHandle<JS::Value> aRv) : mCx(aCx), mRv(aRv) {}
    
    bool operator()(const BerytusChallengeGetIdentityFieldsMessageResponse& aVal) {
      return BerytusChallengeGetIdentityFieldsMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeGetPasswordFieldsMessageResponse& aVal) {
      return BerytusChallengeGetPasswordFieldsMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSelectKeyMessageResponse& aVal) {
      return BerytusChallengeSelectKeyMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSignNonceMessageResponse& aVal) {
      return BerytusChallengeSignNonceMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeSelectSecurePasswordMessageResponse& aVal) {
      return BerytusChallengeSelectSecurePasswordMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeExchangePublicKeysMessageResponse& aVal) {
      return BerytusChallengeExchangePublicKeysMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeComputeClientProofMessageResponse& aVal) {
      return BerytusChallengeComputeClientProofMessageResponse::ToJSVal(mCx, aVal, mRv);
    }

    bool operator()(const BerytusChallengeVerifyServerProofMessageResponse& aVal) {
      return BerytusChallengeVerifyServerProofMessageResponse::ToJSVal(mCx, aVal, mRv);
    }
  };
  return aValue.InternalValue()->match(Matcher(aCx, aRv));
}


RefPtr<ManagerGetSigningKeyResult> AgentProxy::Manager_GetSigningKey(PreliminaryRequestContext& aContext, GetSigningKeyArgs& aArgs) const {
  RefPtr<ManagerGetSigningKeyResult::Private> outPromise = new ManagerGetSigningKeyResult::Private(__func__);
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
    if (NS_WARN_IF(!(StringFromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(NumberFromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(PartialKeyExchangeParametersFromScm::FromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(ArrayBufferFromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(ELoginUserIntent::FromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(RecordMetadata::FromJSVal(aCx, aValue, out)))) {
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
    if (NS_WARN_IF(!(nsTArray_UserAttribute_FromJSVal(aCx, aValue, out)))) {
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
RefPtr<AccountCreationUpdateUserAttributesResult> AgentProxy::AccountCreation_UpdateUserAttributes(RequestContextWithOperation& aContext, UpdateUserAttributesArgs& aArgs) const {
  RefPtr<AccountCreationUpdateUserAttributesResult::Private> outPromise = new AccountCreationUpdateUserAttributesResult::Private(__func__);
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
RefPtr<AccountCreationAddFieldResult> AgentProxy::AccountCreation_AddField(RequestContextWithLoginOperation& aContext, AddFieldArgs& aArgs) const {
  RefPtr<AccountCreationAddFieldResult::Private> outPromise = new AccountCreationAddFieldResult::Private(__func__);
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
    SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> out;
    if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>::FromJSVal(aCx, aValue, out)))) {
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
RefPtr<AccountCreationRejectFieldValueResult> AgentProxy::AccountCreation_RejectFieldValue(RequestContextWithLoginOperation& aContext, RejectFieldValueArgs& aArgs) const {
  RefPtr<AccountCreationRejectFieldValueResult::Private> outPromise = new AccountCreationRejectFieldValueResult::Private(__func__);
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
    SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue> out;
    if (NS_WARN_IF(!(SafeVariant<nsString, BerytusEncryptedPacket, BerytusKeyFieldValue, BerytusSecurePasswordFieldValue, BerytusSharedKeyFieldValue>::FromJSVal(aCx, aValue, out)))) {
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
RefPtr<AccountAuthenticationRespondToChallengeMessageResult> AgentProxy::AccountAuthentication_RespondToChallengeMessage(RequestContextWithOperation& aContext, SafeVariant<BerytusSendGetIdentityFieldsMessage, BerytusSendGetPasswordFieldsMessage, BerytusSendSelectKeyMessage, BerytusSendSignNonceMessage, BerytusSendSelectSecurePasswordMessage, BerytusSendExchangePublicKeysMessage, BerytusSendComputeClientProofMessage, BerytusSendVerifyServerProofMessage>& aArgs) const {
  RefPtr<AccountAuthenticationRespondToChallengeMessageResult::Private> outPromise = new AccountAuthenticationRespondToChallengeMessageResult::Private(__func__);
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
    SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse> out;
    if (NS_WARN_IF(!(SafeVariant<BerytusChallengeGetIdentityFieldsMessageResponse, BerytusChallengeGetPasswordFieldsMessageResponse, BerytusChallengeSelectKeyMessageResponse, BerytusChallengeSignNonceMessageResponse, BerytusChallengeSelectSecurePasswordMessageResponse, BerytusChallengeExchangePublicKeysMessageResponse, BerytusChallengeComputeClientProofMessageResponse, BerytusChallengeVerifyServerProofMessageResponse>::FromJSVal(aCx, aValue, out)))) {
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