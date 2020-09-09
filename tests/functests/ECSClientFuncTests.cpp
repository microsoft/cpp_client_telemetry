// Copyright (c) Microsoft. All rights reserved.
#include "ECSClientCommon.hpp"

#ifdef HAVE_MAT_EXP

namespace {
    using namespace ECSClientCommon;
    int port = 8888;
    const char* const jsonConfigString = "{\"ECSDemo\":{\"intArray\":[1,2],\"dblArray\":[1.1],\"strArray\":[\"hello\"],\"bool\":true,\"int\":123,\"double\":1.1,\"string\":\"str\",\"object\":{\"a\":\"b\",\"c\":\"d\"},\"null\":null},\"Headers\":{\"ETag\":\"\\\"kq49sHJi58LmvklHzjiuJ4UNE/VdiaeFjrYkErAZr1w=\\\"\",\"Expires\":\"Fri, 14 Aug 2020 07:12:13 GMT\",\"CountryCode\":\"JP\",\"StatusCode\":\"200\"}}";
    const char* const agent = "ECSDemo";
    const char* const userIdHitString = "{\"hit\":\"userId\"}";
    const char* const deviceIdHitString = "{\"hit\":\"clientId\"}";
    const char* const filterHitString = "{\"hit\":\"filter\"}";
    const char* const etagVal = "\"kq49sHJi58LmvklHzjiuJ4UNE/VdiaeFjrYkErAZr1w=\"";
    const char* const cacheFilePathName = "cacheFilePathName";
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

