//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

#include "common/Common.hpp"
#include "common/HttpServer.hpp"

#ifdef HAVE_CONSOLE_LOG
#undef LOG_DEBUG
#include "common/DebugConsole.hpp"
#endif

using namespace testing;

class HttpServerTestsBase : public ::testing::Test
{
  protected:
    HttpServer server;
    int port { 0 };
    Socket clientSocket;

  public:
    HttpServerTestsBase()
    {
    }

    ~HttpServerTestsBase()
    {
        if (!clientSocket.invalid()) {
            clientSocket.close();
        }
    }

    virtual void SetUp() override
    {
        port = server.addListeningPort(0);
        server.setServerName("http.server.tests");
        addHandlers();
        server.start();
    }

    virtual void TearDown() override
    {
        server.stop();
    }

  protected:
    virtual void addHandlers() = 0;

    bool connect()
    {
        if (clientSocket.invalid()) {
            clientSocket = Socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        }
        SocketAddr addr(SocketAddr::Loopback, port);
        return clientSocket.connect(addr);
    }

    bool sendRequest(std::string const& request)
    {
        return (clientSocket.send(request.data(), static_cast<unsigned int>(request.size())) == static_cast<int>(request.size()));
    }

    std::string receiveResponse(bool singlePacket = false)
    {
        std::string response;

        for (;;) {
            char buffer[2048];
            int received = clientSocket.recv(buffer, sizeof(buffer));
            if (received == 0) {
                break;
            }
            if (received < 0) {
                response += "<ERROR>";
                break;
            }
            response.append(buffer, received);
            if (singlePacket) {
                break;
            }
        }

        return response;
    }

    std::string receiveResponseScrubDate(int count = 1)
    {
        std::string response = receiveResponse();

        size_t pos = 0;
        while (count--) {
            pos = response.find("\r\nDate: ", pos);
            if (pos != std::string::npos) {
                size_t pos2 = response.find("\r\n", pos + 8);
                if (pos2 != std::string::npos) {
                    response.replace(pos + 8, pos2 - pos - 8, "---");
                    pos += 8;
                }
            }
        }

        return response;
    }
};

//---

class HttpServerTestsSimple : public HttpServerTestsBase,
                              protected HttpServer::Callback
{
  public:
    virtual void addHandlers() override
    {
        server.addHandler("/simple/", *this);
        server.addHandler("/echo/", *this);
    }

    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& response) override
    {
        if (request.uri.substr(0, 8) == "/simple/") {
            response.headers["Content-Type"] = "text/plain";
            response.content = "It works!";
            return atoi(request.uri.substr(8).c_str());
        }

        if (request.uri == "/echo/") {
            auto it = request.headers.find("Content-Type");
            response.headers["Content-Type"] = (it != request.headers.end()) ? it->second : "application/octet-stream";
            response.content = request.content;
            return 200;
        }

        return 0;
    }
};

//---

TEST_F(HttpServerTestsSimple, HandlesSimpleRequest)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "GET /simple/200 HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "\r\n"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n"
        "Content-Type: text/plain\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "It works!"));
}

TEST_F(HttpServerTestsSimple, HandlesRequestWithContent)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "GET /echo/ HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 10\r\n"
        "\r\n"
        "0123456789"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "0123456789"));
}

TEST_F(HttpServerTestsSimple, CombinesReceivedPackets)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(clientSocket.setNoDelay(), true);
    ASSERT_THAT(sendRequest("GET "), true);
    ASSERT_THAT(sendRequest("/echo/ HTTP/1."), true);
    ASSERT_THAT(sendRequest("1\r\n"), true);
    ASSERT_THAT(sendRequest(
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 10\r\n"
        "\r"), true);
    ASSERT_THAT(sendRequest("\n012"), true);
    ASSERT_THAT(sendRequest("3456789"), true);
    ASSERT_THAT(sendRequest("34567890"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "0123456789"));
}

TEST_F(HttpServerTestsSimple, AddsResponseMessagePerResponseCode)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "GET /simple/404 HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "\r\n"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 404 Not Found\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n"
        "Content-Type: text/plain\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "It works!"));
}

TEST_F(HttpServerTestsSimple, PipeliningWorks)
{
    server.setKeepalive(true);
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "GET /simple/200 HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "GET /simple/201 HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "\r\n"), true);
    EXPECT_THAT(receiveResponseScrubDate(2), Eq(
        "HTTP/1.1 200 OK\r\n"
        "Connection: keep-alive\r\n"
        "Content-Length: 9\r\n"
        "Content-Type: text/plain\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "It works!"
        "HTTP/1.1 201 Created\r\n"
        "Connection: close\r\n"
        "Content-Length: 9\r\n"
        "Content-Type: text/plain\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "It works!"));
}

#ifdef _WIN32 /* TODO: [MG] - debug why this test is not working properly on Mac OS X */
TEST_F(HttpServerTestsSimple, LongContentIsTransferredProperly)
{
    std::string content;
    content.reserve(512 * 1024);
    content += "Lorem ipsum dolor sit amet consectetur adipiscing elit! \x01\x21\x41\x61\x81\xA1\xC1\xE1";
    for (int i = 0; i < 13; i++) {
        content += content;
    }
    ASSERT_THAT(content.length(), 524288u);

    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "GET /echo/ HTTP/1.1\r\n"
        "Host: http.server.tests\r\n"
        "X-Payload: "), true);
    ASSERT_THAT(sendRequest(content.substr(0, 7500)), true);
    ASSERT_THAT(sendRequest(
        "\r\n"
        "Connection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 524288\r\n"
        "\r\n"), true);
    ASSERT_THAT(sendRequest(content), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(std::string(
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Length: 524288\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n") + content));
}
#endif

TEST_F(HttpServerTestsSimple, SupportsExpect100Continue)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "POST /echo/ HTTP/1.1\r\n"
        "Expect: 100-continue\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 10\r\n"
        "\r\n"), true);
    EXPECT_THAT(receiveResponse(true), Eq(
        "HTTP/1.1 100 Continue\r\n"
        "\r\n"));
    ASSERT_THAT(sendRequest(
        "0123456789"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 200 OK\r\n"
        "Connection: close\r\n"
        "Content-Length: 10\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"
        "0123456789"));
}

TEST_F(HttpServerTestsSimple, FailsOnUnknownExpect)
{
    ASSERT_THAT(connect(), true);
    ASSERT_THAT(sendRequest(
        "POST /echo/ HTTP/1.1\r\n"
        "Expect: 666-bad\r\n"
        "Host: http.server.tests\r\n"
        "Connection: close\r\n"
        "Content-Type: application/octet-stream\r\n"
        "Content-Length: 10\r\n"
        "\r\n"), true);
    EXPECT_THAT(receiveResponseScrubDate(), Eq(
        "HTTP/1.1 417 Expectation Failed\r\n"
        "Connection: close\r\n"
        "Content-Length: 0\r\n"
        "Date: ---\r\n"
        "Host: http.server.tests\r\n"
        "\r\n"));
}

