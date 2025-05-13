/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* vim:set ts=2 sw=2 sts=2 et cindent: */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/BerytusX509Extension.h"
#include "ErrorList.h"
#include "cert.h"
#include "certt.h"
#include "mozilla/Assertions.h"
#include "mozilla/Components.h"
#include "nsCOMPtr.h"
#include "nsCycleCollectionParticipant.h"
#include "nsDebug.h"
#include "nsISupports.h"
#include "mozilla/dom/Document.h"
#include "nsIURI.h"
#include "nsIX509Cert.h"
#include "nsStringFwd.h"
#include "nsTArray.h"
#include "seccomon.h"

static mozilla::LazyLogModule sLogger("berytus_x509");

namespace mozilla::berytus {

const nsCString PartIterator::mEmptyPart = nsCString();

PartIterator::PartIterator(const nsCString& aString,
                           char aDelimiter) : mString(aString),
                                              mDelimiter(aDelimiter),
                                              mPos(0) {}
PartIterator::~PartIterator() {}

const nsTDependentSubstring<char>& PartIterator::Current() const {
  return mCurrent;
}
nsresult PartIterator::Peek(nsTDependentSubstring<char>& aRv) const {
  uint32_t dummy;
  return Traverse(uint32_t(mPos), aRv, dummy);
}
nsresult PartIterator::Next() {
  return Traverse(uint32_t(mPos), mCurrent, mPos);
}
bool PartIterator::Finished() const {
  return mPos >= mString.Length();
}
nsresult PartIterator::Traverse(const uint32_t& aPos,
                                nsTDependentSubstring<char>& aSubRv,
                                uint32_t& aNextPosRv) const {
  if (aPos >= mString.Length()) {
    return NS_ERROR_FAILURE;
  }
  uint32_t start = aPos;
  uint32_t end = start;
  if (mString.CharAt(start) == mDelimiter) {
    aSubRv.Rebind(mEmptyPart, 0);
    aNextPosRv = start + 1;
    return NS_OK;
  }
  bool reachedLastChar, peekedDelimiter;
  while (!(reachedLastChar = end + 1 >= mString.Length()) &&
         !(peekedDelimiter = mString.CharAt(end + 1) == mDelimiter)) {
    end++;
    peekedDelimiter = false;
  }
  aSubRv.Rebind(mString, start, end - start + 1);
  if (peekedDelimiter) {
    MOZ_ASSERT(!reachedLastChar);
    // the peeked delimiter could be at the end of the string:
    aNextPosRv = std::min(end + 2ul, mString.Length() - 1ul);
    MOZ_ASSERT(aNextPosRv < mString.Length());
    return NS_OK;
  }
  MOZ_ASSERT(reachedLastChar);
  MOZ_ASSERT(mString.CharAt(end) != mDelimiter);
  aNextPosRv = end + 1;
  MOZ_ASSERT(aNextPosRv == mString.Length());
  return NS_OK;
}

nsresult CompareParts(const nsCString& aSearchString,
                      const nsCString& aCandidateString,
                      const char& aDelimiter,
                      const bool& aIgnoreTrailingWildcardIfCandidateConsumed,
                      bool& aRv) {
  nsresult res;
  if (NS_WARN_IF(aSearchString.IsEmpty())) {
    return NS_ERROR_FAILURE;
  }
  if (NS_WARN_IF(aCandidateString.IsEmpty())) {
    return NS_ERROR_FAILURE;
  }
  if (aSearchString.Equals("*")) {
    // short-circuit
    aRv = true;
    return NS_OK;
  }
  PartIterator searchItr(aSearchString, aDelimiter);
  PartIterator candidateItr(aCandidateString, aDelimiter);
  bool freezeSearch = false;
  while (!candidateItr.Finished()) {
    res = candidateItr.Next();
    NS_ENSURE_SUCCESS(res, res);
    if (!freezeSearch) {
      if (searchItr.Finished()) {
        // there's not enough parts in search string
        aRv = false;
        return NS_OK;
      }
      res = searchItr.Next();
      NS_ENSURE_SUCCESS(res, res);
    }
    const auto& searchPart = searchItr.Current();
    const auto& candidatePart = candidateItr.Current();
    if (searchPart.Equals(candidatePart)) {
      continue;
    }
    if (!searchPart.Equals("*")) {
      aRv = false;
      return NS_OK;
    }
    if (searchItr.Finished()) {
      // e.g. [0] = "/abc/de/*"
      //                     ^ we are here
      //      [1] = "/abc/de/da/da/ta"
      //                     ^ we are here
      // we short-circuit the process and return success
      aRv = true;
      return NS_OK;
    }
    if (!freezeSearch) {
      // first time encountering wildcard part
      // e.g. [0] = "/abc/de/*/"
      //                     ^ we are here
      //      [1] = "/abc/de/da/ta"
      //                     ^ we are here
      freezeSearch = true;
      continue;
    }
    MOZ_ASSERT(!searchItr.Finished());
    nsTDependentSubstring<char> nextSearchPart;
    res = searchItr.Peek(nextSearchPart);
    NS_ENSURE_SUCCESS(res, res);
    if (nextSearchPart.Equals("*")) {
      // two wildcards in a row.
      // e.g. [0] = "/abc/def/*/*/jkl"
      //                      ^ we are here
      //                        ^ next search part
      //      [1] = "/abc/def/123/456/ghi/jkl"
      //                          ^ we are here
      // simply move search itr forward without unfreezing
      res = searchItr.Next();
      NS_ENSURE_SUCCESS(res, res);
      MOZ_ASSERT(nextSearchPart.Equals(searchItr.Current()));
      continue;
    }
    if (!nextSearchPart.Equals(candidatePart)) {
      // e.g. [0] = "/abc/def/*/ghi/jkl"
      //                      ^ we are here
      //                        ^ next search part
      //      [1] = "/abc/def/123/456/ghi/jkl"
      //                          ^ we are here
      continue;
    }
    // e.g. [0] = "/abc/def/*/ghi/jkl"
    //                      ^ we are here
    //                        ^ next search part
    //      [1] = "/abc/def/0123/0456/ghi/jkl"
    //                                ^ we are here
    freezeSearch = false;
    res = searchItr.Next();
    NS_ENSURE_SUCCESS(res, res);
    MOZ_ASSERT(nextSearchPart.Equals(searchItr.Current()));
  }
  MOZ_ASSERT(candidateItr.Finished());
  if (!searchItr.Finished()) {
    // e.g. [0] = "/abc/def/*/ghi"
    //                      ^ we are here (unfinished)
    //      or even:
    //      [0] = "/abc/def/*/"
    //                      ^ we are here (unfinished)
    //      [1] = "/abc/def/123/456/xyz"
    //                                 ^ we are here (finished)
    if (aIgnoreTrailingWildcardIfCandidateConsumed) {
      res = searchItr.Next();
      NS_ENSURE_SUCCESS(res, res);
      if (searchItr.Finished() && searchItr.Current().Equals("*")) {
        // e.g. [0] = "/app/a/*"
        //                    ^ we are here (finished)
        //      [1] = "/abc/a"
        //                  ^ we are here (finished)
        aRv = true;
        return NS_OK;
      }
      // NOTE(berytus): The below instance would fail, even if
      // aIgnoreTrailingWildcardIfCandidateConsumed was set:
      //      [0] = "/app/a/*/*"
      //      [1] = "/app/a"
    }
    aRv = false;
    return NS_OK;
  }
  aRv = true;
  return NS_OK;
}

NS_IMPL_ISUPPORTS0(UrlSearchExpression)

UrlSearchExpression::UrlSearchExpression(
    nsCString&& aHostname, const int& aPort, nsCString&& aFilePath)
    : mHostname(aHostname), mPort(aPort), mFilePath(aFilePath) {}

UrlSearchExpression::~UrlSearchExpression() {}

nsresult UrlSearchExpression::Matches(nsIURI* aUrl,
                                      bool& aRv) const {
  MOZ_ASSERT(aUrl);
  auto logAUrl = aUrl->GetSpecOrDefault();
  auto logMUrl = nsCString(*this);

  nsCString candidateScheme, candidateHost, candidateFilePath;
  int32_t candidatePort;
  nsresult res;
  res = aUrl->GetScheme(candidateScheme);
  if (NS_WARN_IF(NS_FAILED(res))) {
    return res;
  }
  res = aUrl->GetHost(candidateHost);
  if (NS_WARN_IF(NS_FAILED(res))) {
    return res;
  }
  res = aUrl->GetFilePath(candidateFilePath);
  if (NS_WARN_IF(NS_FAILED(res))) {
    return res;
  }
  res = aUrl->GetPort(&candidatePort);
  if (NS_WARN_IF(NS_FAILED(res))) {
    return res;
  }
  if (!candidateScheme.Equals(mScheme)) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("UrlSearchExpression::Matches(): "
             "NoMatch[Scheme(%.*s, %.*s)] "
             "aUrl (%.*s) against mUrl (%.*s)\n",
             (int) candidateScheme.Length(), candidateScheme.Data(),
             (int) mScheme.Length(), mScheme.Data(),
             (int) logAUrl.Length(), logAUrl.Data(),
             (int) logMUrl.Length(), logMUrl.Data()));
    aRv = false;
    return NS_OK;
  }
  if (candidatePort != mPort) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("UrlSearchExpression::Matches(): "
             "NoMatch[Port(%d, %d)] "
             "aUrl (%.*s) against mUrl (%.*s)\n",
             candidatePort,
             mPort,
             (int) logAUrl.Length(), logAUrl.Data(),
             (int) logMUrl.Length(), logMUrl.Data()));
    aRv = false;
    return NS_OK;
  }
  bool matched;
  res = berytus::CompareParts(mHostname, candidateHost, '.', false, matched);
  NS_ENSURE_SUCCESS(res, res);
  if (!matched) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("SigningKeyEntry::Url::Matches(): "
             "NoMatch[Hostname(%.*s, %.*s)] "
             "aUrl (%.*s) against mUrl (%.*s)\n",
             (int) candidateHost.Length(), candidateHost.Data(),
             (int) mHostname.Length(), mHostname.Data(),
             (int) logAUrl.Length(), logAUrl.Data(),
             (int) logMUrl.Length(), logMUrl.Data()));
    aRv = false;
    return NS_OK;
  }
  res = berytus::CompareParts(mFilePath, candidateFilePath, '/', true, matched);
  NS_ENSURE_SUCCESS(res, res);
  if (!matched) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("UrlSearchExpression::Matches(): "
             "NoMatch[FilePath(%.*s, %.*s)] "
             "aUrl (%.*s) against mUrl (%.*s)\n",
             (int) candidateFilePath.Length(), candidateFilePath.Data(),
             (int) mFilePath.Length(), mFilePath.Data(),
             (int) logAUrl.Length(), logAUrl.Data(),
             (int) logMUrl.Length(), logMUrl.Data()));
    aRv = false;
    return NS_OK;
  }
  MOZ_LOG(sLogger, LogLevel::Info,
          ("UrlSearchExpression::Matches(): "
           "Matched aUrl (%.*s) against mUrl (%.*s)\n",
           (int) logAUrl.Length(), logAUrl.Data(),
           (int) logMUrl.Length(), logMUrl.Data()));
  aRv = true;
  return NS_OK;
}

