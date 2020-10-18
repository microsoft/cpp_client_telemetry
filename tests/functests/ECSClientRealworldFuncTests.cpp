//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "ECSClientCommon.hpp"
#ifdef HAVE_MAT_EXP

namespace {
    // config: https://ecs.skype.net/?page=ExperimentPage&type=Rollout&id=45994
    
    using namespace ECSClientCommon;
    const char* const clientName = "ECS";
    const char* const agent = "ECSDemo";
    const char* const cacheFilePathName = "cacheFilePathName";

    class ECSClientRealworldFuncTests : public ::testing::Test
    {
        protected:
            std::string offlineStoragePath = ECSConfigCache::GetStoragePath(cacheFilePathName);

            virtual void SetUp()
            {
                std::remove(offlineStoragePath.c_str());
            }

            virtual void TearDown()
            {
                std::remove(offlineStoragePath.c_str());
            }
    };

    ECSClientConfiguration GetECSClientConfiguration()
    {
        auto config = ECSClientConfiguration();
        config.clientName = clientName;
        config.clientVersion = "1";
        config.cacheFilePathName = cacheFilePathName;
        config.serverUrls.push_back("https://s2s.config.skype.net/config/v1/");
        return config;
    }

    TEST_F(ECSClientRealworldFuncTests, GetConfigs)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                {
                    auto configs = client->GetConfigs();
                    std::cout<< configs << std::endl;
                    ASSERT_EQ(json::parse(configs)["ECSDemo"]["hit"], std::string("default"));
                }

                // TEST_F(ECSClientRealworldFuncTests, GetETag)
                {
                    auto etag = client->GetETag();
                    std::cout<< etag << std::endl;
                    // etag start with " and end with "
                    // no need to check is it equal because config change will modify the val
                    ASSERT_EQ(0, etag.find("\""));
                    ASSERT_EQ(etag.size()-1, etag.rfind("\""));
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSetting_Int)
                {
                    auto ret = client->GetSetting(agent, "int", 0);
                    ASSERT_EQ(ret, 123);
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSetting_Bool)
                {
                    auto ret = client->GetSetting(agent, "bool", false);
                    ASSERT_EQ(ret, true);
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSetting_Double)
                {
                    auto ret = client->GetSetting(agent, "double", (double)0);
                    EXPECT_DOUBLE_EQ(ret, 1.1);
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSetting_String)
                {
                    auto ret = client->GetSetting(agent, "string", std::string());
                    ASSERT_EQ(ret, std::string("str"));
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSettingsAsInts)
                {
                    auto ret = client->GetSettingsAsInts(agent, "intArray");
                    ASSERT_THAT(ret, ElementsAre(1, 2));
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSettings)
                {
                    auto ret = client->GetSettings(agent, "strArray");
                    ASSERT_THAT(ret, ElementsAre(std::string("hello")));
                }

                // TEST_F(ECSClientRealworldFuncTests, GetSettingsAsDbls)
                {
                    auto ret = client->GetSettingsAsDbls(agent, "dblArray");
                    ASSERT_THAT(ret, ElementsAre(1.1));
                }

                // TEST_F(ECSClientRealworldFuncTests, TryGetSetting_String)
                {
                    std::string val = "";
                    auto ret = client->TryGetSetting(agent, "string", val);
                    ASSERT_EQ(ret, true);
                    ASSERT_EQ(val, "str");
                }

                // TEST_F(ECSClientRealworldFuncTests, TryGetSetting_Long)
                {
                    long val = 0;
                    auto ret = client->TryGetLongSetting(agent, "int", val);
                    ASSERT_EQ(ret, true);
                    ASSERT_EQ(val, 123);
                }

                // TEST_F(ECSClientRealworldFuncTests, TryGetSetting_Bool)
                {
                    bool val = 0;
                    auto ret = client->TryGetBoolSetting(agent, "bool", val);
                    ASSERT_EQ(ret, true);
                    ASSERT_EQ(val, true);
                }

                // TEST_F(ECSClientRealworldFuncTests, TryGetSetting_Int)
                {
                    int val = 0;
                    auto ret = client->TryGetIntSetting(agent, "int", val);
                    ASSERT_EQ(ret, true);
                    ASSERT_EQ(val, 123);
                }

                // TEST_F(ECSClientRealworldFuncTests, TryGetSetting_Double)
                {
                    double val = 0;
                    auto ret = client->TryGetDoubleSetting(agent, "double", val);
                    ASSERT_EQ(ret, true);
                    EXPECT_DOUBLE_EQ(val, 1.1);
                }

                // TEST_F(ECSClientRealworldFuncTests, GetKeys)
                {
                    auto ret = client->GetKeys(agent, "");
                    for (auto& key: std::vector<std::string>{"bool", "dblArray", "double", "int", "intArray", "null", "object", "strArray", "string", "hit"})
                    {
                        ASSERT_NE(std::find(ret.begin(), ret.end(), key), ret.end());
                    }
                }
        });
    }

    TEST_F(ECSClientRealworldFuncTests, SetUserId)
    {
        auto initCallback = [](ECSClient* client){
            client->SetUserId("testuserid");
        };
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "hit", std::string());
                ASSERT_EQ(ret, std::string("userId"));
        }, initCallback);
    }

    TEST_F(ECSClientRealworldFuncTests, SetRequestParameters)
    {
        auto initCallback = [](ECSClient* client){
            client->SetRequestParameters(std::map<std::string, std::string>{{"projectTeam","1DS"}});
        };
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "hit", std::string());
                ASSERT_EQ(ret, std::string("filter"));
        }, initCallback);
    }
}

#endif
