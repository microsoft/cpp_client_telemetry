// Copyright (c) Microsoft. All rights reserved.
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include <memory>
#include "common/Common.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSClient.hpp"

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;

std::unique_ptr<ECSClient> GetInitializedECSClient()
{
    auto client = std::make_unique<ECSClient>();
    auto config = ECSClientConfiguration();
    config.clientName = "Test";
    config.clientVersion = "1.0";
    config.cacheFilePathName = "cacheFilePathName";

    client->Initialize(config);
    return client;
}

class ECSClientCallback : public IECSClientCallback
{
    std::function<void()> callback;
public:
    ECSClientCallback(std::function<void()> callback = nullptr):callback(callback){ }

    virtual void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext)
    {
        if (evtType == ECSClientEventType::ET_CONFIG_UPDATE_SUCCEEDED)
        {
            callback();
        }
    }
};


TEST(ECSClientTests, Construct_OK)
{
    new ECSClient();
}

TEST(ECSClientTests, Deconstruct_OK)
{
    auto client = new ECSClient();
    delete client;
}

TEST(ECSClientTests, CreateInstance_OK_DestroyInstance_OK)
{
    auto client = IECSClient::CreateInstance();
    IECSClient::DestroyInstance(&client);
}

TEST(ECSClientTests, DestroyInstance_ArgsIsNullptr)
{
    auto client = IECSClient::CreateInstance();
    IECSClient::DestroyInstance(&client);
    IECSClient::DestroyInstance(&client);
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
    auto client = GetInitializedECSClient();
    auto ret = client->Start();
    ASSERT_EQ(ret, true);
}

TEST(ECSClientTests, Stop_OK)
{
    auto client = GetInitializedECSClient();
    auto ret = client->Start();
    ASSERT_EQ(ret, true);
    ret = client->Stop();
    ASSERT_EQ(ret, true);
}

TEST(ECSClientTests, Suspend_OK)
{
    auto client = GetInitializedECSClient();
    auto ret = client->Start();
    ASSERT_EQ(ret, true);
    ret = client->Suspend();
    ASSERT_EQ(ret, true);
}

TEST(ECSClientTests, Resume_OK)
{
    auto client = GetInitializedECSClient();
    auto ret = client->Start();
    ASSERT_EQ(ret, true);
    ret = client->Suspend();
    ASSERT_EQ(ret, true);
    ret = client->Resume();
    ASSERT_EQ(ret, true);
}


TEST(ECSClientTests, AddListener)
{
    auto client = GetInitializedECSClient();
    auto callback = std::make_unique<ECSClientCallback>();
    auto ret = client->AddListener(callback.get());
    ASSERT_EQ(ret, true);
    ret = client->AddListener(callback.get());
    ASSERT_EQ(ret, false);
}

TEST(ECSClientTests, RemoveListener)
{
    auto client = GetInitializedECSClient();
    auto callback = std::make_unique<ECSClientCallback>();
    auto ret = client->RemoveListener(callback.get());
    ASSERT_EQ(ret, false);
    client->AddListener(callback.get());
    ret = client->RemoveListener(callback.get());
    ASSERT_EQ(ret, true);
}

TEST(ECSClientTests, FireClientEvent)
{
    auto client = GetInitializedECSClient();
    bool receiveEvent = false;
    auto callback = std::make_unique<ECSClientCallback>([&receiveEvent](){
        receiveEvent = true;
    });
    auto ret = client->AddListener(callback.get());
    ASSERT_EQ(ret, true);
    ret = client->Start();
    ASSERT_EQ(ret, true);
    client->FireClientEvent(Microsoft::Applications::Experimentation::CommonClientEventType::ET_CONFIG_UPDATE_SUCCEEDED, true);

    const int maxRetryTime = 5;
    int retryTime = 0;
    while(!receiveEvent && retryTime < maxRetryTime){
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ++retryTime;
    };
    ASSERT_EQ(receiveEvent, true);
}

#endif