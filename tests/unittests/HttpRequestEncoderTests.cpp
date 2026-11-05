// Copyright (c) Microsoft Corporation. All rights reserved.

#include "api/AuthTokensController.hpp"
#include "common/Common.hpp"
#include "common/MockIHttpClient.hpp"
#include "common/MockITelemetrySystem.hpp"
#include "config/RuntimeConfig_Default.hpp"
#include "decorators/BaseDecorator.hpp"
#include "http/HttpRequestEncoder.hpp"

#include <future>

using namespace testing;
using namespace MAT;


class MockHttpRequestEncoder : public HttpRequestEncoder
{
public:
    MockHttpRequestEncoder(ITelemetrySystem& system, IHttpClient& httpClient)
        : HttpRequestEncoder(system, httpClient) { }

    using HttpRequestEncoder::handleEncode;
    
    void DispatchDataViewerEvent(const StorageBlob& packet)
    {
        dataPacket = packet;
    }

    StorageBlob dataPacket;
};

class HttpRequestEncoderTests : public Test {

public:

protected:
    ITelemetrySystem & system;
    MockIHttpClient    mockHttpClient;
    HttpRequestEncoder encoder;

protected:

    HttpRequestEncoderTests()
        : system(testing::getSystem()),
        encoder(system, mockHttpClient)
    {
        EXPECT_CALL(mockHttpClient, CreateRequest())
            .WillRepeatedly(Invoke(&HttpRequestEncoderTests::createHttpRequest));
    }

    static IHttpRequest* createHttpRequest()
    {
        return new SimpleHttpRequest("HttpRequestEncoderTests");
    }
};

TEST_F(HttpRequestEncoderTests, SetsAllParameters)
{
    EventsUploadContextPtr ctx = std::make_shared<EventsUploadContext>();
    ctx->compressed = false;
    ctx->body = { 1, 127, 255 };
    ctx->packageIds["tenant1-token"] = 0;
    ctx->latency = EventLatency_RealTime;

    encoder.encode(ctx);

    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    SimpleHttpRequest const* req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);

    EXPECT_THAT(req->m_id, Eq("HttpRequestEncoderTests"));
    EXPECT_THAT(req->m_method, Eq("POST"));
    EXPECT_THAT(req->m_url, Eq("https://mobile.events.data.microsoft.com/OneCollector/1.0/"));
    EXPECT_THAT(req->m_headers, Contains(Pair("Expect", "100-continue")));
    EXPECT_THAT(req->m_headers, Contains(Pair("SDK-Version", PAL::getSdkVersion())));
    EXPECT_THAT(req->m_headers, Contains(Pair("Client-Id", "NO_AUTH")));
    EXPECT_THAT(req->m_headers, Contains(Pair("Content-Type", "application/bond-compact-binary")));
    EXPECT_THAT(req->m_headers, Contains(Pair("APIKey", "tenant1-token")));
    EXPECT_THAT(req->m_body, Eq(std::vector<uint8_t>{1, 127, 255}));
    EXPECT_THAT(req->m_latency, Eq(EventLatency_RealTime));
}

TEST_F(HttpRequestEncoderTests, AddsCompressionHeader)
{
    EventsUploadContextPtr ctx = std::make_shared<EventsUploadContext>();

    ctx->compressed = false;
    encoder.encode(ctx);
    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    SimpleHttpRequest const* req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
    EXPECT_THAT(req->m_headers, Not(Contains(Pair("Content-Encoding", "deflate"))));

    ctx->compressed = true;
    encoder.encode(ctx);
    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
    EXPECT_THAT(req->m_headers, Contains(Pair("Content-Encoding", "deflate")));
}

