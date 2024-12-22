#include "BerytusAccountMetadata.h"
#include "js/Value.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountBinding.h"
#include "mozilla/dom/BerytusWebAppActor.h"
#include "mozilla/dom/Promise.h"

namespace mozilla::dom {

BerytusAccountMetadata::BerytusAccountMetadata()
    : mStatus(BerytusAccountStatus::Pending), mVersion(0) {}

BerytusAccountMetadata::~BerytusAccountMetadata() {}

RefPtr<berytus::LoginUpdateMetadataResult>
BerytusAccountMetadata::UpdateMetadata(
  const uint64_t& aVersion,
  const BerytusAccountStatus& aStatus,
  const nsAString& aCategory,
  const nsAString& aChangePassUrl,
  ErrorResult& aRv
) {
  berytus::RequestContextWithOperation ctx;
  nsresult rv = berytus::Utils_RequestContextWithOperationMetadata(
      GetParentObject(), Channel(), Operation(), ctx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }

  berytus::UpdateMetadataArgs args(
    berytus::RecordMetadata(
      static_cast<double>(aVersion),
      aStatus == BerytusAccountStatus::Created
      ? berytus::EMetadataStatus::Created()
      : aStatus == BerytusAccountStatus::Pending
      ? berytus::EMetadataStatus::Pending()
      : berytus::EMetadataStatus::Retired(),
      nsString(aCategory),
      nsString(aChangePassUrl)
    )
  );
  const berytus::AgentProxy& agent = Channel()->Agent();
  if (NS_WARN_IF(agent.IsDisabled())) {
    return nullptr;
  }
  return agent.Login_UpdateMetadata(ctx, args)
    ->Then(
      GetCurrentSerialEventTarget(), __func__,
      [this, aVersion, aStatus, category = nsString(aCategory), changePassUrl = nsString(aChangePassUrl)]() {
        mVersion = aVersion;
        mStatus = aStatus;
        mCategory.Assign(category);
        mChangePassUrl.Assign(changePassUrl);

        void* out = nullptr;
        return berytus::LoginUpdateMetadataResult::CreateAndResolve(out, __func__);
      },
      [](const berytus::Failure& aFr) {
        return berytus::LoginUpdateMetadataResult::CreateAndReject(aFr, __func__);
      }
    );
}

already_AddRefed<Promise> BerytusAccountMetadata::UpdateMetadataPromise(
  const uint64_t& aVersion,
  const BerytusAccountStatus& aStatus,
  const nsAString& aCategory,
  const nsAString& aChangePassUrl,
  ErrorResult& aRv
) {
  RefPtr<Promise> outPromise = Promise::Create(GetParentObject(), aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  RefPtr<berytus::LoginUpdateMetadataResult> pr = UpdateMetadata(aVersion, aStatus, aCategory, aChangePassUrl, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  pr->Then(
    GetCurrentSerialEventTarget(), __func__,
    [outPromise]() {
      outPromise->MaybeResolve(JS::UndefinedHandleValue);
    },
    [outPromise](const berytus::Failure& aFr) {
      outPromise->MaybeReject(aFr.ToErrorResult());
    }
  );
  return outPromise.forget();
}

void BerytusAccountMetadata::GetCategory(nsString& aRetVal) const {
  aRetVal.Assign(mCategory);
}

uint64_t BerytusAccountMetadata::Version() const { return mVersion; }

BerytusAccountStatus BerytusAccountMetadata::Status() const { return mStatus; }

void BerytusAccountMetadata::GetChangePasswordUrl(nsString& aRetVal) const {
  aRetVal.Assign(mChangePassUrl);
}

already_AddRefed<Promise> BerytusAccountMetadata::SetVersion(uint64_t aVersion,
                                                             ErrorResult& aRv) {
  return UpdateMetadataPromise(aVersion, mStatus, mCategory, mChangePassUrl, aRv);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetCategory(
    const nsAString& aCategory, ErrorResult& aRv) {
  return UpdateMetadataPromise(mVersion, mStatus, aCategory, mChangePassUrl, aRv);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetStatus(
    BerytusAccountStatus aStatus, ErrorResult& aRv) {
  return UpdateMetadataPromise(mVersion, aStatus, mCategory, mChangePassUrl, aRv);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccountMetadata::SetChangePasswordUrl(
    const nsAString& aUrl, ErrorResult& aRv) {
  return UpdateMetadataPromise(mVersion, mStatus, mCategory, aUrl, aRv);
}

}  // namespace mozilla::dom