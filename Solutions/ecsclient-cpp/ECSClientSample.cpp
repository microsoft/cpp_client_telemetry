#include "pch.h"

#include "ECSClientSample.hpp"
#include "ECSClientListener.hpp"
#include "LogManager.hpp"

#include "Utils.hpp"
#include <string>

using namespace std; 
using namespace Microsoft::Applications::Telemetry;
using namespace Microsoft::Applications::Experimentation::ECS;

const std::string tenantToken = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
                              //"d34863e7429e4e569bb90ff06d00e019-404caa7c-ba18-4dae-ac9e-7e08bade9c60-7278";

ILogger* getPrimaryLogger() {
    LogConfiguration configuration;
    configuration.traceLevelMask = 0xFFFFFFFF; // API calls + Global mask for general messages
    configuration.minimumTraceLevel = ACTTraceLevel_Trace;
    return LogManager::Initialize(tenantToken, configuration);
}

void ECSClientSample()
{
    ECSClientListener* pECSClientListener = new ECSClientListener();

    ILogger *pLogger = getPrimaryLogger();
    pECSClientListener->Init(pLogger);

    EventProperties evtProp("EventSetting");
    int setting1 = pECSClientListener->m_pECSClient->GetSetting("SCTTest", "Flight", 0);  
    evtProp.SetProperty("setting1", setting1);
    string setting2 = pECSClientListener->m_pECSClient->GetSetting("AsyncMediaClient", "pes_config", std::string());
    evtProp.SetProperty("setting2", setting2);
    string setting3 = pECSClientListener->m_pECSClient->GetSetting("AsyncMediaClient", "media_params/Audio.1/title", std::string());
    evtProp.SetProperty("setting3", setting3);
    std::vector<std::string> settings4 = pECSClientListener->m_pECSClient->GetSettings("AsyncMediaClient", "storage_limits/imgpsh/format");

    pLogger->LogEvent(evtProp);

    for (int i = 0; i < 2; i++)
    {
        Sleep(500000);
    }

    EventProperties evtProperties("Event1");
    pLogger->LogEvent(evtProperties);

    pECSClientListener->Term();

    delete pECSClientListener;
    pECSClientListener = NULL;

    LogManager::Flush();
}