TEST_F(HttpRequestEncoderTests, BuildsApiKeyCorrectly)
{
    EventsUploadContextPtr ctx = std::make_shared<EventsUploadContext>();

    encoder.encode(ctx);
    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    SimpleHttpRequest const* req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
    EXPECT_THAT(req->m_headers, Contains(Pair("APIKey", "")));

    ctx->packageIds["tenant1-token"] = 0;
    encoder.encode(ctx);
    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
    EXPECT_THAT(req->m_headers, Contains(Pair("APIKey", "tenant1-token")));

    ctx->packageIds["tenant2-token"] = 1;
    ctx->packageIds["tenant3-token"] = 2;
    encoder.encode(ctx);
    ASSERT_THAT(ctx->httpRequestId, Eq("HttpRequestEncoderTests"));
    req = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
    EXPECT_THAT(req->m_headers, Contains(Pair("APIKey", "tenant1-token,tenant2-token,tenant3-token")));
}

TEST_F(HttpRequestEncoderTests, DispatchDataViewerEventCorrectly)
{
    EventsUploadContextPtr ctx = std::make_shared<EventsUploadContext>();
    ctx->body = { 1, 127, 255 };

    MockHttpRequestEncoder mockEncoder(system, mockHttpClient);

    mockEncoder.encode(ctx);

    EXPECT_THAT(mockEncoder.dataPacket, Eq(std::vector<uint8_t>{1, 127, 255}));
}

namespace
{
    class TokenLogManager : public NullLogManager
    {
       public:
        AuthTokensController tokens;

        IAuthTokensController* GetAuthTokensController() override
        {
            return &tokens;
        }
    };

    class TokenTelemetrySystem : public MockITelemetrySystem
    {
       public:
        TokenLogManager manager;

        ILogManager& getLogManager() override
        {
            return manager;
        }
    };
}

TEST(AuthTokensControllerTests, RefreshingUserTokensDoesNotDuplicateTicketKeys)
{
    TokenLogManager manager;
    auto& tokens = manager.tokens;
    const TicketType userTypes[] = {TicketType_MSA_User, TicketType_XAuth_User, TicketType_AAD_User};
    for (int refresh = 0; refresh < 100; ++refresh)
    {
        const auto value = std::to_string(refresh);
        for (auto type : userTypes)
        {
            ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(type, value.c_str()));
        }
        ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_AAD, value.c_str()));
    }

    const std::vector<std::string> expectedTickets{"10001", "10003", "10005"};
    EXPECT_EQ(expectedTickets, tokens.GetTickets());
    for (auto type : userTypes)
    {
        EXPECT_EQ("99", tokens.GetUserTokens().at(type));
    }
    EXPECT_EQ("99", tokens.GetDeviceTokens().at(TicketType_AAD));

    BaseDecorator decorator(manager);
    CsProtocol::Record record;
    ASSERT_TRUE(decorator.decorate(record));
    ASSERT_EQ(1u, record.extProtocol.size());
    ASSERT_EQ(1u, record.extProtocol[0].ticketKeys.size());
    EXPECT_EQ(expectedTickets, record.extProtocol[0].ticketKeys[0]);

    ASSERT_EQ(STATUS_SUCCESS, tokens.Clear());
    EXPECT_TRUE(tokens.GetTickets().empty());
    EXPECT_TRUE(tokens.GetUserTokens().empty());
    EXPECT_TRUE(tokens.GetDeviceTokens().empty());
    ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_MSA_User, "after-clear"));
    ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_MSA_User, "refreshed"));
    EXPECT_THAT(tokens.GetTickets(), ElementsAre("10001"));
    EXPECT_EQ("refreshed", tokens.GetUserTokens().at(TicketType_MSA_User));
}

