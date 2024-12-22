#ifndef DOM_BERYTUSACCOUNT_H_
#define DOM_BERYTUSACCOUNT_H_

#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BerytusChannel.h"
#include "mozilla/dom/BerytusAccountBinding.h" // for BerytusAccountStatus
#include "mozilla/dom/BerytusLoginOperation.h"
#include "mozilla/dom/Record.h"
#include "nsIGlobalObject.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/dom/BerytusField.h"
#include "mozilla/dom/BerytusFieldMap.h"
#include "mozilla/dom/BerytusUserAttributeMap.h"
#include "mozilla/berytus/AgentProxyUtils.h"

namespace mozilla::dom {

struct BerytusUserAttributeDefinition;
struct BerytusFieldRejectionParameters;

class BerytusAccount {
protected:
  virtual ~BerytusAccount();
  virtual nsIGlobalObject* GetParentObject() const = 0;
  virtual BerytusChannel* Channel() const = 0;
  virtual BerytusLoginOperation* Operation() = 0;
  virtual BerytusFieldMap* FieldMap() const = 0;
  virtual BerytusUserAttributeMap* UserAttributeMap() const = 0;

  RefPtr<MozPromise<void*, berytus::Failure, true>> PopulateUserAttributeMap(JSContext* aCx);

  RefPtr<berytus::AccountCreationAddFieldResult::AllPromiseType> AddFieldsSequential(JSContext* aCx, nsTArray<RefPtr<BerytusField>>&& aFields, nsTArray<RefPtr<berytus::AccountCreationAddFieldResult>>&& aPromises = nsTArray<RefPtr<berytus::AccountCreationAddFieldResult>>(), const size_t& aIndex = 0);
  RefPtr<berytus::AccountCreationAddFieldResult> AddField(JSContext* aCx, const RefPtr<BerytusField>& aField, ErrorResult& aRv);
  RefPtr<berytus::AccountCreationRejectFieldValueResult::AllPromiseType> RejectFieldsSequential(JSContext* aCx, nsTArray<BerytusFieldRejectionParameters>&& aParametersList, nsTArray<RefPtr<berytus::AccountCreationRejectFieldValueResult>>&& aPromises = nsTArray<RefPtr<berytus::AccountCreationRejectFieldValueResult>>(), const size_t& aIndex = 0);
  RefPtr<berytus::AccountCreationRejectFieldValueResult> RejectField(JSContext* aCx, const BerytusFieldRejectionParameters& aParameters, ErrorResult& aRv);
  bool CloneRejectionParametersList(JSContext* aCx, const Sequence<BerytusFieldRejectionParameters>& aSrcList, nsTArray<BerytusFieldRejectionParameters>& aDstList);
private:
  void UpdateFieldValueFromProxy(
      JSContext* aCx,
      RefPtr<BerytusField>& aField,
      berytus::utils::FieldValueUnionProxy&& aValueProxy,
      ErrorResult& aRv);
public:
  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusUserAttributeMap> UserAttributes() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<BerytusFieldMap> Fields() const;

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> AddFields(
    JSContext *aCx,
    const Sequence<OwningNonNull<BerytusField>>& aFields,
    ErrorResult& aRv
  );

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> RejectAndReviseFields(
    JSContext* aCx,
    const Sequence<BerytusFieldRejectionParameters>& aRejectionParameters,
    ErrorResult& aRv
  );

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> SetUserAttributes(
    JSContext* aCx,
    const Sequence<BerytusUserAttributeDefinition>& aAttributes,
    ErrorResult& aRv
  );

  // Return a raw pointer here to avoid refcounting, but make sure it's safe (the object should be kept alive by the callee).
  already_AddRefed<Promise> AddFieldCategory(
    const nsAString& aId,
    const nsAString& aInfo,
    ErrorResult& aRv
  );

};

}

#endif