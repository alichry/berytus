#include "BerytusAccountMetadata.h"
#include "AuthRealmUtils.h"
#include "mozilla/dom/CommonPromiseHandler.h"

namespace mozilla::dom {

void BerytusAccountMetadata::GetCategory(nsString& aRetVal) const { aRetVal.Assign(mCategory); }

uint64_t BerytusAccountMetadata::Version() const { return mVersion; }

BerytusAccountStatus BerytusAccountMetadata::Status() const { return mStatus; }

void BerytusAccountMetadata::GetChangePasswordUrl(nsString& aRetVal) const { aRetVal.Assign(mChangePassUrl); }

nsresult BerytusAccountMetadata::ReadMetadata()
{
    nsresult res;
    nsTArray<RefPtr<nsIPropertyBag2>> properties;
    res = Operation()->GetRecordMetadata(properties);
    if (NS_FAILED(res)) {
        return res;
    }
    res = ApplyMetadata(properties);
    if (NS_FAILED(res)) {
        return res;
    }
    return NS_OK;
}

already_AddRefed<Promise> BerytusAccountMetadata::SetVersion(uint64_t aVersion, ErrorResult& aRv)
{
    nsresult res;
    RefPtr<Promise> outPromise;
    BR_CREATE_PROMISE_OR_RET_NULLPTR_WITH_ERR(outPromise, Global(), aRv);
    RefPtr<nsVariantCC> value = new nsVariantCC();
    value->SetAsUint64(aVersion);
    res = UpdateMetadata(
        mozIBerytusRegistrationOperation::MetadataProperty::EMetadataPropertyVersion,
        value,
        outPromise
    );
    AR_RET_REJ_OPERROR_IF(NS_FAILED(res), outPromise, "Unable to update metadata");
    return outPromise.forget();
}


// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetCategory(
    const nsAString& aCategory,
    ErrorResult& aRv
) {
    nsresult res;
    RefPtr<Promise> outPromise;
    BR_CREATE_PROMISE_OR_RET_NULLPTR_WITH_ERR(outPromise, Global(), aRv);
    RefPtr<nsVariantCC> value = new nsVariantCC();
    value->SetAsAString(aCategory);
    res = UpdateMetadata(
        mozIBerytusRegistrationOperation::MetadataProperty::EMetadataPropertyCategory,
        value,
        outPromise
    );
    AR_RET_REJ_OPERROR_IF(NS_FAILED(res), outPromise, "Unable to update metadata");
    return outPromise.forget();
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetStatus(
    BerytusAccountStatus aStatus,
    ErrorResult& aRv
) {
    nsresult res;
    RefPtr<Promise> outPromise;
    BR_CREATE_PROMISE_OR_RET_NULLPTR_WITH_ERR(outPromise, Global(), aRv);
    RefPtr<nsVariantCC> value = new nsVariantCC();
    value->SetAsUint8(static_cast<uint8_t>(aStatus));
    res = UpdateMetadata(
        mozIBerytusRegistrationOperation::MetadataProperty::EMetadataPropertyStatus,
        value,
        outPromise
    );
    AR_RET_REJ_OPERROR_IF(NS_FAILED(res), outPromise, "Unable to update metadata");
    return outPromise.forget();
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetChangePasswordUrl(
    const nsAString& aUrl,
    ErrorResult& aRv
) {
    nsresult res;
    RefPtr<Promise> outPromise;
    BR_CREATE_PROMISE_OR_RET_NULLPTR_WITH_ERR(outPromise, Global(), aRv);
    RefPtr<nsVariantCC> value = new nsVariantCC();
    value->SetAsAString(aUrl);
    res = UpdateMetadata(
        mozIBerytusRegistrationOperation::MetadataProperty::EMetadataPropertyChangePassUrl,
        value,
        outPromise
    );
    AR_RET_REJ_OPERROR_IF(NS_FAILED(res), outPromise, "Unable to update metadata");
    return outPromise.forget();
}

nsresult BerytusAccountMetadata::UpdateMetadata(
    mozIBerytusRegistrationOperation::MetadataProperty&& aProperty,
    RefPtr<nsVariantCC>& aValue,
    RefPtr<Promise>& aOutPromise
) {
    nsTArray<mozIBerytusRegistrationOperation::MetadataProperty> props;
    nsTArray<RefPtr<nsVariantCC>> vals;
    props.AppendElement(aProperty);
    vals.AppendElement(aValue);
    return UpdateMetadata(props, vals, aOutPromise);
}
nsresult BerytusAccountMetadata::UpdateMetadata(
    nsTArray<mozIBerytusRegistrationOperation::MetadataProperty>& aProperties,
    nsTArray<RefPtr<nsVariantCC>>& aValues,
    RefPtr<Promise>& aOutPromise
) {
    nsresult res;
    if (aProperties.Length() != aValues.Length()) {
        return NS_ERROR_INVALID_ARG;
    }
    nsTArray<RefPtr<nsIPropertyBag2>>* bags = new nsTArray<RefPtr<nsIPropertyBag2>>(); // 3amak abo l bags bi3atil belleil w benhar
    RefPtr<CommonPromiseHandler> handler = new CommonPromiseHandler(
        aOutPromise,
        "Unable to fulfill the set metadata request"_ns,
        "User aborted the set metadata request"_ns
    );
    handler->SetFinallyCallback(
        [bags](bool resolved) {
            delete bags;
        }
    );
    RefPtr<nsHashPropertyBag> bag = new nsHashPropertyBag();
    for (size_t i = 0; i < aProperties.Length(); i++) {
        RefPtr<nsHashPropertyBag> bag = new nsHashPropertyBag();

        //RefPtr<nsVariantCC> propVariant = new nsVariantCC();
        // propVariant->SetAsUint8(aProperties.ElementAt(i));
        // bag->SetProperty(u"property"_ns, propVariant);
        bag->SetPropertyAsUint32(u"property"_ns, aProperties.ElementAt(i));
        bag->SetProperty(u"value"_ns, aValues.ElementAt(i));

        bags->AppendElement(bag);
    }

    Promise* metadataPromise;
    handler->SetResolvedCallback(
        [this, bags](RefPtr<Promise>& aOuterPromise, JSContext* aCx, JS::Handle<JS::Value> aValue) {
            nsresult res;
            res = ApplyMetadata(*bags);
            AR_HALT_REJ_OPERROR_IF(NS_WARN_IF(NS_FAILED(res)), aOuterPromise, "Unable to apply metadata property");
            aOuterPromise->MaybeResolveWithUndefined();
        }
    );
    nsCOMPtr<mozIBerytusRegistrationOperation> op = do_QueryInterface(Operation(), &res);
    if (NS_WARN_IF(NS_FAILED(res))) {
        // This happens when the parent return a non-registration operation.
        // login operations can access the metadata but not update it.
        // Oepration() either returns a login operation or a registration operation.
        return res;
    }

    res = op->UpdateMetadata(*bags, &metadataPromise);
    if (NS_FAILED(res)) {
        return res;
    }
    metadataPromise->AppendNativeHandler(handler);
    return NS_OK;
}

nsresult BerytusAccountMetadata::ApplyMetadata(
    nsTArray<RefPtr<nsIPropertyBag2>>& aProperties
) {
    nsresult res;
    for (const auto& bag : aProperties) {
        uint32_t keyVal;
        bag->GetPropertyAsUint32(u"property"_ns, &keyVal);
        mozIBerytusRegistrationOperation::MetadataProperty key = static_cast<mozIBerytusRegistrationOperation::MetadataProperty>(keyVal);
        nsIVariant* variant;
        bag->GetProperty(u"value"_ns, &variant);

        res = this->ApplyMetadataProperty(
            key,
            variant
        );
        if (NS_FAILED(res)) {
            return res;
        }
    }
    return NS_OK;
}
nsresult BerytusAccountMetadata::ApplyMetadataProperty(
    mozIBerytusRegistrationOperation::MetadataProperty& aProperty,
    nsIVariant* aValue
) {
    nsresult res;
    if (aProperty == mozIBerytusRegistrationOperation::EMetadataPropertyStatus) {
        uint8_t status;
        res = aValue->GetAsUint8(&status);
        if (NS_FAILED(res)) {
            return res;
        }
        if (status >= static_cast<uint8_t>(BerytusAccountStatus::EndGuard_)) {
            return NS_ERROR_FAILURE;
        }
        mStatus = BerytusAccountStatus(status);
    } else if (aProperty == mozIBerytusRegistrationOperation::EMetadataPropertyVersion) {
        res = aValue->GetAsUint64(&mVersion);
        if (NS_FAILED(res)) {
            return res;
        }
    } else if (aProperty == mozIBerytusRegistrationOperation::EMetadataPropertyCategory) {
        res = aValue->GetAsAString(mCategory);
        if (NS_FAILED(res)) {
            return res;
        }
    } else if (aProperty == mozIBerytusRegistrationOperation::EMetadataPropertyChangePassUrl) {
        res = aValue->GetAsAString(mChangePassUrl);
        if (NS_FAILED(res)) {
            return res;
        }
    } else {
        return NS_ERROR_INVALID_ARG;
    }
    return NS_OK;
}
}