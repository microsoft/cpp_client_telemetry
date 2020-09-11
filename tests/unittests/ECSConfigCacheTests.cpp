// Copyright (c) Microsoft. All rights reserved.

#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSConfigCache.hpp"

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;

TEST(ECSConfigCacheTests, Constrcut_OK_RelativePath)
{
    const std::string storagePath = "path";
    std::make_unique<ECSConfigCache>(storagePath);
}

TEST(ECSConfigCacheTests, Constrcut_OK_AbsolutePath)
{    
    const std::string storagePath = std::string("parentfolder") + PATH_SEPARATOR_CHAR + "childfolder";
    std::make_unique<ECSConfigCache>(storagePath);
}

TEST(ECSConfigCacheTests, AddConfig)
{
    const std::string storagePath = "path";
    auto configCache = std::make_unique<ECSConfigCache>(storagePath);
    const std::string requestName = "requestName";
    ECSConfig config;
    config.requestName = requestName;
    config.etag = "etag";
    auto addedConfig = configCache->AddConfig(config);
    ASSERT_EQ(addedConfig->etag, configCache->GetConfigByRequestName(requestName)->etag);
}

TEST(ECSConfigCacheTests, GetConfigByRequestName_NotExistConfig)
{
    const std::string storagePath = "path";
    auto configCache = std::make_unique<ECSConfigCache>(storagePath);
    ASSERT_EQ(NULL, configCache->GetConfigByRequestName("requestName"));
}

#endif