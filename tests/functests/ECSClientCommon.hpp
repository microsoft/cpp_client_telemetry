//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include <chrono>
#include <thread>
#include <iostream>
#include <memory>
#include <functional>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "utils/Utils.hpp"
#include "modules/exp/ecs/ecsclient/ECSClient.hpp"
#include "json.hpp"
#include "utils/StringUtils.hpp"
using json = nlohmann::json;

using namespace testing;
using namespace MAT;
using namespace Microsoft::Applications::Experimentation::ECS;
using namespace Microsoft::Applications::Events;

namespace ECSClientCommon{
    const int maxRetryCount = 50;
    class ECSClientCallback : public IECSClientCallback
    {
        std::function<void()> callback;
    public:
        ECSClientCallback(const std::function<void()> &callback):callback(callback){ }

        virtual void OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext)
        {
            if (evtType == ECSClientEventType::ET_CONFIG_UPDATE_SUCCEEDED)
            {
                callback();
            }
        }
    };

    inline std::unique_ptr<ECSClient> GetInitilizedECSClient(const ECSClientConfiguration& config, const std::function<void(ECSClient*)> &callback = nullptr)
    {
        auto client = std::make_unique<ECSClient>();
        if (callback)
        {
            callback(client.get());
        }
        client->Initialize(config);
        return client;
    }

    inline void InitilizeAndStartECSClientThen(const ECSClientConfiguration& config, const std::function<void(ECSClient*)> &callback, const std::function<void(ECSClient*)> &initCallback = nullptr)
    {
        auto client = GetInitilizedECSClient(config);
        if (initCallback)
        {
            initCallback(client.get());
        }
        auto ret = client->Start();
        ASSERT_EQ(ret, true);
        bool configUpdated = false;
        auto onConfigUpdate = std::make_unique<ECSClientCallback>([&configUpdated](){
            configUpdated = true;
        });
        client->AddListener(onConfigUpdate.get());
        int retryCount = 0;
        while(!configUpdated && retryCount < maxRetryCount){
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ++retryCount;
        };
        if(configUpdated)
        {
            callback(client.get());
        }
        else
        {
            FAIL() << "Config should be updated";
        }
    }
}
#endif
