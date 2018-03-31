// Copyright (c) Microsoft. All rights reserved.

#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "common/MockIRuntimeConfig.hpp"
#include <aria/ILogManager.hpp>

using namespace testing;
using namespace ARIASDK_NS;
using namespace resource_manager2;


class VideoStreamSimulator
{
  protected:
    auf::ThreadRef m_thread;

  protected:
    class StreamListener : public BitStream::EventListener
    {
      public:
        virtual void BitStream_allocationsChanged(StreamStatus const& status, BitStream& stream) override
        {
            m_bytesPerSecond = status.proposedBandwidthBps;
        }

        virtual void BitStream_mtuChanged(uint32_t mtu, BitStream& stream)
        {
        }

        uint32_t GetBW() const
        {
            return m_bytesPerSecond;
        }

      private:
        uint32_t m_bytesPerSecond;
    };

  public:
    VideoStreamSimulator()
      : m_thread("VideoStreamSimulator", 0)
    {
    }

    ~VideoStreamSimulator()
    {
        m_thread.stop();
    }

    void start(ResourceManagerPtr rm)
    {
        auf::startThread(m_thread, *this, &VideoStreamSimulator::run, rm);
    }

    void end()
    {
        m_thread.stop();
    }

  protected:
    void run(ResourceManagerPtr rm)
    {
        rm->callStarted("0");

        ConnectionPtr conn = rm->createConnection(true, 0, false, NULL);
        BitStreamPtr stream = rm->createOutboundStream(StreamConfig(), resource_manager2::VideoP2P);

        StreamListener listener;
        stream->addListener(&listener);

        conn->attachStream(stream->getTag());

        uint32_t interval = 200 * 1000; // 200 milliseconds
        uint32_t overhead = 10;
        uint32_t lastBwUpdate = 0;
        while (!m_thread.done()) {
            uint32_t sent = listener.GetBW() / (1000000 / interval);
            uint32_t timestamp = static_cast<uint32_t>(PAL::getMonotonicTimeMs());

            LOG_INFO("VideoStreamSimulator \"sent\" %u bytes", sent);

            uint8_t bwe;
            conn->getBweByteForOutboundPacket(timestamp, bwe);
            static_cast<void>(bwe);
            conn->streamDataSent(timestamp, stream->getTag(), sent, 0, false);
            conn->packetSent(timestamp, overhead, sent, false);

            if (timestamp - lastBwUpdate >= 1000) {
                BandwidthReport report;
                report.bytesPerSecond = 100000; // the total BW is 100kbps
                report.confidence = 15; // the report is totally believed
                conn->processTotalUplinkEstimate(timestamp, report);
                bool marker = false;
                conn->updateOutboundPacketLoss(timestamp, 0, false, marker, NULL, 1);
                lastBwUpdate = timestamp;
            }

            spl::sleep(interval);
        }

        conn->detachStream(stream->getTag());

        stream->removeListener(&listener);

        rm->callEnded("0");
    }
};

//---

class SkypeResourceManagerTests : public ::testing::Test,
                                  public HttpServer::Callback
{
  protected:
    HttpServer                            server;
    std::string                           serverAddress;
    MockIRuntimeConfig                    runtimeConfig;
    std::unique_ptr<ILogManager>          logManager;
    ILogger*                              logger;
    PAL::Event                            requestReceived;
    volatile unsigned                     serverRequestsCount;
    ResourceManagerPtr                    rm;
    std::unique_ptr<VideoStreamSimulator> videoStreamSimulator;

  public:
    SkypeResourceManagerTests()
    {
    }

    virtual void SetUp() override
    {
        int port = server.addListeningPort(0);
        std::ostringstream os;
        os << "http://localhost:" << port << "/";
        serverAddress = os.str();
        server.setServerName(os.str());
        server.addHandler("/", *this);

        auf::init();

        videoStreamSimulator.reset(new VideoStreamSimulator());

        rm = resource_manager2::Factory::createInstance(1, false);

        LogConfiguration configuration;
        configuration.runtimeConfig = &runtimeConfig;
        configuration.skypeResourceManager = rm;
        configuration.cacheFilePath = "SkypeResourceManagerTests.db";
        ::remove(configuration.cacheFilePath.c_str());

        EXPECT_CALL(runtimeConfig, SetDefaultConfig(_)).WillRepeatedly(Return());
        EXPECT_CALL(runtimeConfig, GetCollectorUrl()).WillRepeatedly(Return(serverAddress));
        EXPECT_CALL(runtimeConfig, DecorateEvent(_, _, _)).WillRepeatedly(Return());
        EXPECT_CALL(runtimeConfig, GetEventPriority(_, _)).WillRepeatedly(Return(EventPriority_Unspecified));
        EXPECT_CALL(runtimeConfig, GetMetaStatsSendIntervalSec()).WillRepeatedly(Return(0));
        EXPECT_CALL(runtimeConfig, GetMetaStatsTenantToken()).WillRepeatedly(Return("metastats-token"));
        EXPECT_CALL(runtimeConfig, GetOfflineStorageMaximumSizeBytes()).WillRepeatedly(Return(UINT_MAX));
        EXPECT_CALL(runtimeConfig, GetMaximumRetryCount()).WillRepeatedly(Return(1));
        EXPECT_CALL(runtimeConfig, IsHttpRequestCompressionEnabled()).WillRepeatedly(Return(false));
        EXPECT_CALL(runtimeConfig, GetMinimumUploadBandwidthBps()).WillRepeatedly(Return(512));
        EXPECT_CALL(runtimeConfig, GetMaximumUploadSizeBytes()).WillRepeatedly(Return(1 * 1024 * 1024));

        logManager.reset(ILogManager::Create(configuration));
        logger = logManager->GetLogger("functests", "tenant ID");

        serverRequestsCount = 0;
        server.start();
    }

    virtual void TearDown() override
    {
        logManager->FlushAndTeardown();
        videoStreamSimulator.reset();
        auf::stop();
        server.stop();
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        serverRequestsCount += 1;
#if ARIASDK_PAL_SKYPE
        // Workaround for HTTP Stack weirdness on win-x86_vs2013, to be investigated
        response.content = ".";
#endif
        return 200;
    }

    void waitForRequests(unsigned msec, unsigned count)
    {
        int64_t maxTime = PAL::getMonotonicTimeMs() + msec;
        while (serverRequestsCount < count && PAL::getMonotonicTimeMs() < maxTime) {
            PAL::sleep(100);
        }
    }

    void simulateCallStart()
    {
        videoStreamSimulator->start(rm);
    }

    void simulateCallEnd()
    {
        videoStreamSimulator->end();
    }
};

//---

TEST_F(SkypeResourceManagerTests, ActiveCallSuppressesUpload)
{
    ASSERT_THAT(serverRequestsCount, 0u);

    EventProperties event1("test1");
    logger->LogEvent(event1);

    waitForRequests(5000, 1u);
    ASSERT_THAT(serverRequestsCount, 1u);

    simulateCallStart();

    EventProperties event2("test2");
    event2.SetPriority(EventPriority_High);
    logger->LogEvent(event2);

    waitForRequests(3000, 2u);
    ASSERT_THAT(serverRequestsCount, 1u);

    EventProperties event3("test3");
    logger->LogEvent(event3);

    waitForRequests(3000, 2u);
    ASSERT_THAT(serverRequestsCount, 1u);

    simulateCallEnd();

    waitForRequests(5000, 3u);
    ASSERT_THAT(serverRequestsCount, 3u);
}
