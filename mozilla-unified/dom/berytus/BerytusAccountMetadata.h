#ifndef DOM_BERYTUSACCOUNTMETADATA_H_
#define DOM_BERYTUSACCOUNTMETADATA_H_

#include <type_traits>
//#include "mozIAuthRealmSession.h"
#include "mozilla/dom/BerytusAccountBinding.h" // for BerytusAccountStatus
#include "nsVariant.h"
#include "nsIGlobalObject.h"
#include "mozIBerytusRegistrationOperation.h" // mozIBerytusRegistrationOperation

namespace mozilla::dom {

// template<
//     class T,
//     typename = std::enable_if_t<std::is_base_of_v<mozIAuthRealmSession, T>>
// >
class BerytusAccountMetadata {
public:
    void GetCategory(nsString& aRetVal) const;

    uint64_t Version() const;

    BerytusAccountStatus Status() const;

    void GetChangePasswordUrl(nsString& aRetVal) const;

    already_AddRefed<Promise> SetVersion(uint64_t aVersion, ErrorResult& aRv);

    // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
    already_AddRefed<Promise> SetCategory(
        const nsAString& aCategory,
        ErrorResult& aRv
    );

    // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
    already_AddRefed<Promise> SetStatus(
        BerytusAccountStatus aStatus,
        ErrorResult& aRv
    );

    // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
    already_AddRefed<Promise> SetChangePasswordUrl(
        const nsAString& aUrl,
        ErrorResult& aRv
    );
protected:
    virtual nsIGlobalObject* Global() const = 0; // perhaps These should be placed in a separate class if we implement other Account classes
    virtual mozIAuthRealmSession* Operation() const = 0; // perhaps These should be placed in a separate class if we implement other Account classes

    BerytusAccountStatus mStatus = BerytusAccountStatus::Pending;
    uint64_t mVersion = 0;
    nsString mCategory = nsString();
    nsString mChangePassUrl = nsString();

    nsresult ReadMetadata();

    nsresult UpdateMetadata(
        mozIBerytusRegistrationOperation::MetadataProperty&& aProperty,
        RefPtr<nsVariantCC>& aValue,
        RefPtr<Promise>& aOutPromise
    );
    nsresult UpdateMetadata(
        nsTArray<mozIBerytusRegistrationOperation::MetadataProperty>& aProperties,
        nsTArray<RefPtr<nsVariantCC>>& aValues,
        RefPtr<Promise>& aOutPromise
    );
    nsresult ApplyMetadata(
        nsTArray<RefPtr<nsIPropertyBag2>>& aProperties
    );
    nsresult ApplyMetadataProperty(
        mozIBerytusRegistrationOperation::MetadataProperty& aProperty,
        nsIVariant* aValue
    );
};

}

#endif