already_AddRefed<UrlSearchExpression>
UrlSearchExpression::Create(const nsCString& aUrl,
                                                   nsresult& aRv) {
  nsCOMPtr<nsIIOService> io = mozilla::components::IO::Service();
  if (NS_WARN_IF(!io)) {
    aRv = NS_ERROR_FAILURE;
    return nullptr;
  }
  if (aUrl.Equals("*")) {
    aRv = NS_OK;
    return do_AddRef(new UrlSearchExpression(nsCString("*"_ns), -1, nsCString("/*"_ns)));
  }
  nsCOMPtr<nsIURI> parsedUri;
  aRv = io->NewURI(aUrl, nullptr, nullptr,
                   getter_AddRefs(parsedUri));
  NS_ENSURE_SUCCESS(aRv, nullptr);

  MOZ_ASSERT(parsedUri);
  if (!parsedUri->SchemeIs("https")) {
    aRv = NS_ERROR_INVALID_ARG;
    return nullptr;
  }
  bool tmp;
  parsedUri->GetHasUserPass(&tmp);
  if (tmp) {
    aRv = NS_ERROR_INVALID_ARG;
    return nullptr;
  }
  parsedUri->GetHasQuery(&tmp);
  if (tmp) {
    aRv = NS_ERROR_INVALID_ARG;
    return nullptr;
  }
  parsedUri->GetHasRef(&tmp);
  if (tmp) {
    aRv = NS_ERROR_INVALID_ARG;
    return nullptr;
  }
  nsCString host, filePath;
  int32_t port;
  aRv = parsedUri->GetHost(host);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  aRv = parsedUri->GetFilePath(filePath);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  aRv = parsedUri->GetPort(&port);
  if (NS_WARN_IF(NS_FAILED(aRv))) {
    return nullptr;
  }
  aRv = NS_OK;
  return do_AddRef(new UrlSearchExpression(std::move(host), port, std::move(filePath)));
}


} // namespace mozilla::berytus

