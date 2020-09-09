// Copyright (c) Microsoft. All rights reserved.

#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSClientUtils.hpp"

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;

TEST(ECSClientUtilsTests, CreateServerUrl)
{
    const std::string serverUrl = "https://this.is.a.fake.endpoint.com/config/v1";
    const std::string clientName = "ATM";
    const std::string clientVersion = "1.0.0.0";
    const auto ret = CreateServerUrl(serverUrl, clientName, clientVersion);
    ASSERT_EQ(ret, "https://this.is.a.fake.endpoint.com/config/v1/ATM/1.0.0.0");
}

TEST(ECSClientUtilsTests, CreateServerUrl_EmptyClientVersion)
{
    const std::string serverUrl = "https://this.is.a.fake.endpoint.com/config/v1";
    const std::string clientName = "ATM";
    const std::string clientVersion = "";
    const auto ret = CreateServerUrl(serverUrl, clientName, clientVersion);
    ASSERT_EQ(ret, "https://this.is.a.fake.endpoint.com/config/v1/ATM");
}
#endif