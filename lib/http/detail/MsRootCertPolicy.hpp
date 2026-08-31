//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
// PRIVATE, internal-only header. It is intentionally NOT part of the installed
// public SDK surface: it is not referenced by any public header, is not copied
// by the install rules, and exposes no ABI. It contains a single pure,
// platform-independent policy-decision function so the MS-root certificate
// decision can be reasoned about and unit-tested without a live TLS connection
// or any WinInet/Wincrypt dependency. The runtime transport (HttpClient_WinInet)
// gathers the raw query/build/policy facts from WinInet and feeds them here; it
// does not reach back into transport internals, so no friend/test hook is
// required.
//
#ifndef HTTP_DETAIL_MSROOTCERTPOLICY_HPP
#define HTTP_DETAIL_MSROOTCERTPOLICY_HPP

#include "ctmacros.hpp"

#include <cstdint>

namespace MAT_NS_BEGIN
{
namespace detail
{
    /// <summary>
    /// Tri-state outcome of the Microsoft-root certificate policy evaluation.
    ///
    /// The distinction between <c>Reject</c> and <c>Unable</c> is the whole point
    /// of this helper: the legacy transport collapsed both into a single "not
    /// trusted" boolean, which conflated "the chain was evaluated and is not
    /// MS-rooted" with "the chain could not be evaluated at all". The policy
    /// fails closed whenever evaluation cannot establish that the server
    /// certificate satisfies the Microsoft-root requirement.
    /// </summary>
    enum class MsRootPolicyDecision
    {
        /// The connection may proceed: policy is not applicable (non-HTTPS) or
        /// the chain was evaluated and satisfies the Microsoft-root policy.
        Allow,

        /// The chain was evaluated and confirmed NOT to be MS-rooted (or the
        /// policy engine reported an explicit policy error). Reject the request.
        Reject,

        /// The chain could not be queried, built, or verified. This is distinct
        /// from an evaluated rejection for diagnostics, but both outcomes stop
        /// the request.
        Unable
    };

    /// <summary>
    /// Raw, transport-gathered facts required to make the policy decision. All
    /// fields are plain scalars so this header carries no platform dependency.
    /// </summary>
    struct MsRootCertQuery
    {
        /// True when the request scheme is HTTPS. The MS-root policy only
        /// inspects HTTPS connections; anything else is Allow.
        bool httpsScheme{false};

        /// True when querying the server certificate chain context succeeded
        /// (e.g. InternetQueryOption(INTERNET_OPTION_SERVER_CERT_CHAIN_CONTEXT)).
        bool chainQuerySucceeded{false};

        /// True when the query actually produced a non-null chain context to
        /// evaluate. A successful query that yields no context is still "unable".
        bool chainContextPresent{false};

        /// True when the policy-verification API ran to completion (e.g.
        /// CertVerifyCertificateChainPolicy returned TRUE). False means the
        /// verification itself could not be performed.
        bool policyCheckPerformed{false};

        /// The policy status error reported by the verification API when
        /// policyCheckPerformed is true (0 == success == MS-rooted).
        std::uint32_t policyStatusError{0};
    };

    /// <summary>
    /// Deterministically maps the gathered facts to an Allow / Reject / Unable
    /// decision. Pure function: no I/O, no globals, no platform calls.
    /// </summary>
    inline MsRootPolicyDecision EvaluateMsRootPolicy(const MsRootCertQuery& query) noexcept
    {
        // Policy only applies to HTTPS. HTTP (and anything non-HTTPS) proceeds.
        if (!query.httpsScheme)
        {
            return MsRootPolicyDecision::Allow;
        }

        // Could not obtain a chain to evaluate -> cannot establish trust.
        if (!query.chainQuerySucceeded || !query.chainContextPresent)
        {
            return MsRootPolicyDecision::Unable;
        }

        // Obtained a chain but the verification API itself did not run to
        // completion -> cannot establish trust.
        if (!query.policyCheckPerformed)
        {
            return MsRootPolicyDecision::Unable;
        }

        // Verification ran: a non-success status is an evaluated rejection.
        if (query.policyStatusError != 0u)
        {
            return MsRootPolicyDecision::Reject;
        }

        return MsRootPolicyDecision::Allow;
    }

    /// <summary>
    /// Only a successfully evaluated Allow permits the request.
    /// </summary>
    inline bool ShouldProceed(MsRootPolicyDecision decision) noexcept
    {
        return decision == MsRootPolicyDecision::Allow;
    }

}  // namespace detail
}
MAT_NS_END

#endif  // HTTP_DETAIL_MSROOTCERTPOLICY_HPP
