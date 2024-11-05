#include "mozilla/dom/BerytusDataVariant.h"
#include "BerytusFieldValueDictionary.h"
#include "ErrorList.h"
#include "js/ArrayBuffer.h"
#include "mozilla/dom/TypedArray.h"
#include "mozilla/dom/AuthRealmUtils.h"
#include "mozilla/dom/UnionTypes.h"

namespace mozilla::dom {

//NS_IMPL_CYCLE_COLLECTION(BerytusDataVariant)
/**
 * I could not find any documentation on handing Variant<RefPtr<..>>
 *  for cycle colleciton. The below is adapted from ResizeObserver.cpp.
 * The commented bit above was sufficient, without referencing mValue
 *  for cycle collection, to compile successfully.
 */
/* START */
NS_IMPL_CYCLE_COLLECTION_CLASS(BerytusDataVariant)

NS_IMPL_CYCLE_COLLECTION_TRAVERSE_BEGIN(BerytusDataVariant)
  NS_IMPL_CYCLE_COLLECTION_TRAVERSE();
  if (tmp->mValue.is<RefPtr<BerytusEncryptedPacket>>()) {
    ImplCycleCollectionTraverse(
        cb, tmp->mValue.as<RefPtr<BerytusEncryptedPacket>>(), "mValue", 0);
  }
  if (tmp->mValue.is<RefPtr<BerytusFieldValueDictionary>>()) {
    ImplCycleCollectionTraverse(
        cb, tmp->mValue.as<RefPtr<BerytusFieldValueDictionary>>(), "mValue", 0);
  }
NS_IMPL_CYCLE_COLLECTION_TRAVERSE_END

NS_IMPL_CYCLE_COLLECTION_UNLINK_BEGIN(BerytusDataVariant)
  NS_IMPL_CYCLE_COLLECTION_UNLINK();
  if (tmp->mValue.is<RefPtr<BerytusEncryptedPacket>>()) {
    ImplCycleCollectionUnlink(tmp->mValue.as<RefPtr<BerytusEncryptedPacket>>());
  }
  if (tmp->mValue.is<RefPtr<BerytusFieldValueDictionary>>()) {
    ImplCycleCollectionUnlink(tmp->mValue.as<RefPtr<BerytusFieldValueDictionary>>());
  }
  //   NS_IMPL_CYCLE_COLLECTION_UNLINK_PRESERVED_WRAPPER
NS_IMPL_CYCLE_COLLECTION_UNLINK_END
/* END */

NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusDataVariant)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusDataVariant)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusDataVariant)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusDataVariant::BerytusDataVariant(const bool& aValue) : mValue(aValue) {}
BerytusDataVariant::BerytusDataVariant(const nsAString& aValue) : mValue(nsString()) {
    mValue.as<nsString>().Assign(aValue);
}
BerytusDataVariant::BerytusDataVariant(const nsACString& aValue) : mValue(nsCString()) {
    mValue.as<nsCString>().Assign(aValue);
}
BerytusDataVariant::BerytusDataVariant(
    JSContext* aCx,
    const ArrayBuffer& aValue,
    ErrorResult& aRv
) : mValue(ArrayBuffer()) {
    AuthRealmUtils::ArrayBufferToByteArray(aValue, mArrayBufferBytes);
    // BRTTODO: 3/1/2024 - the below didnt seem to work after a GC cycle...
    // For some reason, the buffer got nuked. I am storing the bytes manually instead...

    // let's copy the buffer.
    // aValue.ComputeState();
    // JSObject* jsBuf = ArrayBuffer::Create(aCx, aValue.Length(), aValue.Data());
    // if (NS_WARN_IF(!jsBuf)) {
    //     aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    //     return;
    // }
    // mArrayBuffer = JS::Heap<JSObject*>(jsBuf);
    // mValue.as<ArrayBuffer>().Init(mArrayBuffer.get());
}
BerytusDataVariant::BerytusDataVariant(
    JSContext* aCx,
    const ArrayBufferView& aValue,
    ErrorResult& aRv
) : mValue(ArrayBufferView()) {
    AuthRealmUtils::ArrayBufferViewToByteArray(aValue, mArrayBufferBytes);

    // let's copy the buffer.
    // aValue.ComputeState();
    // JSObject* jsBuf = ArrayBuffer::Create(aCx, aValue.Length(), aValue.Data());
    // if (NS_WARN_IF(!jsBuf)) {
    //     aRv.Throw(NS_ERROR_OUT_OF_MEMORY);
    //     return;
    // }

    // //mArrayBuffer = jsBuf;
    // mArrayBuffer = JS::Heap<JSObject*>(jsBuf);
    // // BRTTODO: We are copying the ArrayBufferView to an ArrayBuffer instance...
    // mValue.as<ArrayBuffer>().Init(jsBuf);
}
BerytusDataVariant::BerytusDataVariant(BerytusEncryptedPacket* aValue)
    : mValue({VariantIndex<5>{}, RefPtr<BerytusEncryptedPacket>(aValue)})
{}
BerytusDataVariant::BerytusDataVariant(BerytusFieldValueDictionary* aValue)
    : mValue({VariantIndex<6>{}, RefPtr<BerytusFieldValueDictionary>(aValue)})
{}

