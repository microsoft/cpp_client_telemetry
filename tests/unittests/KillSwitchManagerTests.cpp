//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "offline/KillSwitchManager.hpp"

using namespace Microsoft::Applications::Events;

TEST(KillSwitchManagerTests, handleResponse_ValidRetryAfter_ActivatesRetryAfter)
{
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "120");
    manager.handleResponse(headers);
    ASSERT_TRUE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_NonNumericRetryAfter_DoesNotThrowAndIsIgnored)
{
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "not-a-number");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_HttpDateRetryAfter_DoesNotThrowAndIsIgnored)
{
    // RFC 7231 permits Retry-After as an HTTP-date. It is non-numeric and must
    // be ignored rather than crash the worker thread.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "Wed, 21 Oct 2025 07:28:00 GMT");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_OutOfRangeRetryAfter_DoesNotThrowAndIsIgnored)
{
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "999999999999999999999999999999");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_RetryAfterWithTrailingGarbage_IsIgnored)
{
    // A numeric prefix followed by trailing garbage (e.g. "120; foo") must be
    // treated as malformed, not parsed as 120.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "120; foo");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_RetryAfterWithTrailingWhitespace_IsAccepted)
{
    // Trailing whitespace is allowed (HTTP OWS) and must still parse.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "120  ");
    manager.handleResponse(headers);
    ASSERT_TRUE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_RetryAfterWithTrailingCRLF_IsIgnored)
{
    // CR/LF are not HTTP OWS (only SP / HTAB is); a value carrying them is
    // malformed and must be ignored rather than parsed from its numeric prefix.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "120\r\n");
    manager.handleResponse(headers);
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_ValidKillTokenAndDuration_BlocksToken)
{
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "tenant-token-1");
    headers.add("kill-duration", "300");
    ASSERT_TRUE(manager.handleResponse(headers));
    ASSERT_TRUE(manager.isTokenBlocked("tenant-token-1"));
}

TEST(KillSwitchManagerTests, handleResponse_NonNumericKillDuration_DoesNotThrowAndDoesNotBlock)
{
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "tenant-token-1");
    headers.add("kill-duration", "forever");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isTokenBlocked("tenant-token-1"));
}

TEST(KillSwitchManagerTests, handleResponse_KillTokenWithControlChars_IsRejected)
{
    // A token containing control characters (e.g. CR/LF) is unsafe -- it could
    // enable log injection -- and must be ignored.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", std::string("tenant\r\ninjected"));
    headers.add("kill-duration", "300");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isActive());
}

TEST(KillSwitchManagerTests, handleResponse_OpaqueKillTokenWithUnusualChars_IsBlocked)
{
    // Tenant tokens are opaque (they may contain spaces/quotes), and the
    // offline-storage DELETE is parameterized, so such a token is safe to act on
    // and must remain killable -- over-restricting would let it escape kill.
    KillSwitchManager manager;
    HttpHeaders headers;
    const std::string token = "x\" OR 1=1; DROP TABLE events; --";
    headers.add("kill-tokens", token);
    headers.add("kill-duration", "300");
    ASSERT_TRUE(manager.handleResponse(headers));
    ASSERT_TRUE(manager.isTokenBlocked(token));
}
