#ifndef DOM_BERYTUSACCOUNTMETADATA_H_
#define DOM_BERYTUSACCOUNTMETADATA_H_

#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusAccountBinding.h" // for BerytusAccountStatus
#include "nsVariant.h"
#include "nsIGlobalObject.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusChannel.h"

namespace mozilla::dom {

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
    BerytusAccountMetadata();
    virtual ~BerytusAccountMetadata();
    virtual nsIGlobalObject* GetParentObject() const = 0;
    virtual BerytusChannel* Channel() const = 0;
    virtual BerytusLoginOperation* Operation() = 0;
    virtual bool Active() const = 0;

    BerytusAccountStatus mStatus;
    uint64_t mVersion;
    nsString mCategory;
    nsString mChangePassUrl;

    RefPtr<berytus::LoginUpdateMetadataResult>
    UpdateMetadata(
        const uint64_t& aVersion,
        const BerytusAccountStatus& aStatus,
        const nsAString& aCategory,
        const nsAString& aChangePassUrl,
        ErrorResult& aRv
    );

    already_AddRefed<Promise> UpdateMetadataPromise(
        const uint64_t& aVersion,
        const BerytusAccountStatus& aStatus,
        const nsAString& aCategory,
        const nsAString& aChangePassUrl,
        ErrorResult& aRv
    );

    RefPtr<MozPromise<void*, berytus::Failure, true>> PopulateMetadata();
};

}

#endif