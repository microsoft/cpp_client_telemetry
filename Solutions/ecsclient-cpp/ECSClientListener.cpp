#include "pch.h"

#include "ECSClientListener.hpp"
#include "Utils.hpp"

#include <string>
#include <cassert>

using namespace std;
using namespace Microsoft::Applications::Events ;
using namespace Microsoft::Applications::Experimentation::ECS;

const std::string TEST_CLIENT_NAME = "Skype";
const std::string TEST_CLIENT_VERSION = "102909_0.0.0.0";
const std::string TEST_CACHE_FILE_PATH_NAME = "Skype_ecs.dat";
const std::string TEST_AGENT_NAME = "C2CPlugin";

ECSClientListener::ECSClientListener()
    : m_pECSClient(NULL)
{}

ECSClientListener::~ECSClientListener()
{}

void ECSClientListener::Init(ILogger* pLogger)
{
    // ECS PROD
    //config.servers.push_back("https://a.config.skype.com/config/v1/");
    //config.servers.push_back("https://b.config.skype.com/config/v1/");

    // ECS INT
    //config.servers.push_back("https://a.config.skype.net/config/v1/");
    //config.servers.push_back("https://b.config.skype.net/config/v1/");

    ECSClientConfiguration ecsclientConfig;

    ecsclientConfig.clientName          = TEST_CLIENT_NAME;
    ecsclientConfig.clientVersion       = TEST_CLIENT_VERSION;
    ecsclientConfig.cacheFilePathName   = TEST_CACHE_FILE_PATH_NAME;
    ecsclientConfig.serverUrls.push_back("https://a.config.skype.net/config/v1/");// "https://ecsdemo.cloudapp.net/config/v1/");

    m_pECSClient = IECSClient::CreateInstance();
    assert(m_pECSClient != NULL);

    try
    {
        m_pECSClient->Initialize(ecsclientConfig);
    }
    catch (std::exception& e)
    {
        EventProperties prop("test");
        pLogger->LogFailure("Exception", e.what(), prop);
   
        return;
    }

    bool fResult = m_pECSClient->AddListener(this);
    assert(fResult);

    if (pLogger != NULL)
    {
        m_pECSClient->RegisterLogger(pLogger, TEST_AGENT_NAME);
    }
        
	m_pECSClient->SetDeviceId("test");
	m_pECSClient->SetUserId("sanjayga");
    fResult = m_pECSClient->Start();
	
    assert(fResult);
}

void ECSClientListener::Term()
{
    if (m_pECSClient)
    {
        bool fResult = m_pECSClient->RemoveListener(this);
        assert(fResult);

        fResult = m_pECSClient->Stop();
        assert(fResult);

        IECSClient::DestroyInstance(&m_pECSClient);
        assert(m_pECSClient == NULL);
    }
}

// IECSClientCallback
void ECSClientListener::OnECSClientEvent(ECSClientEventType evtType, ECSClientEventContext evtContext)
{
    TraceMsg("callback invoked by ECS Client\n");
    std::string msg = "evtType=";
    msg += std::to_string((unsigned)evtType);
    msg += "\n";
    TraceMsg(msg.c_str());

    if (evtType == IECSClientCallback::ET_CONFIG_UPDATE_SUCCEEDED)
    {
        TraceMsg("ET_CONFIG_UPDATE_SUCCEEDED\n");
        int setting1 = m_pECSClient->GetSetting("SCTTest", "Flight", 0);
        std::string message;
        message += std::to_string(setting1); message += " ,";
        string setting2 = m_pECSClient->GetSetting("UI", "id", std::string());
        message += setting2; message += " ,";
        string setting3 = m_pECSClient->GetSetting("AsyncMediaClient", "media_params/Audio.1/title", std::string());
        message += setting3; message += " ,";
        std::vector<std::string> settings4 = m_pECSClient->GetSettings("AsyncMediaClient", "storage_limits/imgpsh/format");
        for (auto elem : settings4)
        {
            message += elem; message += " ,";
        }
        TraceMsg(msg.c_str());
    }
}
