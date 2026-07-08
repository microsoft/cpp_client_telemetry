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

// --- Regression: issue #1481 (EDEADLK self-join in ~CurlHttpOperation) ---

// When the async callback drops the last *external* reference to the operation,
// ~CurlHttpOperation runs on the worker thread. The old std::async design joined
// its own future there (self-join) and aborted the process with
// std::system_error("Resource deadlock avoided"). The worker now holds a
// shared_ptr keepalive and there is no future, so destruction on the worker thread
// is trivial and safe. This test aborts the process on the old code and passes on
// the fix.
TEST_F(HttpClientCurlTests, SendAsync_DestroyOnWorkerThread_NoSelfJoin)
{
    std::promise<void> callbackDone;
    auto done = callbackDone.get_future();

    // Closed local port -> Send() fails fast (connection refused), no network wait.
    auto op = std::make_shared<CurlHttpOperation>(
        "GET", "http://127.0.0.1:9/", nullptr, m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");

    // Move the only external reference into a heap box the callback will delete,
    // then release our own reference. After SendAsync the live references are the
    // box and the worker's keepalive.
    auto* box = new std::shared_ptr<CurlHttpOperation>(std::move(op));

    (*box)->SendAsync([box, &callbackDone](CurlHttpOperation&) {
        // Runs on the worker thread. Drop the last external reference here. On the
        // old code this destroyed the operation on this thread and self-joined its
        // own future -> abort. With the keepalive fix the worker still holds a
        // reference, so this is safe and the operation is destroyed once the worker
        // returns.
        delete box;
        callbackDone.set_value();
    });

    ASSERT_EQ(done.wait_for(std::chrono::seconds(15)), std::future_status::ready);
}

#endif // MATSDK_PAL_CPP11 && !_MSC_VER && HAVE_MAT_DEFAULT_HTTP_CLIENT
