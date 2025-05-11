/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_
#define DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_

#include "js/TypeDecls.h"
#include "mozilla/AlreadyAddRefed.h"
#include "mozilla/ErrorResult.h"
#include "mozilla/dom/BindingDeclarations.h"
#include "mozilla/dom/CryptoBuffer.h"
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsISupports.h"
#include "nsStringFwd.h"
#include "nsWrapperCache.h"
#include "nsIGlobalObject.h"

namespace mozilla::dom {

class BerytusChannel;

class BerytusKeyAgreementParameters final : public nsISupports /* or NonRefcountedDOMObject if this is a non-refcounted object */,
                                            public nsWrapperCache /* Change wrapperCache in the binding configuration if you don't want this */
{
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_WRAPPERCACHE_CLASS(BerytusKeyAgreementParameters)

  class SupportsToDictionary : public nsISupports {
  public:
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_SCRIPT_HOLDER_CLASS(SupportsToDictionary)

    void ToDictionary(JSContext* aCx,
                      JS::MutableHandle<JS::Value> aValue,
                      ErrorResult& aRv);
  protected:
    // Adapted from ReadableStream.h:
    // If one extends SupportsToDictionary with another cycle collectable class,
    // calling HoldJSObjects and DropJSObjects should happen using 'this' of
    // that extending class. And in that case Explicit should be passed to the
    // constructor of SupportsToDictionary so that it doesn't make those calls.
    // See also https://bugzilla.mozilla.org/show_bug.cgi?id=1801214.
    enum class HoldDropJSObjectsCaller { Implicit, Explicit };

    SupportsToDictionary(nsIGlobalObject* aGlobal,
                         HoldDropJSObjectsCaller aHoldDropCaller = HoldDropJSObjectsCaller::Implicit);
    virtual ~SupportsToDictionary();
    void ClearCachedDictionary();
    virtual void CacheDictionary(JSContext* aCx,
                                 ErrorResult& aRv) = 0;
    nsCOMPtr<nsIGlobalObject> mGlobal;
    JS::Heap<JSObject*> mCachedDictionary;
  private:
    HoldDropJSObjectsCaller mHoldDropCaller;
  }; // class SupportsToDictionary

  class Session final : public SupportsToDictionary {
  public:
    NS_DECL_ISUPPORTS_INHERITED
    NS_DECL_CYCLE_COLLECTION_CLASS_INHERITED(Session, SupportsToDictionary)

    class Fingerprint final : public SupportsToDictionary {
    public:
      NS_DECL_ISUPPORTS_INHERITED

      static already_AddRefed<Fingerprint> Create(
          const RefPtr<const BerytusChannel>& aChannel,
          const std::time_t& aTimestamp,
          ErrorResult& aRv);
      void CacheDictionary(JSContext* aCx,
                           ErrorResult& aRv) override;
      const nsLiteralString& GetHash() const;
      const nsLiteralString& GetVersion() const;
      const CryptoBuffer& GetSalt() const;
      const CryptoBuffer& GetDigest() const;
    protected:
      Fingerprint(nsIGlobalObject* aGlobal,
        CryptoBuffer&& aSalt,
        CryptoBuffer&& aDigest);
      ~Fingerprint();
      constexpr static nsLiteralString mHash = u"SHA-256"_ns;
      constexpr static nsLiteralString mVersion = u"v0.3"_ns;
      CryptoBuffer mSalt;
      CryptoBuffer mDigest;
    }; // class Fingerprint

    static already_AddRefed<Session> Create(
        const RefPtr<const BerytusChannel>& aChannel,
        nsTArray<nsString>&& aCiphertextUrls,
        ErrorResult& aRv);
    const nsString& GetID() const;
    const std::time_t& GetTimestamp() const;
    const RefPtr<Fingerprint>& GetFingerprint() const;
    Span<const nsString> GetCiphertextUrls() const;

    void CacheDictionary(JSContext* aCx,
                         ErrorResult& aRv) override;
  protected:
    Session(nsIGlobalObject* aGlobal,
            const nsAString& aId,
            const std::time_t& aTimestamp,
            const RefPtr<Fingerprint>& aFingerprint,
            nsTArray<nsString>&& aCiphertextUrls);
    ~Session();
    nsString mId;
    std::time_t mTimestamp;
    RefPtr<Fingerprint> mFingerprint;
    nsTArray<nsString> mCiphertextUrls;
  }; // class Session

  class Authentication final : public SupportsToDictionary {
  public:
    NS_DECL_ISUPPORTS_INHERITED

    Authentication(nsIGlobalObject* aGlobal,
                   const nsAString& aWebApp,
                   const nsAString& aScm);
    const nsLiteralString& GetName() const;
    const nsString& GetWebApp() const;
    const nsString& GetScm() const;
    void CacheDictionary(JSContext* aCx,
                         ErrorResult& aRv) override;
    protected:
      ~Authentication();
      constexpr static nsLiteralString mName = u"Ed25519"_ns; // WEBCRYPTO_ALG_ED25519
      nsString mWebApp;
      nsString mScm;
  }; // class Authentication

  class Exchange final : public SupportsToDictionary {
  public:
    NS_DECL_ISUPPORTS_INHERITED

    Exchange(nsIGlobalObject* aGlobal,
             const nsAString& aWebApp,
             const nsAString& aScm);
    const nsLiteralString& GetName() const;
    const nsString& GetWebApp() const;
    const nsString& GetScm() const;
    void CacheDictionary(JSContext* aCx,
                         ErrorResult& aRv) override;
  protected:
    ~Exchange();
    constexpr static nsLiteralString mName = u"X25519"_ns; // WEBCRYPTO_ALG_X25519
    nsString mWebApp;
    nsString mScm;
  }; // class Exchange

  class Derivation final : public SupportsToDictionary {
  public:
    NS_DECL_ISUPPORTS_INHERITED

    static already_AddRefed<Derivation> Create(
        nsIGlobalObject* aGlobal,
        ErrorResult& aRv);
    const nsLiteralString& GetName() const;
    const nsLiteralString& GetHash() const;
    const CryptoBuffer& GetSalt() const;
    const CryptoBuffer& GetInfo() const;
    void CacheDictionary(JSContext* aCx,
                         ErrorResult& aRv) override;
  protected:
    Derivation(nsIGlobalObject* aGlobal, CryptoBuffer&& aSalt);
    ~Derivation();
    constexpr static nsLiteralString mName = u"HKDF"_ns; //WEBCRYPTO_ALG_HKDF
    constexpr static nsLiteralString mHash = u"SHA-256"_ns; //WEBCRYPTO_ALG_SHA256
    CryptoBuffer mSalt;
    static CryptoBuffer mInfo;
  }; // class Derivation

  class Generation final : public SupportsToDictionary {
  public:
    NS_DECL_ISUPPORTS_INHERITED

    Generation(nsIGlobalObject* aGlobal);
    const nsLiteralString& GetName();
    const uint16_t& GetLength();
    void CacheDictionary(JSContext* aCx,
                         ErrorResult& aRv) override;
  protected:
    ~Generation();
    constexpr static nsLiteralString mName = u"AES-GCM"_ns; //WEBCRYPTO_ALG_AES_GCM
    constexpr static uint16_t mLength = 256;
  }; // class Generation
  
protected:
  BerytusKeyAgreementParameters(
    nsIGlobalObject* aGlobal,
    RefPtr<Session>& aSession,
    RefPtr<Authentication>& aAuthentication,
    RefPtr<Exchange>& aExchange,
    RefPtr<Derivation>& aDerivation,
    RefPtr<Generation>& aGeneration
  );
  ~BerytusKeyAgreementParameters();
  nsCOMPtr<nsIGlobalObject> mGlobal;
  RefPtr<Session> mSession;
  RefPtr<Authentication> mAuthentication;
  RefPtr<Exchange> mExchange;
  RefPtr<Derivation> mDerivation;
  RefPtr<Generation> mGeneration;

public:
  static already_AddRefed<BerytusKeyAgreementParameters> Create(
      const RefPtr<BerytusChannel>& aChannel,
      const nsAString& aExchangeWebApp,
      const nsAString& aExchangeScm,
      nsTArray<nsString>&& aCiphertextUrls,
      ErrorResult& aRv);
  // This should return something that eventually allows finding a
  // path to the global this object is associated with.  Most simply,
  // returning an actual global works.
  nsIGlobalObject* GetParentObject() const;

  JSObject* WrapObject(JSContext* aCx, JS::Handle<JSObject*> aGivenProto) override;

  RefPtr<const Session> GetSession() const;
  const RefPtr<Session>& GetSession();
  RefPtr<const Authentication> GetAuthentication() const;
  const RefPtr<Authentication>& GetAuthentication();
  RefPtr<const Exchange> GetExchange() const;
  const RefPtr<Exchange>& GetExchange();
  RefPtr<const Derivation> GetDerivation() const;
  const RefPtr<Derivation>& GetDerivation();
  RefPtr<const Generation> GetGeneration() const;
  const RefPtr<Generation>& GetGeneration();

  void GetSession(JSContext* aCx,
                  JS::MutableHandle<JS::Value> aValue,
                  ErrorResult& aRv);
  void GetAuthentication(JSContext* aCx,
                         JS::MutableHandle<JS::Value> aValue,
                         ErrorResult& aRv);
  void GetExchange(JSContext* aCx,
                   JS::MutableHandle<JS::Value> aValue,
                   ErrorResult& aRv);
  void GetDerivation(JSContext* aCx,
                     JS::MutableHandle<JS::Value> aValue,
                     ErrorResult& aRv);
  void GetGeneration(JSContext* aCx,
                     JS::MutableHandle<JS::Value> aValue,
                     ErrorResult& aRv);

  void ToCanonicalJSON(nsString& aJson, ErrorResult& aRv) const;
};

class JSONStructWriter {
  public:
  JSONStructWriter(nsString& aJson, ErrorResult& aRv);
  virtual ~JSONStructWriter();
  virtual bool Begin() = 0;
  virtual bool End() = 0;
protected:
  nsString& mJson;
  ErrorResult& mRv;
};

class JSONObjectWriter final : public JSONStructWriter {
public:
  JSONObjectWriter(nsString& aJson, ErrorResult& aRv);
  ~JSONObjectWriter();
  bool Begin() override;
  bool Key(const nsLiteralString& aKey);
  template <typename T>
  bool Value(const T& aValue);
  bool End() override;
protected:
  bool mEmpty;
};
class JSONArrayWriter final : public JSONStructWriter {
public:
  JSONArrayWriter(nsString& aJson, ErrorResult& aRv);
  ~JSONArrayWriter();
  bool Begin() override;
  template <typename T>
  bool Value(const T& aValue);
  bool End() override;
protected:
  bool mEmpty;
};

} // namespace mozilla::dom

#endif // DOM_BERYTUSKEYAGREEMENTPARAMETERS_H_
