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

TEST(KillSwitchManagerTests, handleResponse_MalformedKillToken_IsRejected)
{
    // A token containing characters outside the tenant-token set must be ignored.
    KillSwitchManager manager;
    HttpHeaders headers;
    headers.add("kill-tokens", "x\" OR 1=1; DROP TABLE events; --");
    headers.add("kill-duration", "300");
    ASSERT_NO_THROW(manager.handleResponse(headers));
    ASSERT_FALSE(manager.isActive());
}
