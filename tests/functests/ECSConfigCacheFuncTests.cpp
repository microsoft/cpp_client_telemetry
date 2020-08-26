// Copyright (c) Microsoft. All rights reserved.

#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSConfigCache.hpp"

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;

TEST(ECSConfigCacheTests, SaveConfig)
{
    const std::string storagePath = "storagePath";
    const std::string requestName = "requestName";
    auto configCache = new ECSConfigCache(storagePath);
    ECSConfig config;
    config.requestName = requestName;
    config.etag = "etag";
    auto addedConfig = configCache->AddConfig(config);
    ASSERT_EQ(addedConfig->etag, configCache->GetConfigByRequestName(requestName)->etag);
    ASSERT_EQ(true, configCache->SaveConfig(config));
    delete configCache;
}

TEST(ECSConfigCacheTests, LoadConfig)
{
    const std::string storagePath = "storagePath";
    const std::string requestName = "requestName";
    auto configCache = new ECSConfigCache(storagePath);
    ECSConfig config;
    config.requestName = requestName;
    config.etag = "etag";
    auto addedConfig = configCache->AddConfig(config);
    ASSERT_EQ(addedConfig->etag, configCache->GetConfigByRequestName(requestName)->etag);
    ASSERT_EQ(true, configCache->SaveConfig(config));

    configCache->LoadConfig();
    delete configCache;
}

#endif