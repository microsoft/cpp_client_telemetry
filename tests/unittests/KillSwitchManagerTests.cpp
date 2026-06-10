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

TEST(KillSwitchManagerTests, handleResponse_RetryAfterWithLeadingWhitespace_IsAccepted)
{
    // Leading HTTP OWS (SP / HTAB) is trimmed; the value still parses.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "  120");
    manager.handleResponse(headers);
    ASSERT_TRUE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_RetryAfterWithLeadingSign_IsIgnored)
{
    // RFC 7231 delay-seconds is 1*DIGIT; a leading '+'/'-' sign (which std::stoll
    // would accept) is malformed and must be ignored.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "+120");
    manager.handleResponse(headers);
    ASSERT_FALSE(manager.isRetryAfterActive());
}

TEST(KillSwitchManagerTests, handleResponse_HugeRetryAfter_IsClampedNotWrapped)
{
    // A huge (still in-range) Retry-After must be clamped so the expiry time does
    // not overflow and wrap into the past; the back-off must stay active.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("Retry-After", "9000000000000000000"); // ~9e18, near INT64_MAX
    manager.handleResponse(headers);
    ASSERT_TRUE(manager.isRetryAfterActive());
    ASSERT_TRUE(manager.isTokenBlocked("any-token")); // far-future expiry, not wrapped
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

TEST(KillSwitchManagerTests, handleResponse_HugeKillDuration_IsClampedNotWrapped)
{
    // A huge (still in-range) kill-duration flows through the same parser as
    // Retry-After but is applied per-token via addToken(); it must be clamped so
    // the token's expiry does not overflow and wrap into the past -- the token
    // must stay blocked.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "tenant-token-1");
    headers.add("kill-duration", "9000000000000000000"); // ~9e18, near INT64_MAX
    ASSERT_TRUE(manager.handleResponse(headers));
    ASSERT_TRUE(manager.isTokenBlocked("tenant-token-1")); // clamped expiry, not wrapped
}

TEST(KillSwitchManagerTests, handleResponse_OutOfRangeKillDuration_DoesNotThrowAndDoesNotBlock)
{
    // An out-of-range kill-duration cannot be parsed; it must be ignored (no
    // token blocked) rather than throwing out of the worker thread.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "tenant-token-1");
    headers.add("kill-duration", "999999999999999999999999999999");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isTokenBlocked("tenant-token-1"));
}

TEST(KillSwitchManagerTests, handleResponse_KillTokenWithAllSuffix_BlocksBaseToken)
{
    // The collector may send "<tenant>:all" to mean all events of that tenant are
    // killed. The ":all" suffix (everything from the first ':') is stripped and
    // the base tenant token is what gets blocked.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "tenant-token-1:all");
    headers.add("kill-duration", "300");
    ASSERT_TRUE(manager.handleResponse(headers));
    ASSERT_TRUE(manager.isTokenBlocked("tenant-token-1"));       // suffix stripped
    ASSERT_FALSE(manager.isTokenBlocked("tenant-token-1:all"));  // unstripped form is not blocked
}