                            if (kv[0] == "customStatusCode")
                            {
                                response.content = jsonConfigString;
                                return std::stoi(kv[1]);
                            }
                        }
                    }
                }

                response.headers = std::map<std::string, std::string>
                {
                    {"etag", etagVal},
                    {"date", "Thu, 20 Aug 2020 01:28:19 GMT"},
                    {"expires", "Thu, 20 Aug 2020 01:28:19 GMT"}
                };
                response.content = jsonConfigString;
                return 200;
            }
    };

    class ECSClientFuncTests : public ::testing::Test
    {
        protected:
            HttpServer server;
            ECSServerCallback callback;
            const std::string offlineStoragePath = ECSConfigCache::GetStoragePath(cacheFilePathName);

            virtual void SetUp()
            {
                // there are some error on windows
                // looks like port didn't get recycled immediately so we randomize port
                port += 1;
                std:: cout<< "server port:" << port << std::endl;
                server.addListeningPort(port);
                server.addHandler("/config/v1", callback);
                server.start();
                std::remove(offlineStoragePath.c_str());
            }

            virtual void TearDown()
            {
                server.stop();
                std::remove(offlineStoragePath.c_str());
            }
    };

    ECSClientConfiguration GetECSClientConfiguration()
    {
        auto config = ECSClientConfiguration();
        config.clientName = "Test";
        config.clientVersion = "1.0";
        config.cacheFilePathName = cacheFilePathName;
        config.serverUrls.push_back("http://127.0.0.1:" + std::to_string(port) + "/config/v1");
        return config;
    }

    TEST_F(ECSClientFuncTests, GetConfigs)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto configs = client->GetConfigs();
                ASSERT_EQ(json::parse(configs), json::parse(jsonConfigString));
        });
    }

    TEST_F(ECSClientFuncTests, GetETag)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto etag = client->GetETag();
                ASSERT_EQ(etag, etagVal);
        });
    }

    TEST_F(ECSClientFuncTests, GetSetting_Int)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "int", 0);
                ASSERT_EQ(ret, 123);
        });
    }

    TEST_F(ECSClientFuncTests, GetSetting_Bool)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "bool", false);
                ASSERT_EQ(ret, true);
        });
    }

    TEST_F(ECSClientFuncTests, GetSetting_Double)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "double", 0.0);
                EXPECT_DOUBLE_EQ(ret, 1.1);
        });
    }

    TEST_F(ECSClientFuncTests, GetSetting_String)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSetting(agent, "string", std::string());
                ASSERT_EQ(ret, std::string("str"));
        });
    }

    TEST_F(ECSClientFuncTests, GetSettingsAsInts)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSettingsAsInts(agent, "intArray");
                ASSERT_THAT(ret, ElementsAre(1, 2));
        });
    }

    TEST_F(ECSClientFuncTests, GetSettings)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSettings(agent, "strArray");
                ASSERT_THAT(ret, ElementsAre(std::string("hello")));
        });
    }

    TEST_F(ECSClientFuncTests, GetSettingsAsDbls)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetSettingsAsDbls(agent, "dblArray");
                ASSERT_THAT(ret, ElementsAre(1.1));
        });
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_String)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                std::string val;
                auto ret = client->TryGetSetting(agent, "string", val);
                ASSERT_EQ(ret, true);
                ASSERT_EQ(val, "str");
        });
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Long)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                long val = 0;
                auto ret = client->TryGetLongSetting(agent, "int", val);
                ASSERT_EQ(ret, true);
                ASSERT_EQ(val, 123);
        });
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Bool)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                bool val = 0;
                auto ret = client->TryGetBoolSetting(agent, "bool", val);
                ASSERT_EQ(ret, true);
                ASSERT_EQ(val, true);
        });
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Int)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                int val = 0;
                auto ret = client->TryGetIntSetting(agent, "int", val);
                ASSERT_EQ(ret, true);
                ASSERT_EQ(val, 123);
        });
    }

    TEST_F(ECSClientFuncTests, TryGetSetting_Double)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                double val = 0.0;
                auto ret = client->TryGetDoubleSetting(agent, "double", val);
                ASSERT_EQ(ret, true);

                EXPECT_DOUBLE_EQ(val, 1.1);
        });
    }

    TEST_F(ECSClientFuncTests, GetKeys)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetKeys(agent, "");
                ASSERT_THAT(ret, ElementsAre("bool", "dblArray", "double", "int", "intArray", "null", "object", "strArray", "string"));
        });
    }

    TEST_F(ECSClientFuncTests, SetUserId)
    {
        auto initCallback = [](ECSClient* client){
            client->SetUserId("hocai");
        };
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetConfigs();
                ASSERT_EQ(ret, userIdHitString);
        }, initCallback);
    }

    TEST_F(ECSClientFuncTests, SetDeviceId)
    {
        auto initCallback = [](ECSClient* client){
            client->SetDeviceId("hocaiDevice");
        };
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetConfigs();
                ASSERT_EQ(ret, deviceIdHitString);
        }, initCallback);
    }

    TEST_F(ECSClientFuncTests, SetRequestParameters)
    {
        auto initCallback = [](ECSClient* client){
            client->SetRequestParameters(std::map<std::string, std::string>{{"customFilter","filterVal"}});
        };
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto ret = client->GetConfigs();
                ASSERT_EQ(ret, filterHitString);
        }, initCallback);
    }

    TEST_F(ECSClientFuncTests, GetActiveConfigVariant_NotStarted)
    {
        auto client = GetInitilizedECSClient(GetECSClientConfiguration());
        auto configVariant = client->GetActiveConfigVariant();
        ASSERT_EQ(configVariant, json());
    }

    TEST_F(ECSClientFuncTests, GetActiveConfigVariant_Started)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto configVariant = client->GetActiveConfigVariant();
                ASSERT_EQ(configVariant, json::parse(jsonConfigString));
        });
    }

    TEST_F(ECSClientFuncTests, GetExpiryTimeInSec_NotStarted)
    {
        auto client = GetInitilizedECSClient(GetECSClientConfiguration());
        auto expiryTimeInSec = client->GetExpiryTimeInSec();
        ASSERT_EQ(expiryTimeInSec, DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN);
    }

    TEST_F(ECSClientFuncTests, GetExpiryTimeInSec_Started)
    {
        InitilizeAndStartECSClientThen(
            GetECSClientConfiguration(),
            [](ECSClient* client){
                auto expiryTimeInSec = client->GetExpiryTimeInSec();
                // server return expriy time is zero, client will use DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN as expiry time
                ASSERT_EQ(expiryTimeInSec > 0, true);
                ASSERT_LE(expiryTimeInSec, DEFAULT_EXPIRE_INTERVAL_IN_SECONDS_MIN);
        });
    }
}

#endif