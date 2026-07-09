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
    // Heap-owned promise so a captured copy keeps it alive: if the ASSERT below
    // fails and the test returns early, the still-detached worker can safely call
    // set_value() on it instead of touching a destroyed stack promise.
    auto callbackDone = std::make_shared<std::promise<void>>();
    auto done = callbackDone->get_future();

    // Host under the RFC 6761 reserved .invalid TLD never resolves, so Send() fails
    // fast and deterministically (name resolution error) on any environment --
    // unlike a fixed port, which could happen to be open.
    auto op = std::make_shared<CurlHttpOperation>(
        "GET", "http://selfjoin.regression.invalid/", nullptr, m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");

    // Non-owning handle, used only to cancel the worker on the timeout path below.
    // It must not keep the operation alive, or the callback's box->reset() would no
    // longer drop the last external reference (the exact scenario under test).
    std::weak_ptr<CurlHttpOperation> weakOp = op;

    // A shared box holds the only external reference. The callback resets the
    // contained shared_ptr (on the worker thread) to drop the last external
    // reference -- the exact #1481 trigger -- without raw new/delete.
    auto box = std::make_shared<std::shared_ptr<CurlHttpOperation>>(std::move(op));

    (*box)->SendAsync([box, callbackDone](CurlHttpOperation&) {
        // Runs on the worker thread. Drop the last external reference here. On the
        // old code this destroyed the operation on this thread and self-joined its
        // own future -> abort. With the keepalive fix the worker still holds a
        // reference, so this is safe and the operation is destroyed once the worker
        // returns.
        box->reset();
        callbackDone->set_value();
    });

    if (done.wait_for(std::chrono::seconds(15)) != std::future_status::ready)
    {
        // The detached worker is unexpectedly still running (Send() against the
        // non-resolving host should fail within milliseconds). Best-effort: signal
        // it to abort and give it a moment to finish so it does not outlive fixture
        // teardown, which destroys m_client (curl_global_cleanup) and the
        // m_headers/m_body it may still be reading. Then fail.
        if (auto liveOp = weakOp.lock())
            liveOp->Abort();
        done.wait_for(std::chrono::seconds(5));
        FAIL() << "SendAsync did not complete within 15s";
    }
}

// A stack-constructed operation is not owned by a shared_ptr, so shared_from_this()
// throws std::bad_weak_ptr. SendAsync() must not let that escape: it falls back to a
// synchronous run and still invokes the callback (issue #1481 review round 6).
TEST_F(HttpClientCurlTests, SendAsync_NotSharedOwned_RunsSynchronouslyNoThrow)
{
    CurlHttpOperation op(
        "GET", "http://selfjoin.regression.invalid/", nullptr, m_headers, m_body,
        false, 1 /*connTimeout*/, false /*sslVerify*/, "");

    bool callbackRan = false;
    // No shared owner -> the fallback runs Send()+callback synchronously on this
    // thread, so SendAsync() returns only after the callback has run. Capturing
    // callbackRan by reference is therefore safe.
    op.SendAsync([&callbackRan](CurlHttpOperation&) { callbackRan = true; });

    EXPECT_TRUE(callbackRan);
}

#endif // MATSDK_PAL_CPP11 && !_MSC_VER && HAVE_MAT_DEFAULT_HTTP_CLIENT