TEST_F(HttpRequestEncoderTests, TokenRefreshIsSafeDuringEncodingAndDecoration)
{
    TokenTelemetrySystem tokenSystem;
    auto& tokens = tokenSystem.manager.tokens;
    const std::string firstValue(256, 'a');
    const std::string secondValue(256, 'b');
    ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_MSA_User, firstValue.c_str()));
    ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_XAuth_User, firstValue.c_str()));
    ASSERT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_AAD, firstValue.c_str()));

    HttpRequestEncoder tokenEncoder(tokenSystem, mockHttpClient);
    BaseDecorator decorator(tokenSystem.manager);
    std::promise<void> start;
    auto ready = start.get_future().share();
    auto firstWriter = std::async(std::launch::async, [&]()
                                  {
        ready.wait();
        for (int refresh = 0; refresh < 2000; ++refresh)
        {
            const auto& value = refresh % 2 == 0 ? firstValue : secondValue;
            EXPECT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_MSA_User, value.c_str()));
            EXPECT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_AAD, value.c_str()));
        } });
    auto secondWriter = std::async(std::launch::async, [&]()
                                   {
        ready.wait();
        for (int refresh = 0; refresh < 2000; ++refresh)
        {
            const auto& value = refresh % 2 == 0 ? firstValue : secondValue;
            EXPECT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_XAuth_User, value.c_str()));
        } });

    const auto ticketHeader = [](const std::string& msa, const std::string& xauth)
    {
        return "\"10001\"=\"p:" + msa + "\";\"10003\"=\"x:XBL3.0 x=" + xauth + "\"";
    };
    start.set_value();
    for (int event = 0; event < 1000; ++event)
    {
        auto ctx = std::make_shared<EventsUploadContext>();
        tokenEncoder.encode(ctx);
        const auto* request = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
        EXPECT_THAT(request->m_headers, Contains(Pair("Aad-Token", AnyOf(Eq(firstValue), Eq(secondValue)))));
        EXPECT_THAT(request->m_headers, Contains(Pair("Tickets",
                                                      AnyOf(Eq(ticketHeader(firstValue, firstValue)), Eq(ticketHeader(firstValue, secondValue)),
                                                            Eq(ticketHeader(secondValue, firstValue)), Eq(ticketHeader(secondValue, secondValue))))));

        CsProtocol::Record record;
        ASSERT_TRUE(decorator.decorate(record));
        ASSERT_EQ(1u, record.extProtocol.size());
        ASSERT_EQ(1u, record.extProtocol[0].ticketKeys.size());
        EXPECT_THAT(record.extProtocol[0].ticketKeys[0], ElementsAre("10001", "10003"));
    }
    firstWriter.get();
    secondWriter.get();
}

TEST_F(HttpRequestEncoderTests, TokenClearIsSafeDuringEncodingAndDecoration)
{
    TokenTelemetrySystem tokenSystem;
    auto& tokens = tokenSystem.manager.tokens;
    HttpRequestEncoder tokenEncoder(tokenSystem, mockHttpClient);
    BaseDecorator decorator(tokenSystem.manager);
    const std::string value(256, 'a');
    std::promise<void> start;
    auto ready = start.get_future().share();
    auto writer = std::async(std::launch::async, [&]()
                             {
        ready.wait();
        for (int refresh = 0; refresh < 2000; ++refresh)
        {
            EXPECT_EQ(STATUS_SUCCESS, tokens.Clear());
            EXPECT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_MSA_User, value.c_str()));
            EXPECT_EQ(STATUS_SUCCESS, tokens.SetTicketToken(TicketType_AAD, value.c_str()));
            tokens.SetStrictMode(refresh % 2 == 0);
        } });

    start.set_value();
    for (int event = 0; event < 1000; ++event)
    {
        auto ctx = std::make_shared<EventsUploadContext>();
        tokenEncoder.encode(ctx);
        const auto* request = static_cast<SimpleHttpRequest*>(ctx->httpRequest);
        auto deviceHeader = request->m_headers.find("Aad-Token");
        if (deviceHeader != request->m_headers.end())
        {
            EXPECT_EQ(value, deviceHeader->second);
        }
        auto userHeader = request->m_headers.find("Tickets");
        if (userHeader != request->m_headers.end())
        {
            EXPECT_EQ("\"10001\"=\"p:" + value + "\"", userHeader->second);
        }

        CsProtocol::Record record;
        ASSERT_TRUE(decorator.decorate(record));
        if (!record.extProtocol.empty())
        {
            ASSERT_EQ(1u, record.extProtocol[0].ticketKeys.size());
            EXPECT_THAT(record.extProtocol[0].ticketKeys[0], ElementsAre("10001"));
        }
    }
    writer.get();
}
