//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "common/Common.hpp"
#include "offline/ClockSkewManager.hpp"

using namespace Microsoft::Applications::Events;

TEST(ClockSkewManagerTests, handleResponse_NoTimeDelta_SetsResumeTransmissionAfterClockSkew)
{
   ClockSkewManager clockSkewManager;
   clockSkewManager.handleResponse(HttpHeaders {});
   ASSERT_TRUE(clockSkewManager.GetResumeTransmissionAfterClockSkew());
}

TEST(ClockSkewManagerTests, handleResponse_TimeDeltaSet_SetsResumeTransmissionAfterClockSkew)
{
   ClockSkewManager clockSkewManager;
   HttpHeaders headers;
   headers.add("time-delta-millis", "42");
   clockSkewManager.handleResponse(headers);
   ASSERT_TRUE(clockSkewManager.GetResumeTransmissionAfterClockSkew());
}

TEST(ClockSkewManagerTests, handleResponse_TimeDeltaEmpty_GetDeltaReturnsSentinel)
{
   ClockSkewManager clockSkewManager;
   HttpHeaders headers;
   headers.add("time-delta-millis", std::string {});
   clockSkewManager.handleResponse(headers);
   ASSERT_EQ(clockSkewManager.GetDelta(), std::string { "use-collector-delta" });
}

TEST(ClockSkewManagerTests, setDelta_EmptyString_GetDeltaReturnsSentinel)
{
   ClockSkewManager clockSkewManager;
   clockSkewManager.SetDelta(std::string {});
   ASSERT_EQ(clockSkewManager.GetDelta(), std::string { "use-collector-delta" });
}

TEST(ClockSkewManagerTests, isClockSkewOn_DefaultConstructed_ReturnsTrue)
{
   ASSERT_TRUE(ClockSkewManager { }.isClockSkewOn());
}

TEST(ClockSkewManagerTests, isClockSkewOn_AfterCallingGetDelta_ReturnsFalse)
{
   ClockSkewManager clockSkewManager;
   clockSkewManager.GetDelta();
   ASSERT_FALSE(clockSkewManager.isClockSkewOn());
}

TEST(ClockSkewManagerTests, isClockSkewOn_AfterCallingSetDeltaNonEmpty_ReturnsTrue)
{
   ClockSkewManager clockSkewManager;
   clockSkewManager.SetDelta(std::string { "42" });
   ASSERT_TRUE(clockSkewManager.isClockSkewOn());
}

TEST(ClockSkewManagerTests, isClockSkewOn_AfterCallingSetDeltaAndGetDeltaEmptyString_ReturnsFalse)
{
   ClockSkewManager clockSkewManager;
   clockSkewManager.GetDelta();
   clockSkewManager.SetDelta(std::string {});
   ASSERT_FALSE(clockSkewManager.isClockSkewOn());
}

TEST(ClockSkewManagerTests, GetDelta_DefaultConstructed_ReturnsSentinel)
{
   ASSERT_EQ(ClockSkewManager {}.GetDelta(), std::string { "use-collector-delta" });
}
