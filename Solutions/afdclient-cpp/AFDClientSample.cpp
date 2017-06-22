#include "pch.h"

#include "AFDClientSample.hpp"
#include "AFDClientListener.hpp"
#include "LogManager.hpp"

#include "Utils.hpp"
#include <string>

using namespace std; 
using namespace Microsoft::Applications::Telemetry;
using namespace Microsoft::Applications::Experimentation::AFD;

const std::string tenantToken = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
                              //"d34863e7429e4e569bb90ff06d00e019-404caa7c-ba18-4dae-ac9e-7e08bade9c60-7278";

ILogger* getPrimaryLogger() {
    LogConfiguration configuration;
    configuration.traceLevelMask = 0xFFFFFFFF; // API calls + Global mask for general messages
    configuration.minimumTraceLevel = ACTTraceLevel_Trace;
    return LogManager::Initialize(tenantToken, configuration);
}

void AFDClientSample()
{
    AFDClientListener* pAFDClientListener = new AFDClientListener();

    ILogger *pLogger = getPrimaryLogger();
    pAFDClientListener->Init(pLogger);

    for (int i = 0; i < 2; i++)
    {
        Sleep(500000);
    }

    EventProperties evtProperties("Event1");
    pLogger->LogEvent(evtProperties);

    pAFDClientListener->Term();

    delete pAFDClientListener;
    pAFDClientListener = NULL;

    LogManager::Flush();
}
