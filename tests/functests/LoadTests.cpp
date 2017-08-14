// Copyright (c) Microsoft. All rights reserved.

#include "Common/Common.hpp"
#include "common/HttpServer.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include "utils/Utils.hpp"
#include <ILogManager.hpp>
#include <bond_lite/All.hpp>
#include "bond/generated/AriaProtocol_types.hpp"
#include "bond/generated/AriaProtocol_readers.hpp"
#include <fstream>

#ifndef ARIASDK_PAL_SKYPE
#include "pdh.h"
#endif
using namespace testing;
using namespace ARIASDK_NS;

char const* const TEST_STORAGE_FILENAME = "LoadTests.db";


class EventSender : public Thread
{
  public:
    EventSender(ILogger* logger, EventPriority priority)
      : m_logger(logger),
        m_priority(priority)
    {
    }

    void start()
    {
        startThread();
    }

    void stop()
    {
        joinThread();
    }

  protected:
    virtual void onThread() override
    {
        int i = 0;
        while (!shouldTerminate()) {
            EventProperties event("event" + toString(++i));
            event.SetPriority(m_priority);
            event.SetProperty("data", std::string(static_cast<size_t>(m_prg.getRandomDouble() * 3072), '\42'));
            m_logger->LogEvent(event);
            if (i % 5) {
                PAL::sleep(static_cast<unsigned>(5 + m_prg.getRandomDouble() * 100));
            }
        }
    }

  private:
    ILogger*                   m_logger;
    EventPriority              m_priority;
    PAL::PseudoRandomGenerator m_prg;
};

