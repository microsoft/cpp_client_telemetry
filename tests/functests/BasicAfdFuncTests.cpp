// Copyright (c) Microsoft. All rights reserved.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "pal/UtcHelpers.hpp"
#include "common/Common.hpp"
#include "common/HttpServer.hpp"

#include "utils/Utils.hpp"
#include <api/LogManagerImpl.hpp>

#include <IAFDClient.hpp>
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include <fstream>
#include <string>

using namespace testing;
using namespace ARIASDK_NS;
using namespace Microsoft::Applications::Experimentation::AFD;
char const* const TEST_STORAGE_FILENAME = "BasicFuncTests.db";
char const* const TEST_CLIENT_NAME = "";
char const* const TEST_CLIENT_VERSION = "1_1.1.1.1";
char const* const TEST_CACHE_FILE_PATH_NAME = "BasicAfdFuncTests.db";

class AFDClientListener : public IAFDClientCallback
{
public:
    AFDClientListener()
    {}
    ~AFDClientListener(){}

    // IAFDClientCallback
    virtual void OnAFDClientEvent(IAFDClientCallback::AFDClientEventType evtType, IAFDClientCallback::AFDClientEventContext evtContext)
    {
        features.clear();
        flights.clear();

        if (evtType == IAFDClientCallback::ET_CONFIG_UPDATE_SUCCEEDED)
        {
            if (evtContext.flights.size() > 0)
            {
                std::vector<std::string>::iterator iter;
                for (iter = evtContext.flights.begin(); iter < evtContext.flights.end(); iter++)
                {
                    flights.push_back(*iter);
                    std::string temp = *iter;
                }
            }

            if (evtContext.features.size() > 0)
            {
                std::vector<std::string>::iterator iter;
                for (iter = evtContext.features.begin(); iter < evtContext.features.end(); iter++)
                {
                    features.push_back(*iter);
                    std::string temp = *iter;
                }
            }
        }
    }

public:
    std::vector<std::string> features;
    std::vector<std::string> flights;
    
};

