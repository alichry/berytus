// A data container for these different data types:
// 1. DOMString --> easy conversion to authrealm field value
// 2. ArrayBuffer --> easy conversion to authrealm field value
// 3. ArrayBufferView --> easy conversion to authrealm field value
// 4. Encrypted Packet --> easy conversion to authrealm field value
// 5. Typed field value dictionaries

// The whole purpose of this class is to provide the
// GetValue method and ToAuthRealmFieldValue

#ifndef DOM_BERYTUSDATAVARIANT_H_
#define DOM_BERYTUSDATAVARIANT_H_

#include "ErrorList.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/Components.h" // mozilla::components::AuthRealmFactory
#include "mozilla/Variant.h"
#include "mozilla/dom/TypedArray.h" // ArrayBuffer
#include "nsISupports.h"
#include "nsCycleCollectionParticipant.h"
#include "mozilla/dom/BerytusEncryptedPacket.h"
#include "mozIAuthRealmRegistrationField.h" // mozIAuthRealmFieldValue
#include "mozIAuthRealmFactory.h"
#include "mozilla/dom/BerytusFieldBinding.h" // OwningStringOrArrayBufferOrBerytusEncryptedPacketOrBerytusFieldValueDictionary
#include "mozilla/dom/BerytusFieldValueDictionary.h" // Referred to in BerytusFieldValueMatcher
#include "mozilla/dom/BerytusSecurePasswordFieldBinding.h" // OwningStringOrBerytusEncryptedPacket
#include "mozilla/dom/BerytusEncryptedPacketBinding.h" // OwningStringOrArrayBufferOrBerytusEncryptedPacket

namespace mozilla::dom {

struct BerytusFieldValueMatcher {
    nsresult* mOutRes;
    nsTArray<uint8_t>& mArrayBufferBytes;
    BerytusFieldValueMatcher(nsresult* aOutRes, nsTArray<uint8_t>& aArrayBufferBytes) :
        mArrayBufferBytes(aArrayBufferBytes) {
        mOutRes = aOutRes;
    }

