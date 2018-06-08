#define _CRT_SECURE_NO_WARNINGS

#include "pch.h"

#include "EXPClientListener.hpp"

#include "Utils.hpp"

#include <string>
#include <cassert>

#include <algorithm>    // std::find

using namespace std;
using namespace MAT;
using namespace EXP;

const std::string TEST_CLIENT_NAME = "Skype";
const std::string TEST_CLIENT_VERSION = "102909_0.0.0.0";
const std::string TEST_CACHE_FILE_PATH_NAME = "Skype_ecs.dat";
const std::string TEST_AGENT_NAME = "C2CPlugin";

/**
 * Attach logger to new instance of EXP client
 */
IEXPClient* EXPClientListener::Attach(ILogger* pLogger)
{
    // ECS PROD
    //config.servers.push_back("https://a.config.skype.com/config/v1/");
    //config.servers.push_back("https://b.config.skype.com/config/v1/");

    // ECS INT
    //config.servers.push_back("https://a.config.skype.net/config/v1/");
    //config.servers.push_back("https://b.config.skype.net/config/v1/");

#ifdef USE_ECS
    EXPClientConfiguration config;
    config.clientName = TEST_CLIENT_NAME;
    config.clientVersion = TEST_CLIENT_VERSION;
    config.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;
    config.defaultExpiryTimeInMin = 0;
    // config.serverUrls.push_back("http://127.0.0.1:5002/");
    config.serverUrls.push_back("https://a.config.skype.net/config/v1/");
    config.serverUrls.push_back("https://ecsdemo.cloudapp.net/config/v1/");
    config.enableECSClientTelemetry = false;
#endif

    auto client = IEXPClient::CreateInstance();
    assert(client != NULL);
    m_clients.push_back(client);

    try
    {
        client->Initialize(config);
    }
    catch (std::exception& e)
    {
        if (pLogger != nullptr)
        {
            EventProperties prop("exception");
            pLogger->LogFailure("Exception", e.what(), prop);
        }
        return nullptr;
    }

    bool fResult = client->AddListener(this);
    assert(fResult);

    if (pLogger != NULL)
    {
        client->RegisterLogger(pLogger, TEST_AGENT_NAME);
    }

    client->SetDeviceId("test");
    client->SetUserId("sanjayga");
    fResult = client->Start();
    assert(fResult);

    TraceMsg("created EXP client=%p", client);
    return client;
}

/**
 * Destroy client by pointer passed
 */
void EXPClientListener::Destroy(IEXPClient *client)
{
    bool fResult = client->RemoveListener(this);
    assert(fResult);

    fResult = client->Stop();
    assert(fResult);

    IEXPClient::DestroyInstance(&client);
}

/**
 * Destroy client if it's not destroyed yet
 */
void EXPClientListener::Detach(IEXPClient *client)
{
    auto it = std::find(m_clients.begin(), m_clients.end(), client);
    if (it != std::end(m_clients))
    {
        Destroy(*it);
        m_clients.erase(it);
    }
}

/**
 * Callback executed for all clients
 */
void EXPClientListener::OnEXPEvent(EXPEventType evtType, EXPEventContext evtContext)
{
    TraceMsg("callback invoked by EXP Client, evtType=%d", (unsigned)evtType);

    if (evtType == ET_CONFIG_UPDATE_SUCCEEDED)
    {
        /**
         * Perform GetConfigs for each client
         */
        for (auto client : m_clients)
        {
            TraceMsg("ET_CONFIG_UPDATE_SUCCEEDED for client=%p", client);

            string etag = client->GetETag();
            TraceMsg("ETAG=%s", etag.c_str());

            /*
                {   "AddIn_Weather": {"weatherDayCount":5},
                    "AsyncMediaClient" : {
                        "asl_xmm_enabled":true,
                        "auth_trusted_domains" : "go.skype.com;login.skype.com;*.api.skype.com;*.asm.skype.com;*.asm.skype.net;neu1-api-xmm.cloudapp.net;*.neu1-api-xmm.cloudapp.net;url-preview.cloudapp.net;*.url-preview.cloudapp.net;latest-webclient.skype.com;web.skype.com;mockproxy-asynclib.cloudapp.net;mockproxy-asynclib3.cloudapp.net;mockproxy-asynclib2.cloudapp.net;authgwint.trafficmanager.net;authgw-preqa.cloudapp.net;srm.pre.skype.net;qa
             */
            string config = client->GetConfigs(); // get raw config
            TraceMsg("CONFIG=%s", config.c_str());

#if 0
            int setting1 = client->GetSetting("SCTTest", "Flight", 0);
            std::string message;
            message += std::to_string(setting1); message += " ,";
            string setting2 = client->GetSetting("UI", "id", std::string());

            int setting10 = client->GetSetting("AddIn_Weather", "weatherDayCount", 3);
            setting10 = setting10;
            message += setting2; message += " ,";
            string setting3 = client->GetSetting("AsyncMediaClient", "media_params/Audio.1/title", std::string());
            message += setting3; message += " ,";
            std::vector<std::string> settings4 = client->GetSettings("AsyncMediaClient", "storage_limits/imgpsh/format");
            for (auto elem : settings4)
            {
                message += elem; message += " ,";
            }
#endif

        }
    }
}
