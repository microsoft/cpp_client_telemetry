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
    const int maxRetryTime = 5;
    const std::string jsonConfigString = "{\"ECSDemo\":{\"intArray\":[1,2],\"dblArray\":[1.1],\"strArray\":[\"hello\"],\"bool\":true,\"int\":123,\"double\":1.0,\"string\":\"str\",\"object\":{\"a\":\"b\",\"c\":\"d\"},\"null\":null},\"Headers\":{\"ETag\":\"\\\"kq49sHJi58LmvklHzjiuJ4UNE/VdiaeFjrYkErAZr1w=\\\"\",\"Expires\":\"Fri, 14 Aug 2020 07:12:13 GMT\",\"CountryCode\":\"JP\",\"StatusCode\":\"200\"}}";
    const std::string jsonConfigStringForFilter = "{}";
    const std::string jsonConfigStringForUserID = "{}";
    const std::string agent = "ECSDemo";
    const std::string userIdHitString = "{\"hit\":\"userId\"}";
    const std::string deviceIdHitString = "{\"hit\":\"clientId\"}";
    const std::string filterHitString = "{\"hit\":\"filter\"}";
    const std::string etagVal = "\"kq49sHJi58LmvklHzjiuJ4UNE/VdiaeFjrYkErAZr1w=\"";
    class ECSServerCallback : public HttpServer::Callback
    {
        public:
            virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
            {
                std::vector<std::string> uriParam;
                std::vector<std::string> params;
                StringUtils::SplitString(request.uri, '?', uriParam);
                if (uriParam.size() == 2)
                {
                    StringUtils::SplitString(uriParam[1], '&', params);
                    for (auto param : params)
                    {
                        std::vector<std::string> kv;
                        StringUtils::SplitString(param, '=', kv);
                        if (kv.size() == 2)
                        {                            
                            if (kv[0] == "id" && kv[1] == "hocai")
                            {
                                response.content = userIdHitString;
                                return 200;
                            }

                            if (kv[0] == "clientId" && kv[1] == "hocaiDevice")
                            {
                                response.content = deviceIdHitString;
                                return 200;
                            }

                            if (kv[0] == "customFilter" && kv[1] == "filterVal")
                            {
                                response.content = filterHitString;
                                return 200;
                            }
                        }
                    }
                }

                response.headers = std::map<std::string, std::string>{{"etag", etagVal}};
                response.content = jsonConfigString;
                return 200;
            }
    };

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

    class ECSClientFuncTests : public ::testing::Test 
    {
        protected:
            HttpServer server;
            ECSServerCallback callback;

            virtual void SetUp()
            {
                server.addListeningPort(8888);
                server.addHandler("/config/v1", callback);
                server.start();
            }

            virtual void TearDown()
            {
                server.stop();            
            }
    };

    std::unique_ptr<ECSClient> GetInitilizedECSClient(std::function<void(ECSClient*)> callback = nullptr)
    {
        auto client = std::make_unique<ECSClient>();
        auto config = ECSClientConfiguration();
        config.clientName = "Test";
        config.clientVersion = "1.0";
        config.cacheFilePathName = "cacheFilePathName";    
        config.serverUrls.push_back("http://127.0.0.1:8888/config/v1");
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
        callback(client.get());
    }

    TEST_F(ECSClientFuncTests, GetConfigs)
    {
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto configs = client->GetConfigs();
            ASSERT_EQ(json::parse(configs), json::parse(jsonConfigString));
        });   
    }

    TEST_F(ECSClientFuncTests, GetETag)
    {   
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto etag = client->GetETag();
            ASSERT_EQ(etag, etagVal);
        });   
    }

    TEST_F(ECSClientFuncTests, GetSetting_Int)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "int", 0);
            ASSERT_EQ(ret, 123);
        });   
    }

    TEST_F(ECSClientFuncTests, GetSetting_Bool)
    {
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "bool", false);
            ASSERT_EQ(ret, true);
        });   
    }

    TEST_F(ECSClientFuncTests, GetSetting_Double)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "double", (double)0);
            EXPECT_DOUBLE_EQ(ret, 1.0);
        });   
    }

    TEST_F(ECSClientFuncTests, GetSetting_String)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSetting(agent, "string", std::string(""));
            ASSERT_EQ(ret, std::string("str"));
        });   
    }

    TEST_F(ECSClientFuncTests, GetSettingsAsInts)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSettingsAsInts(agent, "intArray");
            ASSERT_THAT(ret, ElementsAre(1, 2));
        });   
    }

    TEST_F(ECSClientFuncTests, GetSettings)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSettings(agent, "strArray");
            ASSERT_THAT(ret, ElementsAre(std::string("hello")));
        });   
    }

    TEST_F(ECSClientFuncTests, GetSettingsAsDbls)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetSettingsAsDbls(agent, "dblArray");
            ASSERT_THAT(ret, ElementsAre(1.1));
        });   
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_String)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            std::string val = "";
            auto ret = client->TryGetSetting(agent, "string", val);
            ASSERT_EQ(ret, true);
            ASSERT_EQ(val, "str");
        });   
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Long)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            long val = 0;
            auto ret = client->TryGetLongSetting(agent, "int", val);
            ASSERT_EQ(ret, true);
            ASSERT_EQ(val, 123);
        });   
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Bool)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            bool val = 0;
            auto ret = client->TryGetBoolSetting(agent, "bool", val);
            ASSERT_EQ(ret, true);
            ASSERT_EQ(val, true);
        });   
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Int)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            int val = 0;
            auto ret = client->TryGetIntSetting(agent, "int", val);
            ASSERT_EQ(ret, true);
            ASSERT_EQ(val, 123);
        });   
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Double)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            double val = 0;
            auto ret = client->TryGetDoubleSetting(agent, "double", val);
            ASSERT_EQ(ret, true);
            EXPECT_DOUBLE_EQ(val, 1.0);
        });   
    }

    TEST_F(ECSClientFuncTests, GetKeys)
    {     
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetKeys(agent, "");
            ASSERT_THAT(ret, ElementsAre("bool", "dblArray", "double", "int", "intArray", "null", "object", "strArray", "string"));
        });   
    }

    TEST_F(ECSClientFuncTests, SetUserId)
    {     
        auto initCallback = [](ECSClient* client){
            client->SetUserId("hocai");
        };
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetConfigs();
            ASSERT_EQ(ret, userIdHitString);
        }, initCallback);   
    }

    TEST_F(ECSClientFuncTests, SetDeviceId)
    {     
        auto initCallback = [](ECSClient* client){
            client->SetDeviceId("hocaiDevice");
        };
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetConfigs();
            ASSERT_EQ(ret, deviceIdHitString);
        }, initCallback);   
    }

    TEST_F(ECSClientFuncTests, SetRequestParameters)
    {     
        auto initCallback = [](ECSClient* client){
            client->SetRequestParameters(std::map<std::string, std::string>{{"customFilter","filterVal"}});
        };
        InitilizedAndStartECSClientThen([](ECSClient* client){
            auto ret = client->GetConfigs();
            ASSERT_EQ(ret, filterHitString);
        }, initCallback);   
    }
}

#endif