    static mozIAuthRealmFieldValue* FieldValueFromByteArray(
        const nsTArray<uint8_t>& byteArray,
        nsresult* aOutRes
    ) {
        *aOutRes = NS_OK;
        nsresult res;
        nsCOMPtr<mozIAuthRealmFactory> fact =
            mozilla::components::AuthRealmFactory::Create(&res);
        if (NS_WARN_IF(NS_FAILED(res))) {
            *aOutRes = res;
            return nullptr;
        }

        mozIAuthRealmFieldValue* fieldValue;
        res = fact->CreateFieldValue(byteArray, &fieldValue);
        if (NS_WARN_IF(NS_FAILED(res))) {
            *aOutRes = res;
            return nullptr;
        }
        return fieldValue;
    }
    // The return type of all matchers must be identical.
    mozIAuthRealmFieldValue* operator()(bool& a) {
        nsTArray<uint8_t> byteArray(1);
        byteArray[0] = a;
        return FieldValueFromByteArray(byteArray, mOutRes);
    }
    mozIAuthRealmFieldValue* operator()(nsString& b) {
        /* convert string to byte array */
        nsTArray<uint8_t> byteArray;
        nsCString utf8String = NS_ConvertUTF16toUTF8(b);

        byteArray.SetLength(utf8String.Length());
        for (size_t i = 0; i < utf8String.Length(); i++) {
            byteArray[i] = utf8String.CharAt(i);
        }

        return FieldValueFromByteArray(byteArray, mOutRes);
    }
    mozIAuthRealmFieldValue* operator()(nsCString& str) {
        nsTArray<uint8_t> byteArray;
        byteArray.SetLength(str.Length());
        for (size_t i = 0; i < str.Length(); i++) {
            byteArray[i] = str.CharAt(i);
        }
        return FieldValueFromByteArray(byteArray, mOutRes);
    }
    mozIAuthRealmFieldValue* operator()(ArrayBuffer& arrayBuffer) {
        // arrayBuffer.ComputeState();
        // nsTArray<uint8_t> byteArray(arrayBuffer.Length());
        // byteArray.InsertElementsAt(
        //     0,
        //     arrayBuffer.Data(),
        //     arrayBuffer.Length()
        // );
        // return FieldValueFromByteArray(byteArray, mOutRes);
        return FieldValueFromByteArray(mArrayBufferBytes, mOutRes);
     }
    mozIAuthRealmFieldValue* operator()(ArrayBufferView& arrayBuffer) {
        // arrayBuffer.ComputeState();
        // nsTArray<uint8_t> byteArray(arrayBuffer.Length());
        // for (size_t i = 0; i < arrayBuffer.Length(); i++) {
        //     byteArray[i] = arrayBuffer.Data()[i];
        // }
        // return FieldValueFromByteArray(byteArray, mOutRes);
        return FieldValueFromByteArray(mArrayBufferBytes, mOutRes);
    }
    mozIAuthRealmFieldValue* operator()(RefPtr<BerytusEncryptedPacket>& packet) {
        // TODO: Special field value needed.
        printf("###### FUNCBRTTODO: EncryptedPacket->mozIAuthRealmFieldValue ######\n");
        *mOutRes = NS_ERROR_UNEXPECTED;
        return nullptr;
    }
    mozIAuthRealmFieldValue* operator()(RefPtr<BerytusFieldValueDictionary>& dict) {
        return dict->ToAuthRealmFieldValue(mOutRes);
    }
};

class BerytusDataVariant final : public nsISupports
{
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_CLASS(BerytusDataVariant)

public:
    explicit BerytusDataVariant(const bool& aValue);
    explicit BerytusDataVariant(const nsAString& aValue);
    explicit BerytusDataVariant(const nsACString& aValue);
    explicit BerytusDataVariant(JSContext* aCx, const ArrayBuffer& aValue, ErrorResult& aRv);
    explicit BerytusDataVariant(
        JSContext* aCx,
        const ArrayBufferView& aValue,
        ErrorResult& aRv
    );
    explicit BerytusDataVariant(BerytusEncryptedPacket* aValue);
    explicit BerytusDataVariant(BerytusFieldValueDictionary* aValue);


    mozIAuthRealmFieldValue* ToAuthRealmFieldValue(nsresult* aOutRes);

    nsresult GetValue(OwningStringOrBerytusEncryptedPacket& aRetVal);

    nsresult GetValue(
        JSContext*  aCx, // JSContext needed to construct new ArrayBuffer
        OwningStringOrArrayBufferOrBerytusEncryptedPacketOrBerytusFieldValueDictionary& aRetVal
    );

    nsresult GetValue(
        JSContext* aCx,
        OwningStringOrArrayBufferOrBerytusEncryptedPacket& aRetVal
    );

    nsresult GetValue(
        JSContext* aCx,
        OwningArrayBufferOrBerytusEncryptedPacket& aRetVal
    );

    bool IsBool();
    bool IsCString();
    bool IsString();
    bool IsArrayBuffer();
    bool IsArrayBufferView();
    bool IsBerytusEncryptedPacket();
    bool IsBerytusFieldValueDictionary();
    RefPtr<BerytusFieldValueDictionary> AsBerytusFieldValueDictionary();
protected:
    ~BerytusDataVariant();
    Variant<
        bool,
        nsCString,
        nsString,
        ArrayBuffer,
        ArrayBufferView,
        RefPtr<BerytusEncryptedPacket>,
        RefPtr<BerytusFieldValueDictionary>
    > mValue;
    nsTArray<uint8_t> mArrayBufferBytes;
    // BRTTODO: 3/2/2024: ^ This should be included in SCRIPT_HOLDER
};

}

#endif