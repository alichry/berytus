/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef DOM_BERYTUSX509EXTENSION_H_
#define DOM_BERYTUSX509EXTENSION_H_

#include "certt.h"
#include "mozilla/StaticString.h"
#include "nsIURI.h"
#include "nsPIDOMWindow.h"
#include "mozilla/AlreadyAddRefed.h"
#include "nsString.h"
#include "nsTArray.h"
#include "nsISupports.h"

namespace mozilla {
namespace berytus {

/**
 * Caller must provide non-empty input strings.
 */
nsresult CompareParts(const nsCString& aSearchString,
                      const nsCString& aCandidateString,
                      const char& aDelimiter,
                      const bool& aIgnoreTrailingWildcardIfCandidateConsumed,
                      bool& aRv);
/**
 * Examples:
 *
 * delimiter = '/'
 *
 * (1) input = "a/b/c"
 * (1) output = [ "a", "b", "c" ]
 *
 * (2) input = "/a/b/c/"
 * (2) output = [ "", "a", "b", "c", "" ]
 *
 * (3) input = "a/b///c"
 * (3) output = [ "a", "b", "", "", "c" ]
 */
class PartIterator {
public:
  PartIterator(const nsCString& aString,
               char aDelimiter = '.');
  ~PartIterator();
  const nsTDependentSubstring<char>& Current() const;
  nsresult Peek(nsTDependentSubstring<char>& aRv) const;
  nsresult Next();
  bool Finished() const;
protected:
  static const nsCString mEmptyPart;
  const nsCString& mString;
  char mDelimiter;
  uint32_t mPos;
  nsTDependentSubstring<char> mCurrent;

  nsresult Traverse(const uint32_t& aPos,
                    nsTDependentSubstring<char>& aSubRv,
                    uint32_t& aNextPosRv) const;
};
}

namespace dom {

class BerytusX509Extension final : public nsISupports {
public:
  NS_DECL_CYCLE_COLLECTING_ISUPPORTS
  NS_DECL_CYCLE_COLLECTION_CLASS(BerytusX509Extension)

  class SigningKeyEntry final : public nsISupports {
  public:
    NS_DECL_CYCLE_COLLECTING_ISUPPORTS
    NS_DECL_CYCLE_COLLECTION_CLASS(SigningKeyEntry)

    nsresult Matches(const nsCString& aSpki, nsIURI* aUrl, bool& aRv) const;

    class Url final : public nsISupports {
    public:
      NS_DECL_ISUPPORTS

      static already_AddRefed<Url> Create(const nsCString& aUrl, nsresult& aRv);
      nsresult Matches(nsIURI* aUrl, bool& aRv) const;

      explicit operator nsCString() const {
        if (mPort == -1 &&
            mHostname.Equals("*") &&
            mFilePath.Equals("/*")) {
          return nsCString("*"_ns);
        }
        nsCString out;
        out.Append(mScheme);
        out.Append("//");
        out.Append(mHostname);
        if (mPort != -1) {
          out.Append(":");
          out.AppendInt(mPort);
        }
        out.Append(mFilePath);
        return out;
      }
    protected:
      constexpr static const nsLiteralCString mScheme = "https"_ns;
      const nsCString mHostname;
      const int mPort;
      const nsCString mFilePath;

      Url(nsCString&& aHostname,
          const int& aPort,
          nsCString&& aFilePath);
      ~Url();
    };

    SigningKeyEntry(nsCString&& aSpki,
                    nsCString&& aSkSig,
                    RefPtr<Url>& aUrl);

    const nsCString& GetSpki();
    const nsCString& GetSkSig();
    RefPtr<const Url> GetUrl();

  protected:
    const nsCString mSpki;
    const nsCString mSkSig;
    RefPtr<Url> mUrl;
    ~SigningKeyEntry();
  };

  const nsTArray<RefPtr<SigningKeyEntry>>& GetAllowlist() const;

  nsresult IsAllowed(const nsCString& aSpki,
                     nsIURI* aUrl, bool& aRv) const;

  static already_AddRefed<BerytusX509Extension> Create(
      nsPIDOMWindowInner* aInner, nsresult& aRv);
  static already_AddRefed<BerytusX509Extension> Create(
      CERTCertificate* aCert, nsresult& aRv);
protected:
  BerytusX509Extension(nsTArray<RefPtr<SigningKeyEntry>>&& aAllowlist);
  ~BerytusX509Extension();
  nsTArray<RefPtr<SigningKeyEntry>> mAllowlist;

};
}
}

#endif