BerytusDataVariant::~BerytusDataVariant() { }

mozIAuthRealmFieldValue* BerytusDataVariant::ToAuthRealmFieldValue(nsresult* aOutRes)
{
    return mValue.match(BerytusFieldValueMatcher(aOutRes, mArrayBufferBytes));
}

nsresult BerytusDataVariant::GetValue(OwningStringOrBerytusEncryptedPacket& aRetVal)
{
    if (mValue.is<nsString>()) {
        aRetVal.SetAsString() = mValue.as<nsString>();
        return NS_OK;
    }
    if (mValue.is<nsCString>()) {
        aRetVal.SetAsString() = NS_ConvertUTF8toUTF16(mValue.as<nsCString>());
        return NS_OK;
    }
    if (mValue.is<RefPtr<BerytusEncryptedPacket>>()) {
        aRetVal.SetAsBerytusEncryptedPacket() = mValue.as<RefPtr<BerytusEncryptedPacket>>();
        return NS_OK;
    }
    printf("GetValue for String or Packet called: smthn else\n");
    return NS_ERROR_UNEXPECTED;
}

nsresult BerytusDataVariant::GetValue(
    JSContext* aCx,
    OwningStringOrArrayBufferOrBerytusEncryptedPacket& aRetVal
) {
    if (mValue.is<nsString>()) {
        aRetVal.SetAsString() = mValue.as<nsString>();
        return NS_OK;
    }
    if (mValue.is<nsCString>()) {
        aRetVal.SetAsString() = NS_ConvertUTF8toUTF16(mValue.as<nsCString>());
        return NS_OK;
    }
    if (mValue.is<RefPtr<BerytusEncryptedPacket>>()) {
        aRetVal.SetAsBerytusEncryptedPacket() = mValue.as<RefPtr<BerytusEncryptedPacket>>();
        return NS_OK;
    }
    if (mValue.is<ArrayBuffer>() || mValue.is<ArrayBufferView>()) {
        // let's copy it...
        if (NS_WARN_IF(! aCx)) {
            return NS_ERROR_UNEXPECTED;
        }

        // const ArrayBuffer& storedBuf = mValue.as<ArrayBuffer>();
        // storedBuf.ComputeState();

        JSObject* newBuf = ArrayBuffer::Create(aCx, mArrayBufferBytes.Length(), mArrayBufferBytes.Elements());
        if (NS_WARN_IF(!newBuf)) {
            return NS_ERROR_OUT_OF_MEMORY;
        }
        aRetVal.SetAsArrayBuffer().Init(newBuf);
        return NS_OK;
    }
    printf("GetValue for String or ArrayBuffer or Packet called: unexpected variant\n");
    return NS_ERROR_UNEXPECTED;
}