namespace mozilla::dom {

static unsigned char sBerytusX509ExtensionOID[] = {
  0x2A, 0x03, 0x04, 0x16, 0x0B, 0x17 };

struct RawEntry {
  SECItem mKey;
  SECItem mSksig;
  SECItem mUrl;
};

struct RawAllowlist {
  RawEntry** mEntries;
};

const SEC_ASN1Template EntryTemplate[] = {
    {SEC_ASN1_SEQUENCE, 0, nullptr, sizeof(RawEntry)},
    {SEC_ASN1_UTF8_STRING, offsetof(RawEntry, mKey)},
    {SEC_ASN1_UTF8_STRING, offsetof(RawEntry, mSksig)},
    {SEC_ASN1_UTF8_STRING, offsetof(RawEntry, mUrl)},
    {0}};

const SEC_ASN1Template AllowlistTemplate[] = {
    {SEC_ASN1_SEQUENCE_OF, 0, EntryTemplate, sizeof(RawAllowlist)},
};

CERTCertExtension* FindBerytusExtension(CERTCertificate* aCert) {
  CERTCertExtension **extensions, *extension;
  // SECItem oid = {
  //     siDEROID,
  //     (unsigned char *)sBerytusX509ExtensionOID,
  //     sizeof(sBerytusX509ExtensionOID)
  // };
  // CERT_FindCertExtensionByOID(aCert, &oid, &extensionData)
  // NOTE(berytus): ^ CERT_FindCertExtensionByOID seems to be private
  extensions = aCert->extensions;

  while (extensions != nullptr && *extensions != nullptr) {
    extension = *extensions;
    SECItem* currentOid = &extension->id;
    if (currentOid->len == sizeof(sBerytusX509ExtensionOID) &&
        0 == memcmp(currentOid->data, sBerytusX509ExtensionOID,
                    sizeof(sBerytusX509ExtensionOID))) {
      // extension data should live as long as the passed certificate lives
      return extension;
    }
    extensions++;
  }
  return nullptr;
}

NS_IMPL_CYCLE_COLLECTION(BerytusX509Extension, mAllowlist)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusX509Extension)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusX509Extension)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusX509Extension)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusX509Extension::BerytusX509Extension(
    nsTArray<RefPtr<SigningKeyEntry>>&& aAllowlist)
    : mAllowlist(std::move(aAllowlist)) {}
