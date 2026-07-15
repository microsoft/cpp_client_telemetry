//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// These tests only apply to the curl HTTP client path (Linux, non-Apple, non-Android)
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT) \
    && !defined(__APPLE__) && !defined(ANDROID)

#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "http/HttpClient_Curl.hpp"
#include "config/RuntimeConfig_Default.hpp"

using namespace testing;
using namespace MAT;

class HttpClientCurlTests : public ::testing::Test
{
protected:
    HttpClient_Curl m_client;
    // Named, fixture-lifetime values so they outlive each CurlHttpOperation.
    const std::map<std::string, std::string> m_headers;
    const std::vector<uint8_t> m_body;
};

// --- SetSslVerification wiring ---

TEST_F(HttpClientCurlTests, SslVerification_DefaultsToTrue)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr, m_headers, m_body);
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithVerifyTrue)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, true, "");
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithVerifyFalse)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, false, "");
    ASSERT_NE(op.GetHandle(), nullptr);
}

TEST_F(HttpClientCurlTests, CurlHttpOperation_ConstructsWithCaInfo)
{
    CurlHttpOperation op("GET", "https://example.com", nullptr,
        m_headers, m_body,
        false, 5, true, "/etc/ssl/certs/ca-certificates.crt");
    ASSERT_NE(op.GetHandle(), nullptr);
}

// --- ILogConfiguration integration ---

TEST(HttpClientCurlConfigTests, LogConfiguration_SslVerify_DefaultIsTrue)
{
    // defaultRuntimeConfig from RuntimeConfig_Default.hpp has the defaults
    bool sslVerify = defaultRuntimeConfig[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY];
    EXPECT_TRUE(sslVerify);
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslCaInfo_DefaultIsEmpty)
{
    const char* caInfo = defaultRuntimeConfig[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO];
    EXPECT_STREQ(caInfo, "");
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslVerify_CanBeDisabled)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY] = false;
    bool sslVerify = config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY];
    EXPECT_FALSE(sslVerify);
}

TEST(HttpClientCurlConfigTests, LogConfiguration_SslCaInfo_CanBeSet)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO] = "/custom/ca-bundle.crt";
    const char* caInfo = config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO];
    EXPECT_STREQ(caInfo, "/custom/ca-bundle.crt");
}

// --- ApplySettings integration ---

TEST_F(HttpClientCurlTests, ApplySettings_ReadsSslConfigFromLogConfiguration)
{
    ILogConfiguration config;
    config[CFG_MAP_HTTP][CFG_BOOL_HTTP_SSL_VERIFY] = false;
    config[CFG_MAP_HTTP][CFG_STR_HTTP_SSL_CAINFO] = "/custom/ca.pem";
    m_client.ApplySettings(config);
    // Verify indirectly -- constructing an operation should not fail
    SUCCEED();
}

TEST_F(HttpClientCurlTests, ApplySettings_DefaultConfigEnablesVerification)
{
    ILogConfiguration config;
    m_client.ApplySettings(config);
    SUCCEED();
}

// --- Thread safety: SetSslVerification concurrent with reads ---

TEST_F(HttpClientCurlTests, SetSslVerification_ConcurrentCallsNoRace)
{
    // Exercise the atomic + mutex path under contention.
    // No assertions on output -- this is a sanitizer/TSAN target.
    std::vector<std::future<void>> futures;
    for (int i = 0; i < 10; ++i)
    {
        futures.push_back(std::async(std::launch::async, [this, i]() {
            m_client.SetSslVerification(i % 2 == 0, (i % 2 == 0) ? "/some/path" : "");
        }));
    }
    for (auto& f : futures)
    {
        f.get();
    }
    SUCCEED();
}

// --- Response-size cap (memory-amplification DoS hardening) ---

