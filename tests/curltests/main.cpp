/*
 * main.cpp
 *
 *  Created on: Sep 14, 2017
 *      Author: maxgolov
 */

#include <cstdio>
#include <cstdlib>
#include <cstdint>

#include <string.h>
#include <map>

#include "gtest/gtest.h"

#include "HttpClient_Curl.hpp"

#ifndef _WIN32
#include <unistd.h>
#endif

#define TEST_SERVER_URL         "http://127.0.0.1:18000/moo.cgi"
#define TEST_SERVER_INVALID_URL "https://127.0.0.1/"
#define TEST_SERVER_401         "https://httpbin.org/status/401"
#define TEST_SERVER_404         "https://httpbin.org/status/404"
#define TEST_SERVER_TIMEOUT     "https://httpbin.org/delay/10"
#define TEST_SERVER_TIMEOUT5    "https://httpbin.org/delay/5"
#define TEST_BODY               "TEST\0\0\0HTTP\1POST\2BODY\nLine 1\nLine 2\n"

#define GET			const_cast<char*>("GET")
#define POST			const_cast<char*>("POST")

namespace {

    HttpClientCurl httpStack;

    static inline void sleep_ms(int t) {
    #ifndef _WIN32
      usleep(t * 1000);
    #else
      Sleep(t);
    #endif
    }

    /**
     *
     * @param req
     */
    void DebugPrint(HttpRequestCurl& req)
    {
        printf(">>>> HTTP status code: %d\n", req.GetResponseCode());
        printf(">>>> HTTP response headers:\n");
        auto respHeaders = req.GetResponseHeaders();
        for(auto &kv : respHeaders)
        {
            printf("%s: %s\n", kv.first.c_str(), kv.second.c_str());
        }
        auto body = req.GetResponseBody();
        for(auto &c : body) {
            if(c == 0)
                c = '.';
        }
        printf(">>>> HTTP response body:\n%s", (const char *)&body[0]);
        printf("\n<<<<\n");
    }

    /**
     *
     */
    TEST(Curl, TestPost)
    {
        std::map<std::string, std::string> reqHeaders;
        reqHeaders["Cache-Control"] = "no-cache";
        reqHeaders["Accept"] = "*/*";
        reqHeaders["Accept-Encoding"] = "gzip, deflate";
        reqHeaders["User-Agent"] = "AriaSDK Client";
        reqHeaders["Content-Type"] = "application/bond-compact-binary";
        reqHeaders["Client-Id"] = "NO_AUTH";
        reqHeaders["sdk-version"] = "ACT-Linux-C++-no-2.0.0.0-no";
        reqHeaders["x-apikey"] = "6d084bbf6a9644ef83f40a77c9e34580-c2d379e0-4408-4325-9b4d-2a7d78131e14-7322";
        // Content-Length is automatically calculated by libcurl
        std::vector<uint8_t> payload(TEST_BODY, TEST_BODY + sizeof(TEST_BODY));
        HttpRequestCurl req(POST, TEST_SERVER_URL, reqHeaders, payload);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(200, result);
    }

    TEST(Curl, TestGet)
    {
        HttpRequestCurl req(GET, TEST_SERVER_URL);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(200, result);
    }

    TEST(Curl, TestGetAsync)
    {
        HttpRequestCurl req(GET, TEST_SERVER_URL);
        auto &result = req.SendAsync([](HttpRequestCurl& req) -> void {
            printf("Async callback got status: %d\n", req.GetResponseCode());
            DebugPrint(req);
        });
        printf("HTTP request is executing async...\n");
        // Block and wait for async routine to complete
        EXPECT_EQ(200, result.get());

    }

    TEST(Curl, UnreachableHost)
    {
        HttpRequestCurl req(GET, TEST_SERVER_INVALID_URL);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(CURLE_COULDNT_CONNECT, result);
    }

    TEST(Curl, TestCode401)
    {
        HttpRequestCurl req(GET, TEST_SERVER_401);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(401, result);
    }

    TEST(Curl, TestCode404)
    {
        HttpRequestCurl req(GET, TEST_SERVER_404);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(404, result);
    }

    TEST(Curl, TestTimeout10)
    {
        // Verify that default 5s timeout works
        HttpRequestCurl req(GET, TEST_SERVER_TIMEOUT);
        long result = req.Send();
        DebugPrint(req);
        EXPECT_EQ(CURLE_OPERATION_TIMEDOUT, result);
    }

    TEST(Curl, TestTimeoutAbort)
    {
        for (size_t i=0; i<5; i++) {
            HttpRequestCurl req(GET, TEST_SERVER_TIMEOUT5);
            auto &result = req.SendAsync([](HttpRequestCurl& req) -> void {
                printf("Async callback got status: %d\n", req.GetResponseCode());
                DebugPrint(req);
            });
            printf("HTTP request is executing async...\n");
            // Sleep for some time
            sleep_ms(i * 1000);
            // ...and abruptly abort the request, so that we don't get 200
            req.Abort();
            // Block and wait for async routine to complete with an error code:
            // CURLE_SEND_ERROR         (55)    - aborted during HTTP send
            // CURLE_RECV_ERROR         (56)    - aborted during HTTP receive
            // CURLE_OPERATION_TIMEDOUT (28)    - aborted during SSL handshake
            EXPECT_NE(200, result.get());
        }
    }


}