BerytusX509Extension::~BerytusX509Extension() {}

const nsTArray<RefPtr<BerytusX509Extension::SigningKeyEntry>>&
BerytusX509Extension::GetAllowlist() const {
  return mAllowlist;
}

nsresult BerytusX509Extension::IsAllowed(const nsCString& aSpki, nsIURI* aUrl,
                                         bool& aRv) const {
  auto logUrl = aUrl->GetSpecOrDefault();
  MOZ_LOG(sLogger, LogLevel::Info,
         ("BerytusX509Extension::IsAllowed(): Checking (%.*s, %.*s) against %d entries\n",
          (int) logUrl.Length(), logUrl.Data(),
          (int) aSpki.Length(), aSpki.Data(),
          (int) mAllowlist.Length()));
  nsresult res;
  for (const auto& entry : mAllowlist) {
    bool matched;
    res = entry->Matches(aSpki, aUrl, matched);
    if (NS_WARN_IF(NS_FAILED(res))) {
      return res;
    }
    if (matched) {
      MOZ_LOG(sLogger, LogLevel::Info,
              ("BerytusX509Extension::IsAllowed(): Allowed (%.*s, %.*s)\n",
                (int) logUrl.Length(), logUrl.Data(),
                (int) aSpki.Length(), aSpki.Data()));
      aRv = true;
      return NS_OK;
    }
  }
  MOZ_LOG(sLogger, LogLevel::Info,
          ("BerytusX509Extension::IsAllowed(): NotAllowed (%.*s, %.*s)\n",
          (int) logUrl.Length(), logUrl.Data(),
          (int) aSpki.Length(), aSpki.Data()));
  aRv = false;
  return NS_OK;
}

