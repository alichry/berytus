/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "gtest/gtest.h"
#include "mozilla/dom/BerytusX509Extension.h"
#include "cert.h"
#include "certt.h"
#include "nsError.h"
#include <iostream>
#include "nss.h"

using namespace mozilla::dom;

TEST(BerytusX509Extension, TestComparePartsBasicMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtHead1stCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "Abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("Abc/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtHead2ndCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "aBc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("aBc/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsHead3rdCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "abC/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abC/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtBody1stCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/def"_ns,
                                      "abc/Def"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/Def"_ns,
                                      "abc/def"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtBody2ndCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/dEf"_ns,
                                      "abc/def"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/def"_ns,
                                      "abc/dEf"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtBody3rdCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/deF"_ns,
                                      "abc/def"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/def"_ns,
                                      "abc/deF"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtTail1stCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/def/hij"_ns,
                                      "abc/def/Hij"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/def/Hij"_ns,
                                      "abc/def/hij"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtTail2ndCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/def/hij"_ns,
                                      "abc/def/hIj"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/def/hIj"_ns,
                                      "abc/def/hij"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsAtTail3rdCharNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/def/hij"_ns,
                                      "abc/def/hiJ"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/def/hiJ"_ns,
                                      "abc/def/hij"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsLeadingDelimMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/abc/123"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsLeadingDelimNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/abc/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsTrailingDelimMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123/"_ns,
                                      "abc/123/"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsTrailingDelimNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/abc/123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsLeadingTrailingDelimMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/abc/123/"_ns,
                                      "/abc/123/"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsLeadingTrailingDelimNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/abc/123"_ns,
                                      "abc/123/"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/123/"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("///abc///123///"_ns,
                                      "///abc///123///"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksAtHeadMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("///abc/123"_ns,
                                      "///abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksAtMiidMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc///123"_ns,
                                      "abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksAtTailMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123///"_ns,
                                      "abc/123///"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksAtHeadNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("//abc/123"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("/abc/123"_ns,
                                      "//abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}
TEST(BerytusX509Extension, TestComparePartsBlanksAtBodyNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "abc//123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc//123"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsBlanksAtTailNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123"_ns,
                                      "abc/123//"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("abc/123//"_ns,
                                      "abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtHeadWithEmptyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtHeadMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "xyz/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtHeadNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "xyz/hij/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "xyz/hij/abc"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtHeadMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtHeadWithBlanksAtHeadMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/*/abc/123"_ns,
                                      "/xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
  rv = mozilla::berytus::CompareParts("///*/abc/123"_ns,
                                      "///xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
    rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "///xyz///hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
  rv = mozilla::berytus::CompareParts("/*/abc/123"_ns,
                                      "///xyz///hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtHeadWithBlanksAtHeadNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("/*/abc/123"_ns,
                                      "xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("//*/abc/123"_ns,
                                      "/xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtHeadWithBlanksAtBodyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/123"_ns,
                                      "xyz///abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
  rv = mozilla::berytus::CompareParts("*/abc///123"_ns,
                                      "xyz///abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtHeadWithBlanksAtBodyNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("*/abc/123"_ns,
                                      "xyz///abC/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("*/abC/123"_ns,
                                      "xyz///abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}


TEST(BerytusX509Extension, TestComparePartsWildcardAtBodyWithEmptyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/*/123"_ns,
                                      "abc//123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtBodyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/*/123"_ns,
                                      "xyz/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtBodyNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/*/abc/123"_ns,
                                      "xyz/hij/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("xyz/*/abc/123"_ns,
                                      "xyz/hij/abc"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtBodyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/*/123"_ns,
                                      "xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtBodyWithBlanksAtBodyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/*/123"_ns,
                                      "xyz///abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
  rv = mozilla::berytus::CompareParts("xyz/*/123"_ns,
                                      "xyz///abc///123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtBodyWithBlanksAtBodyNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/*/abc"_ns,
                                      "xyz///123/abC"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
  rv = mozilla::berytus::CompareParts("xyz/*/abC"_ns,
                                      "xyz///123///abc"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}


TEST(BerytusX509Extension, TestComparePartsWildcardAtTailWithEmptyMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123/*"_ns,
                                      "abc/123/"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtTailWithEmptyNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("abc/123/*/"_ns,
                                      "abc/123/"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtTailMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/123/*"_ns,
                                      "xyz/123/abc"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestComparePartsWildcardAtTailNoMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/hij/abc/*"_ns,
                                      "xyz/hij/abc"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_FALSE(matched);
}

TEST(BerytusX509Extension, TestComparePartsMultiWildcardAtTailMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/hjk/*"_ns,
                                      "xyz/hjk/abc/123"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension,
     TestComparePartsMultiWildcardAtTailWithBlanksAtTailMatch)
{
  nsresult rv;
  bool matched;
  rv = mozilla::berytus::CompareParts("xyz/abc/123/*"_ns,
                                      "xyz/abc/123//"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
  rv = mozilla::berytus::CompareParts("xyz/abc/123/*"_ns,
                                      "xyz/abc/123///"_ns,
                                      '/',
                                      matched);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(matched);
}

TEST(BerytusX509Extension, TestFromCertificate)
{
  ASSERT_TRUE(NSS_NoDB_Init(nullptr) == SECSuccess);

  std::ifstream file("berytus/example.tls.crt");
  ASSERT_TRUE(!!file);
  std::ostringstream buffer;
  buffer << file.rdbuf();
  std::string pem = buffer.str();

  CERTCertificate* cert =
      CERT_DecodeCertFromPackage(pem.data(), (int)pem.length());

  nsresult rv;
  RefPtr<BerytusX509Extension> ext = BerytusX509Extension::Create(cert, rv);
  ASSERT_TRUE(NS_SUCCEEDED(rv));
  ASSERT_TRUE(ext != nullptr);

  CERT_DestroyCertificate(cert);
  NSS_Shutdown();

  auto const& allowlist = ext->GetAllowlist();
  ASSERT_EQ(allowlist.Length(), 1ul);

  auto const& entry = allowlist.ElementAt(0);
  MOZ_ASSERT(entry->GetUrl());

  ASSERT_TRUE(nsCString(*entry->GetUrl()).EqualsASCII("*"));
  ASSERT_TRUE(entry->GetSpki().EqualsASCII(
      "MCowBQYDK2VwAyEAgz0GKn8PrHDQRRj/AGVJKMXLMhrNsaG71SYBPgxjVXs="));
  ASSERT_TRUE(entry->GetSkSig().EqualsASCII(
      "onDJhaR2JzTgUiWgnDeAGAolOOPJrdgunhwEgP81Z5zoqxRtz4/QRBGziebJu/yH/vt72OeMxQbQE5xhH+yADw=="));
}