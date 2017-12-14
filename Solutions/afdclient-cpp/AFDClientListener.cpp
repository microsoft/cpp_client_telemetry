#include "pch.h"

#include "AFDClientListener.hpp"
#include "Utils.hpp"

#include <string>
#include <cassert>

using namespace std;
using namespace Microsoft::Applications::Events ;
using namespace Microsoft::Applications::Experimentation::AFD;

const std::string TEST_CLIENT_NAME = "word3";// "C2CPlugin";
const std::string TEST_CLIENT_VERSION = "1.0.0.0";
const std::string TEST_CACHE_FILE_PATH_NAME = "Skype_afd.dat";
const std::string TEST_AGENT_NAME = "word";// "C2CPlugin";

AFDClientListener::AFDClientListener()
    : m_pAFDClient(NULL)
{}

AFDClientListener::~AFDClientListener()
{}

void AFDClientListener::Init(ILogger* pLogger)
{
    // AFD PROD
    //config.servers.push_back("https://a.config.skype.com/config/v1/");
    //config.servers.push_back("https://b.config.skype.com/config/v1/");

    // AFD INT
    //config.servers.push_back("https://a.config.skype.net/config/v1/");
    //config.servers.push_back("https://b.config.skype.net/config/v1/");

    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId            = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion       = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName   = TEST_CACHE_FILE_PATH_NAME;
    //afdclientConfig.corpnet             = 1;  //0 means look like it is from outside of corpnet
    afdclientConfig.serverUrls.push_back("https://ocos-office365-s2s.msedge.net/ab"); //  "http://test-exp-s2s.msedge.net/ab/");

    m_pAFDClient = IAFDClient::CreateInstance();
    assert(m_pAFDClient != NULL);

    try
    {
        m_pAFDClient->Initialize(afdclientConfig);
    }
    catch (std::exception& e)
    {
        EventProperties prop("test");
        pLogger->LogFailure("Exception",e.what(), prop);
       
        return;
    }

    bool fResult = m_pAFDClient->AddListener(this);
    assert(fResult);

    if (pLogger != NULL)
    {
        m_pAFDClient->RegisterLogger(pLogger, TEST_AGENT_NAME);
    }
        
    fResult = m_pAFDClient->Start();
    assert(fResult);
}

void AFDClientListener::Term()
{
    if (m_pAFDClient)
    {
        bool fResult = m_pAFDClient->RemoveListener(this);
        assert(fResult);

        fResult = m_pAFDClient->Stop();
        assert(fResult);

        IAFDClient::DestroyInstance(&m_pAFDClient);
        assert(m_pAFDClient == NULL);
    }
}

// IAFDClientCallback
void AFDClientListener::OnAFDClientEvent(AFDClientEventType evtType, AFDClientEventContext evtContext)
{
    TraceMsg("callback invoked by AFD Client\n");
    std::string msg = "evtType=";
    msg += std::to_string((unsigned)evtType);
    msg += "\n";
    TraceMsg(msg.c_str());

	if (evtType == IAFDClientCallback::ET_CONFIG_UPDATE_SUCCEEDED)
	{
		TraceMsg("ET_CONFIG_UPDATE_SUCCEEDED\n");
		TraceMsg("\nRunning Flights\n");
		if (evtContext.flights.size() > 0)
		{
			std::vector<std::string>::iterator iter;
			for (iter = evtContext.flights.begin(); iter < evtContext.flights.end(); iter++)
			{
				std::string temp = *iter;
				TraceMsg("\n");
				TraceMsg(temp.c_str());
			}
		}
		TraceMsg("\n");

		TraceMsg("\nRunning Features\n");
		if (evtContext.features.size() > 0)
		{
			std::vector<std::string>::iterator iter;
			for (iter = evtContext.features.begin(); iter < evtContext.features.end(); iter++)
			{
				std::string temp = *iter;
				TraceMsg("\n");
				TraceMsg(temp.c_str());
			}
		}
		TraceMsg("\n\n");
    }
}