already_AddRefed<BerytusX509Extension> BerytusX509Extension::Create(
    nsPIDOMWindowInner* aInner, nsresult& aRv) {
  MOZ_ASSERT(aInner);
  nsCOMPtr<nsIChannel> nsCh = aInner->GetDoc()->GetChannel();
  nsITransportSecurityInfo* securityInfo;
  nsresult rv = nsCh->GetSecurityInfo(&securityInfo);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv = rv;
    return nullptr;
  }
  nsIX509Cert* x509Cert;
  rv = securityInfo->GetServerCert(&x509Cert);
  if (NS_WARN_IF(NS_FAILED(rv))) {
    aRv = rv;
    return nullptr;
  }
  return Create(x509Cert->GetCert(), aRv);
}

already_AddRefed<BerytusX509Extension> BerytusX509Extension::Create(
    CERTCertificate* aCert, nsresult& aRv) {
  MOZ_ASSERT(aCert);
  aRv = NS_OK;
  RefPtr<BerytusX509Extension> ret = nullptr;
  CERTCertExtension* extension;
  SECStatus status;
  PLArenaPool* arena = nullptr;
  RawAllowlist* allowlist = nullptr;
  RawEntry **entries, *entry;
  nsTArray<RefPtr<SigningKeyEntry>> outEntries;

  extension = FindBerytusExtension(aCert);
  if (!extension) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("BerytusX509Extension::Create(): No Berytus X509 "
             "extension was found."));
    aRv = NS_ERROR_FILE_NOT_FOUND;
    goto cleanup;
  }
  MOZ_ASSERT(extension->value.len > 0);
  MOZ_ASSERT(extension->value.data);
  MOZ_LOG(sLogger, LogLevel::Info,
          ("BerytusX509Extension::Create(): A Berytus X509 "
           "extension was found!"));

  arena = PORT_NewArena(DER_DEFAULT_CHUNKSIZE);

  if (NS_WARN_IF(arena == nullptr)) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  allowlist = PORT_ArenaZNew(arena, RawAllowlist);
  if (NS_WARN_IF(allowlist == nullptr)) {
    aRv = NS_ERROR_OUT_OF_MEMORY;
    goto cleanup;
  }

  status = SEC_QuickDERDecodeItem(arena,
                                  allowlist,
                                  AllowlistTemplate,
                                  &extension->value);
  if (NS_WARN_IF(status == SECFailure)) {
    aRv = NS_ERROR_FAILURE;
    goto cleanup;
  }

  entries = allowlist->mEntries;
  while (entries != nullptr && *entries != nullptr) {
    entry = *entries;
    MOZ_LOG(sLogger, LogLevel::Info,
           ("BerytusX509Extension::Create(): Entry.URL = %.*s; "
            "Entry.SPKI = %.*s; "
            "Entry.SKSig = %.*s\n",
            (int)entry->mUrl.len, (char*)entry->mUrl.data,
            (int)entry->mKey.len, (char*)entry->mKey.data,
            (int)entry->mSksig.len, (char*)entry->mSksig.data));
    RefPtr<berytus::UrlSearchExpression> url =
        berytus::UrlSearchExpression::Create(
            nsCString((char*)entry->mUrl.data, entry->mUrl.len), aRv);
    if (aRv == NS_ERROR_INVALID_ARG) {
      MOZ_LOG(sLogger, LogLevel::Warning,
             ("BerytusX509Extension::Create(): Invalid allowlist "
              "encountered; Reason: Bad URL."));
      goto cleanup;
    }
    if (NS_WARN_IF(NS_FAILED(aRv))) {
      goto cleanup;
    }
    MOZ_ASSERT(url);
    RefPtr<BerytusX509Extension::SigningKeyEntry> newEntry =
        new BerytusX509Extension::SigningKeyEntry(
            nsCString((char*)entry->mKey.data, entry->mKey.len),
            nsCString((char*)entry->mSksig.data, entry->mSksig.len),
            url);
    outEntries.AppendElement(newEntry);
    entries++;
  }

  ret = new BerytusX509Extension(std::move(outEntries));
  aRv = NS_OK;
  goto cleanup;
