//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"
#ifdef HAVE_MAT_EXP
#include "ECSClientCX.hpp"
#include "LoggerCX.hpp"
#include <iostream>
#include <string>
#include <sstream>

namespace MATW_NS_BEGIN {

    /**
     * Proxy callback converts values from native to managed class.
     */
    void ECSClientCallbackProxy::OnECSClientEvent(
        MAEE::IECSClientCallback::ECSClientEventType evtType,
        MAEE::IECSClientCallback::ECSClientEventContext evtContext) {

        MATW::ECSClientEventType type;
        MATW::ECSClientEventContext context;

        type = static_cast<MATW::ECSClientEventType>(evtType);
        context.clientName = ToPlatformString(evtContext.clientName);
        context.clientVersion = ToPlatformString(evtContext.clientVersion);
        context.configExpiryTimeInSec = evtContext.configExpiryTimeInSec;
        context.configUpdateFromECS = evtContext.configUpdateFromECS;
        context.deviceId = ToPlatformString(evtContext.deviceId);
        context.userId = ToPlatformString(evtContext.userId);

        //CTDEBUGLOG("[ECSClientCX] OnECSClientEvent: evtType=%u", (unsigned)type);

        // Flatten it from key1=value1, key2=value2 to key1=value1&key2=value2
        context.requestParameters = "";
        for (auto param : evtContext.requestParameters) {
            context.requestParameters += ToPlatformString(param.first);
            context.requestParameters += "=";
            context.requestParameters += ToPlatformString(param.second);
            context.requestParameters += "&";
        }
        listener->OnECSClientEvent(type, context);
    }

    ECSClient::ECSClient() {
        m_ecsClient = MAEE::IECSClient::CreateInstance();
    }

    ECSClient::~ECSClient() {
        if (m_ecsClient != nullptr) {
            Microsoft::Applications::Experimentation::ECS::IECSClient **ptr;
#ifdef _WINRT_DLL
            ptr = &m_ecsClient;
#else
            pin_ptr<Microsoft::Applications::Experimentation::ECS::IECSClient *> pinnedPtr = &m_ecsClient;
            ptr = pinnedPtr;
#endif
            MAEE::IECSClient::DestroyInstance(ptr);
        }
    }

    std::vector<std::string> ECSClient::splitString(std::string str, char delimiter)
    {
        std::vector<std::string> internal;
        std::stringstream ss(str);
        std::string token;

        while (getline(ss, token, delimiter))
        {
            internal.push_back(token);
        }

        return internal;
    }

