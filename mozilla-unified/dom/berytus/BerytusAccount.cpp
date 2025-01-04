#include "mozilla/dom/BerytusAccount.h"
#include "BerytusUserAttributeMap.h"
#include "js/Realm.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/RefPtr.h"
#include "mozilla/berytus/AgentProxy.h"
#include "mozilla/berytus/AgentProxyUtils.h"
#include "mozilla/dom/BerytusAccountBinding.h"
#include "mozilla/dom/BerytusUserAttribute.h"
#include "mozilla/dom/Promise.h"

namespace mozilla::dom {

BerytusAccount::~BerytusAccount() {}

already_AddRefed<BerytusUserAttributeMap> BerytusAccount::UserAttributes()
    const {
  return do_AddRef(UserAttributeMap());
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<BerytusFieldMap> BerytusAccount::Fields() const {
  return do_AddRef(FieldMap());
}

RefPtr<berytus::AccountCreationAddFieldResult::AllPromiseType> BerytusAccount::AddFieldsSequential(
    JSContext* aCx,
    nsTArray<RefPtr<BerytusField>>&& aFields,
    nsTArray<RefPtr<berytus::AccountCreationAddFieldResult>>&& aPromises,
    const size_t& aIndex) {
  if (aIndex >= aFields.Length()) {
    return berytus::AccountCreationAddFieldResult::All(
      GetCurrentSerialEventTarget(), aPromises);
  }
  ErrorResult rv;
  auto promise = AddField(aCx, aFields.ElementAt(aIndex), rv);
  if (NS_WARN_IF(rv.Failed())) {
    return berytus::AccountCreationAddFieldResult::AllPromiseType::CreateAndReject(berytus::Failure(rv.StealNSResult()), __func__);
  }
  MOZ_ASSERT(promise);
  return promise->Then(GetCurrentSerialEventTarget(), __func__,
    [this, aCx, aFields = std::move(aFields), aPromises = std::move(aPromises), aIndex](berytus::AccountCreationAddFieldResult::ResolveValueType&& aValueProxy) mutable -> RefPtr<berytus::AccountCreationAddFieldResult::AllPromiseType> {
      JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
      aPromises.AppendElement(berytus::AccountCreationAddFieldResult::CreateAndResolve(std::move(aValueProxy), __func__));
      return AddFieldsSequential(aCx, std::move(aFields), std::move(aPromises), aIndex + 1);
    },
    [](const berytus::Failure& aFr) -> RefPtr<berytus::AccountCreationAddFieldResult::AllPromiseType> {
      return berytus::AccountCreationAddFieldResult::AllPromiseType::CreateAndReject(aFr, __func__);
    }
  );
}

RefPtr<berytus::AccountCreationAddFieldResult> BerytusAccount::AddField(JSContext* aCx, const RefPtr<BerytusField>& aField, ErrorResult& aRv) {
  BerytusChannel* channel = Channel();
  if (NS_WARN_IF(!channel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  const berytus::AgentProxy& agent = channel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  nsresult rv;
  berytus::RequestContextWithLoginOperation ctx;
  rv = berytus::Utils_RequestContextWithLoginOperationMetadata(
      GetParentObject(), Channel(), Operation(), this, ctx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  berytus::utils::FieldProxy proxyField;
  if (NS_WARN_IF(!berytus::utils::ToProxy::BerytusField(aCx, aField, proxyField))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  berytus::AddFieldArgs args(std::move(proxyField));
  return agent.AccountCreation_AddField(ctx, args);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccount::AddFields(
    JSContext* aCx, const Sequence<OwningNonNull<BerytusField>>& aFields,
    ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(GetParentObject(), aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  AddFieldsSequential(aCx, nsTArray<RefPtr<BerytusField>>(aFields.Elements(), aFields.Length()))
    ->Then(GetCurrentSerialEventTarget(), __func__,
      [this, aCx, aFields, outPromise](berytus::AccountCreationAddFieldResult::AllPromiseType::ResolveValueType&& aArray) {
        JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
        MOZ_ASSERT(aFields.Length() == aArray.Length());
        Record<nsString, RefPtr<BerytusField>> record;
        for (size_t i = 0; i < aFields.Length(); i++) {
          RefPtr<BerytusField> field = aFields.ElementAt(i);
          auto& valueProxy = aArray.ElementAt(i);
          ErrorResult rv;
          UpdateFieldValueFromProxy(aCx, field, std::move(valueProxy), rv);
          if (NS_WARN_IF(rv.Failed())) {
            outPromise->MaybeReject(std::move(rv));
            return;
          }
          Record<nsString, RefPtr<BerytusField>>::EntryType entry;
          field->GetId(entry.mKey);
          entry.mValue = field;
          record.Entries().AppendElement(std::move(entry));
          FieldMap()->AddField(field, rv);
          if (NS_WARN_IF(rv.Failed())) {
            outPromise->MaybeReject(std::move(rv));
            return;
          }
        }
        outPromise->MaybeResolve(record);
      },
      [outPromise](const berytus::Failure& aFr) {
        outPromise->MaybeReject(aFr.ToErrorResult());
      }
    );
  return outPromise.forget();
}

RefPtr<berytus::AccountCreationRejectFieldValueResult::AllPromiseType> BerytusAccount::RejectFieldsSequential(
    JSContext* aCx,
    nsTArray<BerytusFieldRejectionParameters>&& aParametersList,
    nsTArray<RefPtr<berytus::AccountCreationRejectFieldValueResult>>&& aPromises,
    const size_t& aIndex) {
  if (aIndex >= aParametersList.Length()) {
    return berytus::AccountCreationRejectFieldValueResult::All(
      GetCurrentSerialEventTarget(), aPromises);
  }
  ErrorResult rv;
  auto promise = RejectField(aCx, aParametersList.ElementAt(aIndex), rv);
  if (NS_WARN_IF(rv.Failed())) {
    return berytus::AccountCreationRejectFieldValueResult::AllPromiseType::CreateAndReject(berytus::Failure(rv.StealNSResult()), __func__);
  }
  MOZ_ASSERT(promise);
  return promise->Then(GetCurrentSerialEventTarget(), __func__,
    [this, aCx, aParametersList = std::move(aParametersList), aPromises = std::move(aPromises), aIndex](berytus::AccountCreationRejectFieldValueResult::ResolveValueType&& aFieldValueProxy) mutable -> RefPtr<berytus::AccountCreationRejectFieldValueResult::AllPromiseType> {
      JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
      aPromises.AppendElement(berytus::AccountCreationRejectFieldValueResult::CreateAndResolve(std::move(aFieldValueProxy), __func__));
      return RejectFieldsSequential(aCx, std::move(aParametersList), std::move(aPromises), aIndex + 1);
    },
    [](const berytus::Failure& aFr) -> RefPtr<berytus::AccountCreationRejectFieldValueResult::AllPromiseType> {
      return berytus::AccountCreationRejectFieldValueResult::AllPromiseType::CreateAndReject(aFr, __func__);
    }
  );
}

RefPtr<berytus::AccountCreationRejectFieldValueResult> BerytusAccount::RejectField(
    JSContext* aCx,
    const BerytusFieldRejectionParameters& aParameters,
    ErrorResult& aRv) {
  BerytusChannel* channel = Channel();
  if (NS_WARN_IF(!channel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  const berytus::AgentProxy& agent = channel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  nsresult rv;
  berytus::RequestContextWithLoginOperation ctx;
  rv = berytus::Utils_RequestContextWithLoginOperationMetadata(
      GetParentObject(), Channel(), Operation(), this, ctx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  berytus::RejectFieldValueArgs args;
  RefPtr<BerytusField> field;
  if (aParameters.mField.IsBerytusField()) {
    field = aParameters.mField.GetAsBerytusField();
  } else {
    field = FieldMap()->GetField(aParameters.mField.GetAsString(), aRv);
    if (NS_WARN_IF(aRv.Failed())) {
      return nullptr;
    }
  }
  field->GetId(args.mFieldId);
  args.mReason.mCode.Assign(aParameters.mReason);
  if (NS_WARN_IF(!berytus::utils::ToProxy::BerytusOptionalFieldValueUnion(
      aCx,
      !aParameters.mNewValue.WasPassed()
        ? berytus::utils::FieldValueUnion()
        : berytus::utils::FieldValueUnion(aParameters.mNewValue.Value()),
      args.mOptionalNewValue))) {
    aRv.Throw(NS_ERROR_FAILURE);
    return nullptr;
  }
  return agent.AccountCreation_RejectFieldValue(ctx, args);
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccount::RejectAndReviseFields(
    JSContext* aCx,
    const Sequence<BerytusFieldRejectionParameters>& aRejectionParameters,
    ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(GetParentObject(), aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  BerytusChannel* channel = Channel();
  if (NS_WARN_IF(!channel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  const berytus::AgentProxy& agent = channel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  nsTArray<BerytusFieldRejectionParameters> clonedParamsList;
  if (NS_WARN_IF(!CloneRejectionParametersList(aCx, aRejectionParameters, clonedParamsList))) {
    aRv.ThrowInvalidStateError("Out of memory");
    return nullptr;
  }
  nsTArray<RefPtr<BerytusField>> fields;
  for (const auto& params : aRejectionParameters) {
    RefPtr<BerytusField> field;
    if (params.mField.IsBerytusField()) {
      field = params.mField.GetAsBerytusField();
    } else {
      field = FieldMap()->GetField(params.mField.GetAsString(), aRv);
      if (NS_WARN_IF(aRv.Failed())) {
        return nullptr;
      }
    }
    fields.AppendElement(field);
  }
  RejectFieldsSequential(aCx, std::move(clonedParamsList))
    ->Then(GetCurrentSerialEventTarget(), __func__,
      [outPromise, this, fields = std::move(fields), aCx](nsTArray<berytus::AccountCreationRejectFieldValueResult::ResolveValueType>&& aValues) {
        MOZ_ASSERT(aValues.Length() == fields.Length());
        Record<nsString, RefPtr<BerytusField>> record;
        for (size_t i = 0; i < fields.Length(); i++) {
          auto field = fields.ElementAt(i);
          JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
          ErrorResult rv;
          UpdateFieldValueFromProxy(aCx, field, std::move(aValues.ElementAt(i)), rv);
          if (NS_WARN_IF(rv.Failed())) {
            outPromise->MaybeReject(std::move(rv));
            return;
          }
          Record<nsString, RefPtr<BerytusField>>::EntryType entry;
          field->GetId(entry.mKey);
          entry.mValue = field;
          record.Entries().AppendElement(std::move(entry));
        }
        outPromise->MaybeResolve(record);
      },
      [outPromise](const berytus::Failure& aFr) {
        outPromise->MaybeReject(aFr.ToErrorResult());
      }
    );
  return outPromise.forget();
}

void BerytusAccount::UpdateFieldValueFromProxy(
    JSContext* aCx,
    RefPtr<BerytusField>& aField,
    berytus::utils::FieldValueUnionProxy&& aValueProxy,
    ErrorResult& aRv) {
  nsresult rv;
  Nullable<BerytusField::ValueUnion> value;
  rv = berytus::utils::FromProxy::BerytusFieldValueUnion(GetParentObject(), aCx, berytus::utils::FieldValueUnionProxy(std::move(aValueProxy)), value);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return;
  }
  MOZ_ASSERT(!value.IsNull()); // TODO(berytus): Remove Nullable from FromProxy::BerytusFieldValueUnion
  aField->SetValue(aCx, value, aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return;
  }
}

bool BerytusAccount::CloneRejectionParametersList(
    JSContext* aCx,
    const Sequence<BerytusFieldRejectionParameters>& aSrcList,
    nsTArray<BerytusFieldRejectionParameters>& aDstList) {
  for (const auto& params : aSrcList) {
    JS::Rooted<JS::Value> val(aCx);
    if (NS_WARN_IF(!params.ToObjectInternal(aCx, &val))) {
      return false;
    }
    RootedDictionary<BerytusFieldRejectionParameters> newParams(aCx);
    if (NS_WARN_IF(!newParams.Init(aCx, val))) {
      return false;
    }
    aDstList.AppendElement(std::move(newParams));
  }
  return true;
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccount::SetUserAttributes(
    JSContext* aCx,
    const Sequence<BerytusUserAttributeDefinition>& aAttributes,
    ErrorResult& aRv) {
  RefPtr<Promise> outPromise = Promise::Create(GetParentObject(), aRv);
  if (NS_WARN_IF(aRv.Failed())) {
    return nullptr;
  }
  BerytusChannel* channel = Channel();
  if (NS_WARN_IF(!channel->Active())) {
    aRv.ThrowInvalidStateError("Channel no longer active");
    return nullptr;
  }
  const berytus::AgentProxy& agent = channel->Agent();
  MOZ_ASSERT(!agent.IsDisabled());
  nsresult rv;
  berytus::RequestContextWithOperation ctx;
  rv = berytus::Utils_RequestContextWithOperationMetadata(
      GetParentObject(), Channel(), Operation(), ctx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv.Throw(rv);
    return nullptr;
  }
  berytus::UpdateUserAttributesArgs args;
  for (const auto& attr : aAttributes) {
    berytus::UserAttribute attrProxy;
    if (NS_WARN_IF(!berytus::utils::ToProxy::BerytusUserAttributeDefinition(aCx, attr, attrProxy))) {
      aRv.Throw(NS_ERROR_FAILURE);
      return nullptr;
    }
    args.mUserAttributes.AppendElement(std::move(attrProxy));
  }
  agent.AccountCreation_UpdateUserAttributes(ctx, args)
    ->Then(GetCurrentSerialEventTarget(), __func__,
      [this, aCx, outPromise, args = std::move(args)](const berytus::AccountCreationUpdateUserAttributesResult::ResolveValueType& aVal) {
        JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
        nsresult res;
        // aAttributes cannot be copied since BerytusUserAttributeDefinition
        // cannot be copied. We used args instead as a workaround.
        for (const auto& attrDef : args.mUserAttributes) {
          nsString id(attrDef.mId);
          RefPtr<BerytusUserAttribute> attr = UserAttributeMap()->GetAttribute(id);
          BerytusUserAttribute::SourceValueType val;
          res = berytus::utils::FromProxy::BerytusUserAttributeValue(GetParentObject(), attrDef.mValue, val);
          if (NS_WARN_IF(NS_FAILED(res))) {
            outPromise->MaybeReject(res);
            return;
          }
          if (attr) {
            if (attr->CanSetValue(val)) {
              if (NS_WARN_IF(!attr->SetValue(aCx, val))) {
                outPromise->MaybeReject(NS_ERROR_FAILURE);
                return;
              }
              continue;
            }
            ErrorResult rv;
            UserAttributeMap()->RemoveAttribute(id, rv);
            if (NS_WARN_IF(rv.Failed())) {
              outPromise->MaybeReject(std::move(rv));
              return;
            }
            attr = nullptr;
          }
          attr = BerytusUserAttribute::Create(
              aCx,
              GetParentObject(),
              id,
              attrDef.mMimeType.isSome() ? nsString(attrDef.mMimeType.ref()) : nsString(),
              attrDef.mInfo.isSome() ? nsString(attrDef.mInfo.ref()) : nsString(),
              val,
              res);
          if (NS_WARN_IF(NS_FAILED(res))) {
            outPromise->MaybeReject(res);
            return;
          }
          ErrorResult rv;
          UserAttributeMap()->AddAttribute(attr, rv);
          if (NS_WARN_IF(rv.Failed())) {
            outPromise->MaybeReject(std::move(rv));
            return;
          }
        }
        outPromise->MaybeResolveWithUndefined();
      },
      [outPromise](const berytus::Failure& aFr) {
        outPromise->MaybeReject(aFr.ToErrorResult());
      }
    );
  return outPromise.forget();
}

// Return a raw pointer here to avoid refcounting, but make sure it's safe (the
// object should be kept alive by the callee).
already_AddRefed<Promise> BerytusAccount::AddFieldCategory(
    const nsAString& aId, const nsAString& aInfo, ErrorResult& aRv) {
  MOZ_ASSERT(false, "BerytusAccount::AddFieldCategory");
  aRv.Throw(NS_ERROR_FAILURE);
  return nullptr;
}


RefPtr<MozPromise<void*, berytus::Failure, true>> BerytusAccount::PopulateUserAttributeMap(JSContext* aCx) {
  nsresult rv;

  berytus::RequestContextWithOperation reqCtx;
  rv = berytus::Utils_RequestContextWithOperationMetadata(GetParentObject(), Channel(), Operation(), reqCtx);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    RefPtr<MozPromise<void*, berytus::Failure, true>::Private> nextProm;
    nextProm->Reject(berytus::Failure(rv), __func__);
    return nextProm;
  }
  RefPtr<berytus::AccountCreationGetUserAttributesResult> res =
    Channel()->Agent().AccountCreation_GetUserAttributes(reqCtx);
  return res->Then(
    GetCurrentSerialEventTarget(), __func__,
    [this, aCx](const nsTArray<mozilla::berytus::UserAttribute>& attrs) -> RefPtr<MozPromise<void*, berytus::Failure, true>> {
      auto *map = UserAttributeMap();
      JSAutoRealm ar(aCx, GetParentObject()->GetGlobalJSObject());
      for (const auto& attr: attrs) {
        nsresult res;
        BerytusUserAttribute::SourceValueType value;
        res = berytus::utils::FromProxy::BerytusUserAttributeValue(GetParentObject(), attr.mValue, value);
        if (NS_WARN_IF(NS_FAILED(res))) {
          return MozPromise<void*, berytus::Failure, true>::CreateAndReject(berytus::Failure(res), __func__);
        }
        RefPtr<BerytusUserAttribute> newAttr = BerytusUserAttribute::Create(
          aCx,
          GetParentObject(),
          attr.mId,
          attr.mMimeType.isSome() ? attr.mMimeType.ref() : nsString(),
          attr.mInfo.isSome() ? attr.mInfo.ref() : nsString(),
          value,
          res
        );
        if (NS_WARN_IF(NS_FAILED(res))) {
          return MozPromise<void*, berytus::Failure, true>::CreateAndReject(berytus::Failure(res), __func__);
        }
        ErrorResult rv;
        map->AddAttribute(newAttr, rv);
        if (NS_WARN_IF(rv.Failed())) {
          berytus::Failure fr(rv.StealNSResult());
          return MozPromise<void*, berytus::Failure, true>::CreateAndReject(fr, __func__);
        }
      }

      void* out = nullptr;
      return MozPromise<void*, berytus::Failure, true>::CreateAndResolve(out, __func__);
    },
    [](const berytus::Failure& aFr) -> RefPtr<MozPromise<void*, berytus::Failure, true>> {
      RefPtr<MozPromise<void*, berytus::Failure, true>::Private> nextProm =
        new MozPromise<void*, berytus::Failure, true>::Private(__func__);
      nextProm->Reject(aFr, __func__);
      return nextProm;
    }
  );
}

}  // namespace mozilla::dom