class BasicAfdFuncTests : public ::testing::Test,
                          public HttpServer::Callback
{
  protected:
    std::vector<HttpServer::Request> receivedRequests;
    std::string serverAddress;
    HttpServer server;
    std::unique_ptr<LogManagerImpl> logManager;
    ILogger* logger;
    ILogger* logger2;
    IAFDClient* m_pAFDClient;
    AFDClientListener listner;
    std::ostringstream os;
    ILogConfiguration configuration;

  public:
    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        os.clear();
        os << "localhost:" << port;
        serverAddress = "http://" + os.str() + "/simple/";
        server.setServerName(os.str());
        server.addHandler("/simple", *this);
        server.addHandler("/slow", *this);
        server.addHandler("/503", *this);
        server.addHandler("/afdHeader", *this);
        server.addHandler("/afd", *this);
        server.addHandler("/afd503", *this);

        configuration[CFG_INT_RAM_QUEUE_SIZE] = 4096 * 20;
        configuration["cacheFilePath"] = TEST_STORAGE_FILENAME;
        ::remove(configuration["cacheFilePath"]);

        m_pAFDClient = IAFDClient::CreateInstance();

        logManager.reset((LogManagerImpl*)LogManagerImpl::Create(configuration));

        // FIXME: [MG] - TOKEN must be a TOKEN, not a random string...
        // So ideally we should assert right here.
        logger = logManager->GetLogger("functests-Tenant-Token", "source");
        logger2 = logManager->GetLogger("FuncTests2-tenant-token", "Source");

        m_pAFDClient->AddListener(&listner);
        m_pAFDClient->RegisterLogger(logger, TEST_CLIENT_NAME);
        
        server.start();
        listner.flights.clear();
        listner.features.clear();
    }

    virtual void TearDown() override
    {
        ::remove(TEST_STORAGE_FILENAME);
        server.stop();
       

        try
        {
            IAFDClient::DestroyInstance(&m_pAFDClient);
            m_pAFDClient = NULL;
        }
        catch (...)
        {
            printf("exception in IAFDClient::DestroyInstance(&m_pAFDClient);");
        }
        
        PAL::sleep(100);

        try
        {
            logManager->FlushAndTeardown();
        }
        catch (...)
        {
            printf("exception in logManager->FlushAndTeardown();");
        }
        std::string storagePath(TEST_CACHE_FILE_PATH_NAME);

        if (storagePath.find(PATH_SEPARATOR_CHAR) == std::string::npos)
        {
            std::string tempDirectroryPath = PAL::GetAppLocalTempDirectory();
            if (!tempDirectroryPath.empty())
            {
                storagePath = tempDirectroryPath + storagePath;
            }
        }

        int* xPtr = nullptr;
        int IntptrSize = sizeof(xPtr);
        if (IntptrSize > 4) // on 64 bit system, we want session to have different file because FIFO has trouble opening 32 bit file in 64 bit mode
        {
            storagePath = storagePath + "64";
        }
        ::remove(storagePath.c_str());
        UNREFERENCED_PARAMETER(xPtr);
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        if (request.uri.compare("/503/") == 0)
        {
            return 503;
        }

        if (request.uri.compare("/slow/") == 0)
        {
            PAL::sleep(static_cast<unsigned int>(request.content.size() / DELAY_FACTOR_FOR_SERVER));
        }

        if (request.uri.compare("/afd503") == 0)
        {
            response.headers["X-MSEdge-Ref"] = "Ref A: \"c400d03ce02084344ad6b8e98d500406\"  c400d03ce02084344ad6b8e98d500406";
            response.headers["ETag"] = "etag";

            receivedRequests.push_back(request);

            response.headers["Content-Type"] = "text/plain";
            response.content = "";

            return 503;
        }

        if (request.uri.compare("/afdHeader") == 0)
        {
            //add features to response
            response.headers["X-MSEdge-Features"] = "msft-internalt1,otelaatestc,ofzim3zlrl3dbh01"; 
            //add flights to response
            response.headers["X-MSEdge-Flight"] = "preallocation=allexpusers,muidflt120=msft-internalt1,muidflt192=otelaatestc,muidflt238=ofzim3zlrl3dbh01";
            response.headers["X-MSEdge-Ref"] = "Ref A: \"c400d03ce02084344ad6b8e98d500406\"  c400d03ce02084344ad6b8e98d500406";
            response.headers["ETag"] = "etag";

            receivedRequests.push_back(request);

            response.headers["Content-Type"] = "text/plain";
            response.content = "";
            
            return 200;
        }

        if (request.uri.compare("/afd") == 0)
        {
            response.headers["X-MSEdge-Ref"] = "Ref A: \"c400d03ce02084344ad6b8e98d500406\"  c400d03ce02084344ad6b8e98d500406";
            response.headers["ETag"] = "etag";

            receivedRequests.push_back(request);

            response.headers["Content-Type"] = "text/plain";
            //add features and flights to response
            response.content = "{\"Features\":[\"heads\",\"rt-afdcpv01c\",\"rt-afdcpv02c\",\"rt-afdcpv03t\",\"control123\",\"twithig\"],\"Flights\":{\"rt-validation01\":\"rt-afdcpv01c\",\"rt-validation02\":\"rt-afdcpv02c\",\"rt-validation03\":\"rt-afdcpv03t\"},\"Configs\":[{\"Id\":\"Flight_tryexp2rows8columns\",\"Parameters\":{}},{\"Id\":\"Flight_tryexptvdemo3rows\",\"Parameters\":{}},{\"Id\":\"Flight_tryexptvdemodefault\",\"Parameters\":{}},{\"Id\":\"TryExP\",\"Parameters\":{}}],\"FlightingVersion\":505,\"ImpressionId\":\"F45813A6A5D049AE9C1EDA79E1FBE8D0\"}";

            return 200;
        }

        receivedRequests.push_back(request);

        response.headers["Content-Type"] = "text/plain";
        response.content = "It works!";

        return 200;
    }

    void waitForRequests(unsigned timeout, unsigned expected_count = 1)
    {
        auto sz = receivedRequests.size();
        auto start = PAL::getUtcSystemTime();
        while (receivedRequests.size() - sz < expected_count) {
            if (PAL::getUtcSystemTime() - start >= timeout) {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
            PAL::sleep(100);
        }
    }
 };


