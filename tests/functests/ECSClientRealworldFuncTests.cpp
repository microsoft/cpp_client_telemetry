// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <functional>
#include "common/HttpServer.hpp"
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSClient.hpp"
#include "json.hpp"
#include "utils/StringUtils.hpp"
using json = nlohmann::json;

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;
using namespace Microsoft::Applications::Events;

namespace {
    // config: https://ecs.skype.net/?page=ExperimentPage&type=Rollout&id=45994
    const int maxRetryTime = 50; // max wait 50*200 ms
    const std::string clientName = "ECS";
    const std::string agent = "ECSDemo";
    const std::string cacheFilePathName = "cacheFilePathName";

    class ECSClientCallback : public IECSClientCallback
    {
        std::function<void()> callback;
    public:
        ECSClientCallback(std::function<void()> callback):callback(callback){ }

        virtual void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext)
        {
            if (evtType == ECSClientEventType::ET_CONFIG_UPDATE_SUCCEEDED)
            {
                callback();
            }
        }
    };

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

    std::unique_ptr<ECSClient> GetInitilizedECSClient(std::function<void(ECSClient*)> callback = nullptr)
    {
        auto client = std::make_unique<ECSClient>();
        auto config = ECSClientConfiguration();
        config.clientName = clientName;
        config.clientVersion = "1";
        config.cacheFilePathName = cacheFilePathName;
        config.serverUrls.push_back("https://s2s.config.skype.net/config/v1/");
        client->Initialize(config);
        if (callback)
        {
            callback(client.get());
        }
        return client;
    }

    void InitilizedAndStartECSClientThen(std::function<void(ECSClient*)> callback, std::function<void(ECSClient*)> initCallback = nullptr)
    {
        auto client = GetInitilizedECSClient();
        if (initCallback)
        {
            initCallback(client.get());
        }
        auto ret = client->Start();
        ASSERT_EQ(ret, true);
        bool configUpdated = false;
        auto onConfigUpdate = std::make_unique<ECSClientCallback>([&](){
            configUpdated = true;
        });
        client->AddListener(onConfigUpdate.get());
        int retryTime = 0;
        while(!configUpdated && retryTime < maxRetryTime){
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ++retryTime;
        };
        if(configUpdated)
        {
            callback(client.get());
        }
        else
        {
            throw std::runtime_error("Config should be updated");
        }
    }

    TEST_F(ECSClientRealworldFuncTests, GetConfigs)
    {
        InitilizedAndStartECSClientThen([](ECSClient* client){
            {
                auto configs = client->GetConfigs();
                ASSERT_EQ(json::parse(configs)["ECSDemo"]["hit"], std::string("default"));
            }

            // TEST_F(ECSClientRealworldFuncTests, GetETag)
            {
                auto etag = client->GetETag();
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
            client->SetUserId("hocai");
        };
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "hit", std::string());
            ASSERT_EQ(ret, std::string("userId"));
        }, initCallback);
    }

    TEST_F(ECSClientRealworldFuncTests, SetRequestParameters)
    {
        auto initCallback = [](ECSClient* client){
            client->SetRequestParameters(std::map<std::string, std::string>{{"projectTeam","1DS"}});
        };
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "hit", std::string());
            ASSERT_EQ(ret, std::string("filter"));
        }, initCallback);
    }
}

#endif