cleanup:
  if (arena) {
    PORT_FreeArena(arena, PR_TRUE);
  }
  if (!ret) {
    MOZ_ASSERT(NS_FAILED(aRv));
    return nullptr;
  }
  MOZ_ASSERT(NS_SUCCEEDED(aRv));
  return ret.forget();
}

NS_IMPL_CYCLE_COLLECTION(BerytusX509Extension::SigningKeyEntry, mUrl)
NS_IMPL_CYCLE_COLLECTING_ADDREF(BerytusX509Extension::SigningKeyEntry)
NS_IMPL_CYCLE_COLLECTING_RELEASE(BerytusX509Extension::SigningKeyEntry)
NS_INTERFACE_MAP_BEGIN_CYCLE_COLLECTION(BerytusX509Extension::SigningKeyEntry)
  NS_INTERFACE_MAP_ENTRY(nsISupports)
NS_INTERFACE_MAP_END

BerytusX509Extension::SigningKeyEntry::SigningKeyEntry(nsCString&& aSpki,
                                                       nsCString&& aSkSig,
                                                       RefPtr<berytus::UrlSearchExpression>& aUrl)
    : mSpki(std::move(aSpki)), mSkSig(std::move(aSkSig)), mUrl(aUrl) {}

BerytusX509Extension::SigningKeyEntry::~SigningKeyEntry() {}

nsresult BerytusX509Extension::SigningKeyEntry::SigningKeyEntry::Matches(
    const nsCString& aSpki, nsIURI* aUrl, bool& aRv) const {
  MOZ_ASSERT(aUrl);
  if (!mSpki.Equals(aSpki)) {
    MOZ_LOG(sLogger, LogLevel::Info,
            ("SigningKeyEntry::Matches(): NoMatch "
             "aSPKI (%.*s) against mSPKI (%.*s)\n",
             (int) aSpki.Length(), aSpki.Data(),
             (int) mSpki.Length(), mSpki.Data()));
    aRv = false;
    return NS_OK;
  }
  MOZ_LOG(sLogger, LogLevel::Info,
          ("SigningKeyEntry::Matches(): Matched "
           "aSPKI (%.*s) against mSPKI (%.*s)\n",
           (int) aSpki.Length(), aSpki.Data(),
           (int) mSpki.Length(), mSpki.Data()));
  return mUrl->Matches(aUrl, aRv);
}

const nsCString& BerytusX509Extension::SigningKeyEntry::GetSpki() {
  return mSpki;
}
const nsCString& BerytusX509Extension::SigningKeyEntry::GetSkSig() {
  return mSkSig;
}
RefPtr<const berytus::UrlSearchExpression>
BerytusX509Extension::SigningKeyEntry::GetUrl() {
  return mUrl;
}
};  // namespace mozilla::dom