TEST_F(BasicAfdFuncTests, getAfdDataAsHeaders)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;

    serverAddress = "http://" + os.str() + "/afdHeader/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 3);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "msft-internalt1");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "otelaatestc");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "ofzim3zlrl3dbh01");

    EXPECT_THAT(listner.flights.size(), 4);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "preallocation=allexpusers");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "muidflt120=msft-internalt1");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "muidflt192=otelaatestc");
    if (listner.flights.size() > 3)
        EXPECT_THAT(listner.flights[3], "muidflt238=ofzim3zlrl3dbh01");

    m_pAFDClient->Stop();
}

TEST_F(BasicAfdFuncTests, getAfdDataInBody)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;

    serverAddress = "http://" + os.str() + "/afd/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);


    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");
    
    m_pAFDClient->Stop();

}

TEST_F(BasicAfdFuncTests, getAfdDataFailure)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;

    serverAddress = "http://" + os.str() + "/afd503/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);

    EXPECT_THAT(listner.flights.size(), 0);
    EXPECT_THAT(listner.features.size(), 0);
    
    m_pAFDClient->Stop();
   
}

TEST_F(BasicAfdFuncTests, getAfdDataAfterTime)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;
    afdclientConfig.defaultExpiryTimeInMin = 1;

    serverAddress = "http://" + os.str() + "/afd/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    PAL::sleep(2000);

    listner.features = m_pAFDClient->GetFeatures();
    listner.flights = m_pAFDClient->GetFlights();

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    m_pAFDClient->Stop();
}

TEST_F(BasicAfdFuncTests, getAfdDataAfterRestart)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;
    afdclientConfig.defaultExpiryTimeInMin = 1;

    serverAddress = "http://" + os.str() + "/afd/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    m_pAFDClient->Stop();
    
    try
    {
        IAFDClient::DestroyInstance(&m_pAFDClient);
        m_pAFDClient = NULL;
    }
    catch (...)
    {
        printf("exception in IAFDClient::DestroyInstance(&m_pAFDClient);");
    }


    PAL::sleep(1 * 1000); 

    listner.flights.clear();
    listner.features.clear();

    m_pAFDClient = IAFDClient::CreateInstance();

    m_pAFDClient->AddListener(&listner);
    m_pAFDClient->RegisterLogger(logger, TEST_CLIENT_NAME);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();
                          
    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    m_pAFDClient->Stop();
}

TEST_F(BasicAfdFuncTests, getAfdDataAfterSuspendResume)
{
    AFDClientConfiguration afdclientConfig;

    afdclientConfig.clientId = TEST_CLIENT_NAME;
    afdclientConfig.clientVersion = TEST_CLIENT_VERSION;
    afdclientConfig.cacheFilePathName = TEST_CACHE_FILE_PATH_NAME;
    afdclientConfig.defaultExpiryTimeInMin = 1;

    serverAddress = "http://" + os.str() + "/afd/";

    afdclientConfig.serverUrls.push_back(serverAddress);

    m_pAFDClient->Initialize(afdclientConfig);
    m_pAFDClient->Start();

    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    m_pAFDClient->Suspend();
    listner.features.clear();
    listner.flights.clear();
    
    PAL::sleep(2000);
    m_pAFDClient->Resume();

    waitForRequests(50);

    EXPECT_THAT(listner.features.size(), 6);
    if (listner.features.size() > 0)
        EXPECT_THAT(listner.features[0], "heads");
    if (listner.features.size() > 1)
        EXPECT_THAT(listner.features[1], "rt-afdcpv01c");
    if (listner.features.size() > 2)
        EXPECT_THAT(listner.features[2], "rt-afdcpv02c");
    if (listner.features.size() > 3)
        EXPECT_THAT(listner.features[3], "rt-afdcpv03t");
    if (listner.features.size() > 4)
        EXPECT_THAT(listner.features[4], "control123");
    if (listner.features.size() > 5)
        EXPECT_THAT(listner.features[5], "twithig");

    EXPECT_THAT(listner.flights.size(), 3);
    if (listner.flights.size() > 0)
        EXPECT_THAT(listner.flights[0], "rt-validation01=rt-afdcpv01c");
    if (listner.flights.size() > 1)
        EXPECT_THAT(listner.flights[1], "rt-validation02=rt-afdcpv02c");
    if (listner.flights.size() > 2)
        EXPECT_THAT(listner.flights[2], "rt-validation03=rt-afdcpv03t");

    m_pAFDClient->Stop();
}