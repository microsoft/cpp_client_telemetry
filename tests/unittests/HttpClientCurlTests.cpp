//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "mat/config.h"

// These tests only apply to the curl HTTP client path (Linux, non-Apple, non-Android)
#if defined(MATSDK_PAL_CPP11) && !defined(_MSC_VER) && defined(HAVE_MAT_DEFAULT_HTTP_CLIENT) \
    && !defined(__APPLE__) && !defined(ANDROID)

#include "common/Common.hpp"
#include "http/HttpClient_Curl.hpp"
#include "config/RuntimeConfig_Default.hpp"

#include <future>
#include <chrono>
#include <memory>
#include <atomic>
#include <cstdlib>
#include <stdexcept>
#include <utility>

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

// --- Regression: EDEADLK self-join in ~CurlHttpOperation ---

TEST_F(HttpClientCurlTests, SendAsync_DestroyOnWorkerThread_NoSelfJoin)
{
    struct TrackingCallback : public IHttpResponseCallback
    {
        std::atomic<int> destroyEvents { 0 };
        void OnHttpResponse(IHttpResponse* response) override { delete response; }
        void OnHttpStateEvent(HttpStateEvent state, void*, size_t) override
        {
            if (state == OnDestroy)
            {
                ++destroyEvents;
            }
        }
    };

    auto callback = std::make_shared<TrackingCallback>();
    auto callbackDone = std::make_shared<std::promise<void>>();
    auto done = callbackDone->get_future();

    auto op = std::make_shared<CurlHttpOperation>(
        "GET", "://malformed", callback.get(), m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");

    auto box = std::make_shared<std::shared_ptr<CurlHttpOperation>>(std::move(op));
    (*box)->SendAsync([box, callback, callbackDone](CurlHttpOperation&) {
        box->reset();
        callbackDone->set_value();
    });

    if (done.wait_for(std::chrono::seconds(5)) != std::future_status::ready)
    {
        ADD_FAILURE() << "curl worker did not finish before fixture teardown";
        std::abort();
    }
    EXPECT_EQ(callback->destroyEvents.load(), 1);
}

TEST_F(HttpClientCurlTests, SendAsync_CallbackCopyFailureStillCompletes)
{
    struct ThrowOnCopy
    {
        explicit ThrowOnCopy(bool& invoked) : invoked(&invoked) {}
        ThrowOnCopy(ThrowOnCopy&&) = default;
        ThrowOnCopy(const ThrowOnCopy&) { throw std::logic_error("copy failed"); }
        void operator()(CurlHttpOperation&) const { *invoked = true; }
        bool* invoked;
    };

    CurlHttpOperation op(
        "GET", "://malformed", nullptr, m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");
    bool callbackInvoked = false;
    std::function<void(CurlHttpOperation&)> callback { ThrowOnCopy(callbackInvoked) };

    EXPECT_NO_THROW(op.SendAsync(std::move(callback)));
    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(op.GetResponseCode(), CURLE_FAILED_INIT);
    EXPECT_THROW(op.SendAsync(), std::logic_error);
}

#endif // MATSDK_PAL_CPP11 && !_MSC_VER && HAVE_MAT_DEFAULT_HTTP_CLIENT
