//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#include "common/Common.hpp"
#include "compression/HttpDeflateCompression.hpp"
#include "config/RuntimeConfig_Default.hpp"

#include <utils/ZlibUtils.hpp>
#include "zlib.h"
#undef compress

using namespace testing;
using namespace MAT;

namespace testing {

    void ExpandVector(std::vector<uint8_t> &in, std::vector<uint8_t> &out)
    {
        size_t destLen = out.size();
        std::cout << "size=" << destLen << std::endl;
        char *buffer = nullptr;
        EXPECT_THAT(Expand((const char*)(in.data()), in.size(), &buffer, destLen, false), true);
        out = std::vector<uint8_t>(buffer, buffer + destLen);
        if (buffer)
            delete[] buffer;

    }

}

class HttpDeflateCompressionTests : public StrictMock<Test> {
  protected:
    ILogConfiguration                                                     logConfig;
    RuntimeConfig_Default                                                 config;
    HttpDeflateCompression                                                compression;
    RouteSource<EventsUploadContextPtr const&>                            input;
    RouteSink<HttpDeflateCompressionTests, EventsUploadContextPtr const&> succeeded{this, &HttpDeflateCompressionTests::resultSucceeded};
    RouteSink<HttpDeflateCompressionTests, EventsUploadContextPtr const&> failed{this, &HttpDeflateCompressionTests::resultFailed};

  protected:
    HttpDeflateCompressionTests() :
        config(logConfig),
        compression(config)
    {
        input                            >> compression.compress >> succeeded;
        compression.compressionFailed    >> failed;
    }

    MOCK_METHOD1(resultSucceeded, void(EventsUploadContextPtr const &));
    MOCK_METHOD1(resultFailed,    void(EventsUploadContextPtr const &));
};

static std::vector<uint8_t> testPayload = { 1, 2, 3, 3, 3, 3, 3, 3, 3, 3 };

TEST_F(HttpDeflateCompressionTests, DoesNothingWhenTurnedOff)
{
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = false;
    EventsUploadContextPtr event = std::make_shared<EventsUploadContext>();
    EXPECT_THAT(event->compressed, false);
    event->body = testPayload;

    EXPECT_CALL(*this, resultSucceeded(event)).Times(1);
    input(event);

    EXPECT_THAT(event->body, Eq(testPayload));
    EXPECT_THAT(event->compressed, false);
}

TEST_F(HttpDeflateCompressionTests, CompressesCorrectly)
{
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
    EventsUploadContextPtr event = std::make_shared<EventsUploadContext>();
    EXPECT_THAT(event->compressed, false);
    event->body = testPayload;

    EXPECT_CALL(*this, resultSucceeded(event)).Times(1);
    input(event);

    std::vector<uint8_t> inflated;
    ZlibUtils::InflateVector(event->body, inflated, false);

    EXPECT_THAT(inflated, Eq(testPayload));
    EXPECT_THAT(event->compressed, true);
}

TEST_F(HttpDeflateCompressionTests, WorksMultipleTimes)
{
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
    EventsUploadContextPtr event = std::make_shared<EventsUploadContext>();
    EXPECT_THAT(event->compressed, false);
    event->body = {};
    EXPECT_CALL(*this, resultSucceeded(event)).Times(1);
    input(event);
    EXPECT_THAT(event->body, Eq(std::vector<uint8_t>{0x03, 0x00}));
    EXPECT_THAT(event->compressed, true);

    {
        EventsUploadContextPtr event2 = std::make_shared<EventsUploadContext>();
        EXPECT_THAT(event2->compressed, false);
        event2->body = testPayload;
        EXPECT_CALL(*this, resultSucceeded(event2)).Times(1);
        input(event2);

        std::vector<uint8_t> inflated;
        ZlibUtils::InflateVector(event2->body, inflated, false);
        EXPECT_THAT(inflated, Eq(testPayload));
        EXPECT_THAT(event2->compressed, true);
    }

    {
        std::vector<uint8_t> testPayload2 = {};
        EventsUploadContextPtr event3 = std::make_shared<EventsUploadContext>();
        EXPECT_THAT(event3->compressed, false);
        event3->body = testPayload2;
        EXPECT_CALL(*this, resultSucceeded(event3)).Times(1);
        input(event3);

        std::vector<uint8_t> inflated;
        ZlibUtils::InflateVector(event3->body, inflated, false);
        EXPECT_THAT(inflated, Eq(testPayload2));
        EXPECT_THAT(event3->compressed, true);
    }
}