nsresult BerytusDataVariant::GetValue(
    JSContext* aCx, // can be a nullptr when the caller is sure that the value is not an ArrayBuffer(View)
    OwningStringOrArrayBufferOrBerytusEncryptedPacketOrBerytusFieldValueDictionary& aRetVal
)
{
    if (mValue.is<nsString>()) {
        aRetVal.SetAsString() = mValue.as<nsString>();
        return NS_OK;
    }
    if (mValue.is<nsCString>()) {
        aRetVal.SetAsString() = NS_ConvertUTF8toUTF16(mValue.as<nsCString>());
        return NS_OK;
    }
    if (mValue.is<ArrayBuffer>() || mValue.is<ArrayBufferView>()) {
        // let's copy it...
        if (NS_WARN_IF(! aCx)) {
            return NS_ERROR_UNEXPECTED;
        }

        // const ArrayBuffer& storedBuf = mValue.as<ArrayBuffer>();
        // storedBuf.ComputeState();

        JSObject* newBuf = ArrayBuffer::Create(aCx, mArrayBufferBytes.Length(), mArrayBufferBytes.Elements());
        if (NS_WARN_IF(!newBuf)) {
            return NS_ERROR_OUT_OF_MEMORY;
        }
        aRetVal.SetAsArrayBuffer().Init(newBuf);
        return NS_OK;
    }
    if (mValue.is<RefPtr<BerytusEncryptedPacket>>() || mValue.is<5>()) {
        aRetVal.SetAsBerytusEncryptedPacket() = mValue.as<RefPtr<BerytusEncryptedPacket>>();
        return NS_OK;
    }
    if (mValue.is<RefPtr<BerytusFieldValueDictionary>>() || mValue.is<6>()) {
        aRetVal.SetAsBerytusFieldValueDictionary() = mValue.as<RefPtr<BerytusFieldValueDictionary>>();
        return NS_OK;
    }
    if (mValue.is<bool>()) {
        printf("it's a bool bro\n");
    }
    printf("BerytusDataVariant:GetValue not compatible with expected return type (1).\n");
    return NS_ERROR_UNEXPECTED;
}

nsresult BerytusDataVariant::GetValue(
    JSContext* aCx,
    OwningArrayBufferOrBerytusEncryptedPacket& aRetVal
) {
    if (mValue.is<ArrayBuffer>() || mValue.is<ArrayBufferView>()) {
        // let's copy it...
        if (NS_WARN_IF(! aCx)) {
            return NS_ERROR_UNEXPECTED;
        }

        // const ArrayBuffer& storedBuf = mValue.as<ArrayBuffer>();
        // storedBuf.ComputeState();

        JSObject* newBuf = ArrayBuffer::Create(aCx, mArrayBufferBytes.Length(), mArrayBufferBytes.Elements());
        if (NS_WARN_IF(!newBuf)) {
            return NS_ERROR_OUT_OF_MEMORY;
        }
        aRetVal.SetAsArrayBuffer().Init(newBuf);
        return NS_OK;
    }
    if (mValue.is<RefPtr<BerytusEncryptedPacket>>()) {
        aRetVal.SetAsBerytusEncryptedPacket() = mValue.as<RefPtr<BerytusEncryptedPacket>>();
        return NS_OK;
    }

    printf("BerytusDataVariant:GetValue not compatible with expected return type (2).\n");
    return NS_ERROR_UNEXPECTED;
}


bool BerytusDataVariant::IsCString()
{
    return mValue.is<nsString>();
}
bool BerytusDataVariant::IsString()
{
    return mValue.is<nsCString>();
}
bool BerytusDataVariant::IsArrayBuffer()
{
    return mValue.is<ArrayBuffer>();
}
bool BerytusDataVariant::IsArrayBufferView()
{
    return mValue.is<ArrayBufferView>();
}
bool BerytusDataVariant::IsBerytusEncryptedPacket()
{
    return mValue.is<RefPtr<BerytusEncryptedPacket>>();
}
bool BerytusDataVariant::IsBerytusFieldValueDictionary()
{
    return mValue.is<RefPtr<BerytusFieldValueDictionary>>();
}

RefPtr<BerytusFieldValueDictionary> BerytusDataVariant::AsBerytusFieldValueDictionary()
{
    return mValue.as<RefPtr<BerytusFieldValueDictionary>>();
}


}