class LoadTests : public Test,
                  public HttpServer::Callback
{
  protected:
    std::string                  serverAddress;
    LogConfiguration             configuration;
    MockIRuntimeConfig           runtimeConfig;
    HttpServer                   server;
    std::unique_ptr<ILogManager> logManager;
    unsigned                     recordsReceived;
    PDH_HQUERY                   cpuQuery;
    PDH_HQUERY                   diskQuery;
    PDH_HCOUNTER                 cpuTotal;
    PDH_HCOUNTER                 diskTotal;

  public:
    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        std::ostringstream os;
        os << "localhost:" << port;
        serverAddress = "http://" + os.str() + "/";
        server.setServerName(os.str());
        server.addHandler("/", *this);

        configuration.runtimeConfig = &runtimeConfig;
        configuration.SetProperty("cacheFilePath", TEST_STORAGE_FILENAME); 
        ::remove(TEST_STORAGE_FILENAME);

        EXPECT_CALL(runtimeConfig, SetDefaultConfig(_)).WillRepeatedly(DoDefault());
        EXPECT_CALL(runtimeConfig, GetCollectorUrl()).WillRepeatedly(Return(serverAddress));
        EXPECT_CALL(runtimeConfig, IsHttpRequestCompressionEnabled()).WillRepeatedly(Return(false));
        EXPECT_CALL(runtimeConfig, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));
        EXPECT_CALL(runtimeConfig, GetEventPriority(_, _)).WillRepeatedly(Return(EventPriority_Unspecified));
        EXPECT_CALL(runtimeConfig, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
        EXPECT_CALL(runtimeConfig, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-tenant-token"));
        EXPECT_CALL(runtimeConfig, GetMaximumRetryCount()).WillRepeatedly(Return(1));
        EXPECT_CALL(runtimeConfig, GetUploadRetryBackoffConfig()).WillRepeatedly(Return("E,3000,3000,2,0"));
        EXPECT_CALL(runtimeConfig, GetMinimumUploadBandwidthBps()).WillRepeatedly(Return(512));
        EXPECT_CALL(runtimeConfig, GetMaximumUploadSizeBytes()).WillRepeatedly(Return(1 * 1024 * 1024));
        EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillRepeatedly(Return());

        logManager.reset(ILogManager::Create(configuration));

        PdhOpenQuery(NULL, NULL, &cpuQuery);
        PdhAddEnglishCounter(cpuQuery, "\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
        PdhCollectQueryData(cpuQuery);

        PdhOpenQuery(NULL, NULL, &diskQuery);
        PdhAddEnglishCounter(cpuQuery, "\\PhysicalDisk(_Total)\\Disk Transfers/sec", NULL, &diskTotal);
        PdhCollectQueryData(diskQuery);

        recordsReceived = 0;
        server.start();
    }

    virtual void TearDown() override
    {
        if (cpuQuery)
        {
            PdhCloseQuery(cpuQuery);
        }

        logManager->FlushAndTeardown();
        ::remove(TEST_STORAGE_FILENAME);
        server.stop();
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        UNREFERENCED_PARAMETER(response);
        auto payload = decodeRequest(request, false);
        for (auto const& packagesPerTenant : payload.TokenToDataPackagesMap) {
            for (auto const& package : packagesPerTenant.second) {
                recordsReceived += static_cast<unsigned int>(package.Records.size());
            }
        }
        return 200;
    }

    AriaProtocol::ClientToCollectorRequest decodeRequest(HttpServer::Request const& request, bool decompress)
    {
        UNREFERENCED_PARAMETER(decompress);
        AriaProtocol::ClientToCollectorRequest result;
        std::vector<uint8_t> input(request.content.data(), request.content.data() + request.content.size());
        bond_lite::CompactBinaryProtocolReader reader(input);
        EXPECT_THAT(bond_lite::Deserialize(reader, result), true);
        return result;
    }

    void waitForTotalRecordsReceived(unsigned timeout, unsigned expectedCount)
    {
        auto start = PAL::getUtcSystemTimeMs();
        while (recordsReceived < expectedCount) {
            if (PAL::getUtcSystemTimeMs() - start >= timeout * 1000) {
                GTEST_FATAL_FAILURE_("Didn't receive request within given timeout");
            }
            PAL::sleep(500);
        }
    }
};

bool isRequiredAvailableMemory()
{
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG fourGB = 4294967296;
    if (memInfo.ullAvailPhys > fourGB)
    {
        return true;
    }
    else
    {
        return false;
    }
}


TEST_F(LoadTests, StartupAndShutdownIsFast)
{
#ifdef ARIASDK_PAL_SKYPE
    // Avoid counting cost of the fat RootTools initialization/shutdown.
    auf::init();
#endif
    
    unsigned const RESTART_COUNT           = 100;
    unsigned int MAX_TIME_PER_RESTART_MS = 250;

    unsigned int maxtimeperrestart = MAX_TIME_PER_RESTART_MS;

    double avgCpuLoad = 0;
    PDH_FMT_COUNTERVALUE counterVal;	
    PDH_FMT_COUNTERVALUE diskVal;
    double avgDiskLoad = 0;
    

    int64_t time = PAL::getMonotonicTimeMs();
    for (unsigned i = 0; i < RESTART_COUNT; i++)
    {
        logManager.reset();
        logManager.reset(ILogManager::Create(configuration));
    }

    time = PAL::getMonotonicTimeMs() - time;
    PdhCollectQueryData(cpuQuery);
    PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
    PdhCollectQueryData(diskQuery);
    PdhGetFormattedCounterValue(diskTotal, PDH_FMT_DOUBLE, NULL, &diskVal);
    avgCpuLoad = avgCpuLoad + counterVal.doubleValue;
    avgDiskLoad = avgDiskLoad + diskVal.doubleValue;

    //printf("\nAvg Processor (_Total) Processor Time = %f", NULL, avgCpuLoad);
    //printf("\navg PhysicalDisk (_Total)  Disk Read Timee = %f", NULL, avgDiskLoad);

    if (avgCpuLoad > 75)
    {
        maxtimeperrestart = MAX_TIME_PER_RESTART_MS + MAX_TIME_PER_RESTART_MS + MAX_TIME_PER_RESTART_MS;
    }
    else if (avgCpuLoad > 40)
    {
        maxtimeperrestart = MAX_TIME_PER_RESTART_MS + MAX_TIME_PER_RESTART_MS;
    }
    
    if (!isRequiredAvailableMemory())
    {
        maxtimeperrestart = maxtimeperrestart + MAX_TIME_PER_RESTART_MS;
    }	
    
    EXPECT_THAT(time / RESTART_COUNT, Lt(maxtimeperrestart));

#ifdef ARIASDK_PAL_SKYPE
    auf::stop();
#endif
}

TEST_F(LoadTests, ManyStartupsAndShutdownsAreHandledSafely)
{
    unsigned const EVENTS_COUNT  = 100;
    unsigned const RESTART_COUNT = 100;

    // Store some events so that system has to do something each time after startup.
    ILogger* logger = logManager->GetLogger("loadtests-tenant-token");
    for (unsigned i = 0; i < EVENTS_COUNT; i++) {
        EventProperties event("event" + toString(i));
        event.SetProperty("data", std::string(1234, '\42'));
        logger->LogEvent(event);
    }

    for (unsigned i = 0; i < RESTART_COUNT; i++) {
        logManager.reset();
        logManager.reset(ILogManager::Create(configuration));
    }

    // All events should eventually come.
    waitForTotalRecordsReceived(60, EVENTS_COUNT + 1); // +1 for meta stats
}

TEST_F(LoadTests, ManyEventsFromManyThreadsAreHandledSafely)
{
    unsigned const THREAD_COUNT     = 8;
    unsigned const TEST_DURATION_MS = 30000;

    EXPECT_CALL(runtimeConfig, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(3));

    ILogger* const logger1 = logManager->GetLogger("loadtests1-tenant-token");
    ILogger* const logger2 = logManager->GetLogger("loadtests2-tenant-token");
    ILogger* logger = logger1;

    std::vector<std::unique_ptr<EventSender>> senders;
    for (unsigned i = 0; i < THREAD_COUNT; i++) {
        senders.emplace_back(new EventSender(logger, EventPriority_High));
        senders.back()->start();
        logger = (logger == logger1) ? logger2 : logger1;
    }

    PAL::sleep(TEST_DURATION_MS);

    for (auto& sender : senders) {
        sender->stop();
    }
    senders.clear();
}
