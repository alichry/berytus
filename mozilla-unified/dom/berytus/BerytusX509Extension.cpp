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
                      char delimiter,
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
  PartIterator searchItr(aSearchString, delimiter);
  PartIterator candidateItr(aCandidateString, delimiter);
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
  if (!searchItr.Finished()) {
    // e.g. [0] = "/abc/def/*/ghi"
    //                      ^ we are here (unfinished)
    //      or even:
    //      [0] = "/abc/def/*/"
    //                       ^ we are here (unfinished)
    //      [1] = "/abc/def/123/456/xyz"
    //                                 ^ we are here (finished)
    aRv = false;
    return NS_OK;
  }
  MOZ_ASSERT(candidateItr.Finished());
  aRv = true;
  return NS_OK;
}
}

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
  nsresult res;
  for (const auto& entry : mAllowlist) {
    res = entry->Matches(aSpki, aUrl, aRv);
    if (NS_WARN_IF(NS_FAILED(res))) {
      return res;
    }
    if (aRv) {
      return NS_OK;
    }
  }
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
    aRv = NS_ERROR_FILE_NOT_FOUND;
    goto cleanup;
  }
  MOZ_ASSERT(extension->value.len > 0);
  MOZ_ASSERT(extension->value.data);

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

  status = SEC_QuickDERDecodeItem(arena, allowlist, AllowlistTemplate,
                                  &extension->value);
  if (NS_WARN_IF(status == SECFailure)) {
    aRv = NS_ERROR_FAILURE;
    goto cleanup;
  }

  entries = allowlist->mEntries;
  while (entries != nullptr && *entries != nullptr) {
    entry = *entries;
    RefPtr<BerytusX509Extension::SigningKeyEntry::Url> url =
        BerytusX509Extension::SigningKeyEntry::Url::Create(
            nsCString((char*)entry->mUrl.data, entry->mUrl.len), aRv);
    if (aRv == NS_ERROR_INVALID_ARG) {
      // TODO(berytus): Here, log a warning about the invalid cert ext
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
                                                       RefPtr<Url>& aUrl)
    : mSpki(std::move(aSpki)), mSkSig(std::move(aSkSig)), mUrl(aUrl) {}

BerytusX509Extension::SigningKeyEntry::~SigningKeyEntry() {}

nsresult BerytusX509Extension::SigningKeyEntry::SigningKeyEntry::Matches(
    const nsCString& aSpki, nsIURI* aUrl, bool& aRv) const {
  MOZ_ASSERT(aUrl);
  if (!mSpki.Equals(aSpki)) {
    aRv = false;
    return NS_OK;
  }
  return mUrl->Matches(aUrl, aRv);
}

const nsCString& BerytusX509Extension::SigningKeyEntry::GetSpki() {
  return mSpki;
}
const nsCString& BerytusX509Extension::SigningKeyEntry::GetSkSig() {
  return mSkSig;
}
RefPtr<const BerytusX509Extension::SigningKeyEntry::Url>
BerytusX509Extension::SigningKeyEntry::GetUrl() {
  return mUrl;
}

NS_IMPL_ISUPPORTS0(BerytusX509Extension::SigningKeyEntry::Url)

BerytusX509Extension::SigningKeyEntry::SigningKeyEntry::Url::Url(
    nsCString&& aHostname, const int& aPort, nsCString&& aFilePath)
    : mHostname(aHostname), mPort(aPort), mFilePath(aFilePath) {}

BerytusX509Extension::SigningKeyEntry::SigningKeyEntry::Url::~Url() {}

bool ComparePartsStrict(const nsCString& searchString,
                  const nsCString& candidateString, char delimiter = '.') {
  if (searchString.IsEmpty()) {
    MOZ_ASSERT_UNREACHABLE("Search string must not be empty");
    return false;
  }
  if (candidateString.IsEmpty()) {
    MOZ_ASSERT_UNREACHABLE("Candidate string must not be empty");
    return false;
  }
  if (searchString.CharAt(searchString.Length() - 1) == delimiter) {
    MOZ_ASSERT_UNREACHABLE("Search string must not end with {delimiter}");
    return false;
  }
  if (candidateString.CharAt(candidateString.Length() - 1) == delimiter) {
    MOZ_ASSERT_UNREACHABLE("Candidate string must not end with {delimiter}");
    return false;
  }
  if (searchString.Equals("*")) {
    return true;
  }
  uint64_t s1 = 0, s2 = 0, e1 = 0, e2 = 0;
  while (s1 < searchString.Length() && s2 < candidateString.Length()) {
    if (searchString.CharAt(s1) == delimiter) {
      MOZ_ASSERT_UNREACHABLE(
          "Search string part sohuld not begin with {delimiter}; expected "
          "caller to provide valid parts.");
      return false;
    }
    if (candidateString.CharAt(s2) == delimiter) {
      MOZ_ASSERT_UNREACHABLE(
          "Candidate string part sohuld not begin with a {dellimiter}; "
          "expected caller to provide valid parts.");
      return false;
    }
    e1 = s1;
    e2 = s2;
    while (e1 + 1 < searchString.Length() &&
           searchString.CharAt(e1 + 1) != delimiter) {
      e1++;
    }
    while (e2 + 1 < candidateString.Length() &&
           candidateString.CharAt(e2 + 1) != delimiter) {
      e2++;
    }
    const nsACString& searchPart = Substring(searchString, s1, e1);
    const nsACString& candidatePart = Substring(candidateString, s2, e2);
    if (!searchPart.Equals("*") && !searchPart.Equals(candidatePart)) {
      return false;
    }
    // we need to move to the next part.
    // e1, e2 points to either
    //  (1) the end of the string
    //  (2) the end of the part which should leave at least two characters ahead
    // Moreover, if (1) is not reached, (2) is guaranteed since the last char
    // has been checked not to be a {delimiter} (see head of this impl); i.e.,
    // during traversal, we would have stopped when the next character is a
    // {delimiter}, and it surely is not the last character.
    unsigned char searchConsumed = e1 == searchString.Length() - 1;
    unsigned char candidateConsumed = e2 == candidateString.Length() - 1;
    if ((searchConsumed ^ candidateConsumed) == 1) {
      // one has been consumed while the other did not.
      return false;
    }
    // both have not been consummed yet with both having at least 2 chars left
    // OR
    // both have been consumed.
    s1 = e1 + 2;
    s2 = e2 + 2;
  }
  return true;
}

nsresult BerytusX509Extension::SigningKeyEntry::Url::Matches(nsIURI* aUrl,
                                                             bool& aRv) const {
  MOZ_ASSERT(aUrl);
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
    aRv = false;
    return NS_OK;
  }
  if (candidatePort != mPort) {
    aRv = false;
    return NS_OK;
  }
  bool matched;
  res = berytus::CompareParts(mHostname, candidateHost, '.', matched);
  NS_ENSURE_SUCCESS(res, res);
  if (!matched) {
    aRv = false;
    return NS_OK;
  }
  res = berytus::CompareParts(mFilePath, candidateFilePath, '/', matched);
  NS_ENSURE_SUCCESS(res, res);
  if (!matched) {
    aRv = false;
    return NS_OK;
  }
  aRv = true;
  return NS_OK;
}

already_AddRefed<BerytusX509Extension::SigningKeyEntry::Url>
BerytusX509Extension::SigningKeyEntry::Url::Create(const nsCString& aUrl,
                                                   nsresult& aRv) {
  nsCOMPtr<nsIIOService> io = mozilla::components::IO::Service();
  if (NS_WARN_IF(!io)) {
    aRv = NS_ERROR_FAILURE;
    return nullptr;
  }
  if (aUrl.Equals("*")) {
    aRv = NS_OK;
    return do_AddRef(new Url(nsCString("*"_ns), -1, nsCString("/*"_ns)));
  }
  nsCOMPtr<nsIURI> parsedUri;
  aRv = io->NewURI(aUrl, nullptr, nullptr,
                   getter_AddRefs(parsedUri));
  NS_ENSURE_SUCCESS(aRv, nullptr);

  MOZ_ASSERT(parsedUri);
  if (!parsedUri->SchemeIs("https:")) {
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
  return do_AddRef(new Url(std::move(host), port, std::move(filePath)));
}

};  // namespace mozilla::dom
