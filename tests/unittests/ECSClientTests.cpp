// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules\exp\ecs\ecsclient\ECSClient.hpp"

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;

TEST(ECSClientTests, Constrcut_OK)
{
    new ECSClient();
}

TEST(ECSClientTests, Initialize_Failed_ClientNameEmpty)
{
    auto client = new ECSClient();
    auto config = ECSClientConfiguration();
    ASSERT_DEATH({
        client->Initialize(config);
    }, "");
}

TEST(ECSClientTests, Start_Failed_NotInitialized)
{
    auto client = new ECSClient();
    auto ret = client->Start();
    ASSERT_EQ(ret, false);
}

TEST(ECSClientTests, Start_Failed_AlreadyStarted)
{
    auto client = new ECSClient();
    auto config = ECSClientConfiguration();
    config.clientName = "Test";
    config.clientVersion = "1.0";
    config.cacheFilePathName = "cacheFilePathName";    

    client->Initialize(config);
    client->Start();
    auto ret = client->Start();
    ASSERT_EQ(ret, false);
}

TEST(ECSClientTests, Start_OK)
{
    auto client = new ECSClient();
    auto config = ECSClientConfiguration();
    config.clientName = "Test";
    config.clientVersion = "1.0";
    config.cacheFilePathName = "cacheFilePathName";    

    client->Initialize(config);
    auto ret = client->Start();
    ASSERT_EQ(ret, true);
}


#endif