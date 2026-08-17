//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// Unit tests for the pure MS-root certificate policy decision helper. These run
// on any platform with no live network and no WinInet/Wincrypt dependency: they
// exercise the tri-state (Allow / Reject / Unable) that the transport relies on.
//
// They deliberately encode two properties the legacy two-state boolean design
// could not represent, so they FAIL against the old behavior:
//   1. "could not evaluate" is distinct from "evaluated and rejected"
//      (tri-state), and
//   2. both "could not evaluate" cases (query unavailable, policy API failure)
//      preserve fail-open (ShouldProceed == true), whereas the legacy code
//      mapped a policy-API failure to a hard rejection.
//
#include "common/Common.hpp"

#include "http/detail/MsRootCertPolicy.hpp"

using namespace testing;
using namespace MAT;
using MAT::detail::EvaluateMsRootPolicy;
using MAT::detail::MsRootCertQuery;
using MAT::detail::MsRootPolicyDecision;
using MAT::detail::ShouldProceed;

namespace
{
    // A fully successful HTTPS chain query that roots to the Microsoft root.
    MsRootCertQuery MakeSuccessfulHttpsQuery()
    {
        MsRootCertQuery query;
        query.httpsScheme = true;
        query.chainQuerySucceeded = true;
        query.chainContextPresent = true;
        query.policyCheckPerformed = true;
        query.policyStatusError = 0u;  // ERROR_SUCCESS
        return query;
    }
}  // namespace

// success => Allow (and proceeds)
TEST(MsRootCertPolicyTests, SuccessfulMsRootedChainIsAllow)
{
    auto query = MakeSuccessfulHttpsQuery();
    EXPECT_EQ(EvaluateMsRootPolicy(query), MsRootPolicyDecision::Allow);
    EXPECT_TRUE(ShouldProceed(EvaluateMsRootPolicy(query)));
}

// explicit policy error => Reject (and does NOT proceed)
TEST(MsRootCertPolicyTests, EvaluatedNonMsRootedChainIsReject)
{
    auto query = MakeSuccessfulHttpsQuery();
    query.policyStatusError = 0x800B0109u;  // e.g. CERT_E_UNTRUSTEDROOT
    EXPECT_EQ(EvaluateMsRootPolicy(query), MsRootPolicyDecision::Reject);
    EXPECT_FALSE(ShouldProceed(EvaluateMsRootPolicy(query)));
}

// query unavailable => Unable, and fails OPEN (proceeds).
// This is the preserved downlevel-OS / no-cert-chain behavior.
TEST(MsRootCertPolicyTests, ChainQueryUnavailableIsUnableAndFailsOpen)
{
    MsRootCertQuery query;
    query.httpsScheme = true;
    query.chainQuerySucceeded = false;  // InternetQueryOption failed
    query.chainContextPresent = false;
    query.policyCheckPerformed = false;

    EXPECT_EQ(EvaluateMsRootPolicy(query), MsRootPolicyDecision::Unable);
    EXPECT_TRUE(ShouldProceed(EvaluateMsRootPolicy(query)));
}

// query succeeds but yields no chain context => Unable / fail open.
TEST(MsRootCertPolicyTests, ChainQuerySucceedsButNoContextIsUnableAndFailsOpen)
{
    MsRootCertQuery query;
    query.httpsScheme = true;
    query.chainQuerySucceeded = true;
    query.chainContextPresent = false;  // nothing to verify
    query.policyCheckPerformed = false;

    EXPECT_EQ(EvaluateMsRootPolicy(query), MsRootPolicyDecision::Unable);
    EXPECT_TRUE(ShouldProceed(EvaluateMsRootPolicy(query)));
}

// policy API failure => Unable (fail open), NOT Reject.
// The legacy boolean code returned "not trusted" (reject) here; the product
// decision is to preserve fail-open when verification cannot be performed. This
// assertion is what fails the old behavior.
TEST(MsRootCertPolicyTests, PolicyApiFailureIsUnableNotReject)
{
    MsRootCertQuery query;
    query.httpsScheme = true;
    query.chainQuerySucceeded = true;
    query.chainContextPresent = true;
    query.policyCheckPerformed = false;   // CertVerifyCertificateChainPolicy returned FALSE
    query.policyStatusError = 0u;

    auto decision = EvaluateMsRootPolicy(query);
    EXPECT_EQ(decision, MsRootPolicyDecision::Unable);
    EXPECT_NE(decision, MsRootPolicyDecision::Reject);
    EXPECT_TRUE(ShouldProceed(decision));
}

// Non-HTTPS is never subject to the MS-root policy, regardless of other inputs.
TEST(MsRootCertPolicyTests, NonHttpsIsAlwaysAllow)
{
    MsRootCertQuery query;
    query.httpsScheme = false;
    query.chainQuerySucceeded = true;
    query.chainContextPresent = true;
    query.policyCheckPerformed = true;
    query.policyStatusError = 0x800B0109u;  // would be a reject if HTTPS

    EXPECT_EQ(EvaluateMsRootPolicy(query), MsRootPolicyDecision::Allow);
    EXPECT_TRUE(ShouldProceed(EvaluateMsRootPolicy(query)));
}

// The three outcomes are genuinely distinct: a test that only knew about a
// two-state (trusted/untrusted) result could not satisfy all of these at once.
TEST(MsRootCertPolicyTests, AllowRejectUnableAreDistinct)
{
    auto allow = EvaluateMsRootPolicy(MakeSuccessfulHttpsQuery());

    auto rejectQuery = MakeSuccessfulHttpsQuery();
    rejectQuery.policyStatusError = 0x800B0109u;
    auto reject = EvaluateMsRootPolicy(rejectQuery);

    MsRootCertQuery unableQuery;
    unableQuery.httpsScheme = true;
    auto unable = EvaluateMsRootPolicy(unableQuery);

    EXPECT_NE(allow, reject);
    EXPECT_NE(allow, unable);
    EXPECT_NE(reject, unable);

    // Fail-open contract: only Reject stops the request.
    EXPECT_TRUE(ShouldProceed(allow));
    EXPECT_FALSE(ShouldProceed(reject));
    EXPECT_TRUE(ShouldProceed(unable));
}