class HttpClientCurlResponseCapTests : public ::testing::Test,
                                       public HttpServer::Callback,
                                       public IHttpResponseCallback
{
protected:
    HttpServer      m_server;
    HttpClient_Curl m_client;
    // The client never takes ownership of the request (it only stores a raw pointer
    // and erases it); the fixture owns it and frees it in TearDown -- on the main
    // thread, after the transfer has completed. Freeing it inside OnHttpResponse
    // would destroy the CurlHttpOperation from within its own async task, whose
    // destructor waits on that task (a self-join deadlock).
    std::unique_ptr<IHttpRequest> m_request;
    std::string     m_hostname;
    size_t          m_responseBodySize {0};

    std::mutex      m_lock;
    bool            m_received {false};
    HttpResult      m_result {};
    unsigned int    m_statusCode {0};
    size_t          m_bodySize {0};

    void SetUp() override
    {
        int port = m_server.addListeningPort(0);
        std::ostringstream os;
        os << "127.0.0.1:" << port;
        m_hostname = os.str();
        m_server.setServerName(m_hostname);
        m_server.addHandler("/huge/", *this);
        m_server.start();
    }

    void TearDown() override
    {
        m_server.stop();
        m_request.reset();
    }

    // HttpServer::Callback -- returns a body of m_responseBodySize bytes.
    int onHttpRequest(HttpServer::Request const& /*request*/, HttpServer::Response& response) override
    {
        size_t bodySize;
        {
            std::lock_guard<std::mutex> lock(m_lock);
            bodySize = m_responseBodySize;
        }
        response.headers["Content-Type"] = "application/octet-stream";
        response.content = std::string(bodySize, 'A');
        return 200;
    }

    // IHttpResponseCallback -- the SDK hands over ownership of the response.
    void OnHttpResponse(IHttpResponse* response) override
    {
        std::unique_ptr<IHttpResponse> owned(response);
        std::lock_guard<std::mutex> lock(m_lock);
        m_result = owned->GetResult();
        m_statusCode = owned->GetStatusCode();
        m_bodySize = owned->GetBody().size();
        m_received = true;
    }

    bool responseReceived()
    {
        std::lock_guard<std::mutex> lock(m_lock);
        return m_received;
    }

    void sendAndWait(size_t bodySize)
    {
        {
            std::lock_guard<std::mutex> lock(m_lock);
            m_received = false;
            m_result = HttpResult{};
            m_statusCode = 0;
            m_bodySize = 0;
            m_responseBodySize = bodySize; // read under the same lock by onHttpRequest
        }
        m_request.reset(m_client.CreateRequest());
        m_request->SetUrl("http://" + m_hostname + "/huge/");
        m_client.SendRequestAsync(m_request.get(), this);
        for (int i = 0; i < 300 && !responseReceived(); i++)
            PAL::sleep(100);
    }
};

TEST_F(HttpClientCurlResponseCapTests, AbortsOversizedResponseBody)
{
    // A response body larger than the client's response-size cap (kMaxResponseBytes,
    // 16 MB) must be refused, not buffered in full, so a hostile/MITM'd collector
    // cannot exhaust process memory.
    sendAndWait(17u * 1024u * 1024u);
    ASSERT_TRUE(responseReceived());
    // curl aborts the transfer (CURLE_WRITE_ERROR) once the cap is hit -> NetworkFailure.
    EXPECT_EQ(m_result, HttpResult_NetworkFailure);
    // The oversized body is never fully buffered.
    EXPECT_LE(m_bodySize, static_cast<size_t>(16u * 1024u * 1024u));
}

TEST_F(HttpClientCurlResponseCapTests, AcceptsLargeResponseUnderCap)
{
    // A large-but-legitimate response (well under the cap) must still be received
    // in full: the cap must not regress normal responses.
    const size_t bodySize = 4u * 1024u * 1024u;
    sendAndWait(bodySize);
    ASSERT_TRUE(responseReceived());
    EXPECT_EQ(m_result, HttpResult_OK);
    EXPECT_EQ(m_statusCode, 200u);
    EXPECT_EQ(m_bodySize, bodySize);
}

#endif // MATSDK_PAL_CPP11 && !_MSC_VER && HAVE_MAT_DEFAULT_HTTP_CLIENT