    void ECSClient::Initialize(ECSClientConfiguration config)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::Initialize[...]");
        MAEE::ECSClientConfiguration m_config;
        m_config.cacheFilePathName = std::string(FromPlatformString(config.cacheFilePathName));
        m_config.clientName = std::string(FromPlatformString(config.clientName));
        m_config.clientVersion = std::string(FromPlatformString(config.clientVersion));
        m_config.defaultExpiryTimeInMin = config.defaultExpiryTimeInMin;
        m_config.serverUrls = splitString(FromPlatformString(config.serverUrls), ','); // assume URLs are comma-separated
        m_ecsClient->Initialize(m_config);
    }

    bool ECSClient::AddListener(IECSClientCallback^ listener) {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::AddListener");
        auto callback = std::make_unique<ECSClientCallbackProxy>(listener);
        auto result = m_ecsClient->AddListener(callback.get());
        m_listeners_native.push_back(std::move(callback));
        return result;
    }

    bool ECSClientCallbackProxy::compare(MATW::IECSClientCallback^ inlistener) {
        // CTDEBUGLOG("[ECSClientCX] ECSClientCallbackProxy::compare");
#ifdef _WINRT_DLL
        return (this->listener == inlistener);
#else
        return this->listener->Equals(inlistener);
#endif
    }

    bool ECSClient::RemoveListener(IECSClientCallback ^ listener)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::RemoveListener");
        bool result = false;
        for (auto it = m_listeners_native.begin(); it != m_listeners_native.end(); ++it) {
            const auto& callback = (*it);
            if (callback->compare(listener)) {
                result = m_ecsClient->RemoveListener(callback.get());
                m_listeners_native.erase(it);
            }
        }
        return result;
    }

    bool ECSClient::RegisterLogger(MATW::ILogger ^ pLogger, String ^ agentName)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::RegisterLogger");
        Logger ^logger = static_cast<Logger^>(pLogger);
        const std::string agent = std::string(FromPlatformString(agentName));
        return m_ecsClient->RegisterLogger(logger->m_loggerCore, agent);
    }

    bool ECSClient::SetUserId(String ^ userId)
    {
        const std::string s_userId = FromPlatformString(userId);
        //CTDEBUGLOG("[ECSClientCX] ECSClient::SetUserId: userId=%s", s_userId.c_str());
        return m_ecsClient->SetUserId(s_userId);
    }

    bool ECSClient::SetDeviceId(String ^ deviceId)
    {
        const std::string s_deviceId = FromPlatformString(deviceId);
        //CTDEBUGLOG("[ECSClientCX] ECSClient::SetDeviceId: deviceId=%s", s_deviceId.c_str());
        return m_ecsClient->SetDeviceId(s_deviceId);
    }

    void ECSClient::ClearRequestParams() {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::ClearRequestParams");
        m_requestParams.clear();
        m_ecsClient->SetRequestParameters(m_requestParams);
    }

    bool ECSClient::SetRequestParameter(String^ name, String^ value)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::SetRequestParameter");
        std::string key = FromPlatformString(name);
        std::string val = FromPlatformString(value);
        m_requestParams.emplace(key, val);
        return m_ecsClient->SetRequestParameters(m_requestParams);
    }

    bool ECSClient::Start()
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::Start");
        return m_ecsClient->Start();
    }

    bool ECSClient::Stop()
    {
        // CTDEBUGLOG("[ECSClientCX] ECSClient::Stop");
        return m_ecsClient->Stop();
    }

    bool ECSClient::Suspend()
    {
        // CTDEBUGLOG("[ECSClientCX] ECSClient::Suspend");
        return m_ecsClient->Suspend();
    }

    bool ECSClient::Resume(bool fetchConfig)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::Resume");
        return m_ecsClient->Resume(fetchConfig);
    }

    String^ ECSClient::GetETag()
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetETag");
        return ToPlatformString(m_ecsClient->GetETag());
    }

    String^ ECSClient::GetConfigs()
    {
        return ToPlatformString(m_ecsClient->GetConfigs());
    }

    IPlatformVector^ ECSClient::GetKeys(String ^ agentName, String ^ keysPath)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetKeys");
        IPlatformVector^ result = ToPlatformVector(m_ecsClient->GetKeys(FromPlatformString(agentName), FromPlatformString(keysPath)));
        return result;
    }

    String^ ECSClient::GetSetting(String ^ agentName, String ^ settingPath, String ^ defaultValue)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetSetting[s]");
        std::string name = FromPlatformString(agentName);
        std::string path = FromPlatformString(settingPath);
        std::string value = FromPlatformString(defaultValue);
        return ToPlatformString(m_ecsClient->GetSetting(name, path, value));
    }

    bool ECSClient::GetSetting(String ^ agentName, String ^ settingPath, const bool defaultValue)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetSetting[b]");
        std::string name = FromPlatformString(agentName);
        std::string path = FromPlatformString(settingPath);
        bool result = defaultValue;
        result = m_ecsClient->GetSetting(name, path, result);
        return result;
    }

    int ECSClient::GetSetting(String ^ agentName, String ^ settingPath, const int defaultValue)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetSetting[i]");
        std::string name = FromPlatformString(agentName);
        std::string path = FromPlatformString(settingPath);
        int result = defaultValue;
        result = m_ecsClient->GetSetting(name, path, result);
        return result;
    }

    double ECSClient::GetSetting(String ^ agentName, String ^ settingPath, double defaultValue)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetSetting[d]");
        std::string name = FromPlatformString(agentName);
        std::string path = FromPlatformString(settingPath);
        double result = defaultValue;
        result = m_ecsClient->GetSetting(name, path, result);
        return result;
    }

    IPlatformVector^ ECSClient::GetSettings(String ^ agentName, String ^ settingPath)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClient::GetSettings");
        std::string name = FromPlatformString(agentName);
        std::string path = FromPlatformString(settingPath);
        return ToPlatformVector(m_ecsClient->GetSettings(name, path));
    }

    ECSClientCallbackProxy::ECSClientCallbackProxy(MATW::IECSClientCallback ^ listener)
    {
        //CTDEBUGLOG("[ECSClientCX] ECSClientCallbackProxy::ECSClientCallbackProxy");
        this->listener = listener;
    }

} MATW_NS_END

#endif