#pragma warning(push)
#pragma warning(disable:4125)
TEST_F(HttpDeflateCompressionTests, HasReasonableCompressionRatio)
{
#pragma warning( push )
#pragma warning(disable: 4125)

    static char const bond[] =
        "+\n\001I\003sct\215\t\t\001\nservice_id\0011\251$772bcee2-8c19-454b-9af7-99b97ae4afde\321"
        "\006\262\305\361\313\364T\320\a\004\313\b\n\001)$0ac61ae4-ce84-430e-98d3-81adfb88c6a0q"
        "\200\300\361\313\364T\251\006Custom\311\006\016aria_send_test\315\r\t\t\023\rDeviceInfo.I"
        "d\v24062491136\026DeviceInfo.NetworkCost\aUnknown\026DeviceInfo.NetworkType\aUnknown\021D"
        "eviceInfo.OsName\aWindows\024DeviceInfo.OsVersion\aUnknown\020EventInfo.InitId$6bd8d1f9-5"
        "1e9-44fb-b66b-164c12dd3209\016EventInfo.Name\016aria_send_test\024EventInfo.SdkVersion"
        "\032SCT-Windows-C++-No-0.0.0.0\022EventInfo.Sequence\0011\020EventInfo.Source\003sct\016E"
        "ventInfo.Time\0302016-03-23T18:30:41.241Z\rNumberOfClips\00250\026UserInfo.AdvertisingId$"
        "0f67c3ce-89e7-47db-9090-1b9998a100a0\022VideoTimeInSeconds\b5.687200\nVideoTitle\tUSA Tod"
        "ay\016act_sent_count\0010 act_sent_failure_and_retry_count\0010\016act_session_id$39d9160"
        "f-396d-4427-ad76-9dedc5dea386\reventpriority\0012\315\036\t\n\001\020VideoPublisherIdP\02"
        "4i\t123456789\000\000\000\000";
#pragma warning( pop ) 
    size_t const size = sizeof(bond) - 1;

    EventsUploadContextPtr event = std::make_shared<EventsUploadContext>();
    EXPECT_THAT(event->compressed, false);
    event->body.assign(reinterpret_cast<uint8_t const*>(bond), reinterpret_cast<uint8_t const*>(bond) + size);
    EXPECT_CALL(*this, resultSucceeded(event)).Times(1);
    input(event);
    EXPECT_THAT(event->body, SizeIs(Lt(size * 70 / 100)));
    EXPECT_THAT(event->compressed, true);
}
#pragma warning(pop)

TEST_F(HttpDeflateCompressionTests, CompressesGzipCorrectly)
{
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_COMPRESSION] = true;
    config[CFG_MAP_HTTP]["contentEncoding"] = "gzip";
    EventsUploadContextPtr event = std::make_shared<EventsUploadContext>();
    EXPECT_THAT(event->compressed, false);
    event->body = testPayload;

    std::unique_ptr<HttpDeflateCompression> gzipCompression = std::make_unique<HttpDeflateCompression>(config);
    gzipCompression->compress(event);

    std::vector<uint8_t> inflated;
    ZlibUtils::InflateVector(event->body, inflated, true);

    EXPECT_THAT(inflated, Eq(testPayload));
    EXPECT_THAT(event->compressed, true);
    config[CFG_MAP_HTTP]["contentEncoding"] = "deflate";
}
