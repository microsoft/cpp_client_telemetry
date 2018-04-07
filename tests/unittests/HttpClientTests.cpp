// Copyright (c) Microsoft. All rights reserved.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include "common/Common.hpp"
#if ARIASDK_PAL_SKYPE
    #include "http/HttpClient_HttpStack.hpp"
#elif ARIASDK_PAL_WIN32
    #include "http/HttpClient_WinInet.hpp"
#endif
#include "common/HttpServer.hpp"

using namespace testing;
using namespace ARIASDK_NS;

#define getAriaSdkLogComponent  ::testing::getAriaSdkLogComponent

class HttpClientTests : public PAL::RefCountedImpl<HttpClientTests>,
                        public ::testing::Test,
                        public HttpServer::Callback,
                        public IHttpResponseCallback
{
  protected:
    HttpServer                           _server;
    int                                  _port;
    std::string                          _hostname;
    std::unique_ptr<IHttpClient>         _client;
    volatile bool                        _responseReceived;
    std::unique_ptr<IHttpResponse const> _response;
    volatile int                         _receivedRequestsCount;
    volatile int                         _receivedResponsesCount;
    enum RequestState { Planned, Sent, Processed, Done };
    std::vector<RequestState>            _countedRequestsMap;
    std::mutex                           _lock;

  public:
    HttpClientTests()
    {
#if ARIASDK_PAL_SKYPE
        _client.reset(new HttpClient_HttpStack(nullptr));
#elif ARIASDK_PAL_WIN32
        _client.reset(new HttpClient_WinInet());
#endif
    }

    virtual void SetUp() override
    {
        _port = _server.addListeningPort(0);
        std::ostringstream os;
        os << "localhost:" << _port;
        _hostname = os.str();
        _server.setServerName(_hostname);
        _server.addHandler("/simple/", *this);
        _server.addHandler("/echo/",   *this);
        _server.addHandler("/count/",  *this);
        _server.start();

        _responseReceived       = false;
        _receivedRequestsCount  = 0;
        _receivedResponsesCount = 0;
    }

    virtual void TearDown() override
    {
        _server.stop();
        _client.reset();
    }

  protected:
    virtual int onHttpRequest(HttpServer::Request const& request, HttpServer::Response& inResponse) override
    {
        if (request.uri.substr(0, 8) == "/simple/") {
            inResponse.headers["Content-Type"] = "text/plain";
            inResponse.content = "It works!";
            return atoi(request.uri.substr(8).c_str());
        }

        if (request.uri == "/echo/") {
            auto it = request.headers.find("Content-Type");
            inResponse.headers["Content-Type"] = (it != request.headers.end()) ? it->second : "application/octet-stream";
            inResponse.content = request.content;
            return 200;
        }

        if (request.uri.substr(0, 7) == "/count/") {
            int id = atoi(request.uri.substr(7).c_str());
            if (id >= 0 && static_cast<size_t>(id) < _countedRequestsMap.size()) {
                _countedRequestsMap[id] = Processed;
            }
            _receivedRequestsCount++;
            inResponse.headers["Content-Type"] = "text/plain";
            inResponse.content = request.uri.substr(7);
            return 200;
        }

        return 0;
    }

    virtual void OnHttpResponse(IHttpResponse* inResponse) override
    {
		std::lock_guard<std::mutex> lock(_lock);

        if (!_countedRequestsMap.empty() && inResponse->GetResult() == HttpResult_OK && inResponse->GetStatusCode() == 200) {
            int id = atoi(std::string(reinterpret_cast<char const*>(inResponse->GetBody().data()), inResponse->GetBody().size()).c_str());
            if (id >= 0 && static_cast<size_t>(id) < _countedRequestsMap.size()) {
                _countedRequestsMap[id] = Done;
            }
            _receivedResponsesCount++;
        }

        this->_response.reset(inResponse);
        _responseReceived = true;
    }
};

std::vector<uint8_t> Binary(std::string const& str)
{
    return std::vector<uint8_t>(str.data(), str.data() + str.size());
}

//---

TEST_F(HttpClientTests, HandlesSimpleRequest)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
}

TEST_F(HttpClientTests, HandlesErrorRequest)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/404");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 404u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
}

TEST_F(HttpClientTests, HandlesPostRequest)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
}

TEST_F(HttpClientTests, HandlesLocalErrors)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("://trololo!");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_LocalFailure);
}

TEST_F(HttpClientTests, HandlesDnsError)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://domain.name.doesnt.exist");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
}

TEST_F(HttpClientTests, HandlesConnectionError)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://localhost:4");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
}

TEST_F(HttpClientTests, HandlesCancellation)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/echo/");
    _client->SendRequestAsync(request.release(), this);
    _client->CancelRequestAsync(requestId);

    for (int i = 0; i < 20 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_Aborted);
}

TEST_F(HttpClientTests, Handles100Continue)
{
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Expect", "100-continue");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !_responseReceived; i++) {
        PAL::sleep(100);
    }

    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
}

TEST_F(HttpClientTests, SurvivesManyRequests)
{
    int Count = 2000;
    _countedRequestsMap.resize(Count, Planned);

    for (int i = 0; i < Count; i++) {
        std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
        request->SetMethod("POST");
        request->GetHeaders().set("expect", "100-continue");
        request->GetHeaders().set("content-type", "application/octet-stream");
        std::ostringstream url;
        url << "http://" << _hostname << "/count/" << i;
        request->SetUrl(url.str());
        auto body = Binary("content");
        request->SetBody(body);

        _countedRequestsMap[i] = Sent;
        _client->SendRequestAsync(request.release(), this);

        int lag = i - _receivedRequestsCount;
        if (lag > 100) {
            LOG_ERROR("Failed to receive requests in time");
            break;
        }
        PAL::sleep(std::max(lag, 0) * 10);
    }

    for (int i = 0; i < 50 && _receivedResponsesCount < Count; i++) {
        PAL::sleep(100);
    }

    EXPECT_THAT(_receivedRequestsCount,  Count);
    EXPECT_THAT(_receivedResponsesCount, Count);
    EXPECT_THAT(_countedRequestsMap,     Not(Contains(Ne(Done))));
    for (int i = 0; i < Count; i++) {
        if (_countedRequestsMap[i] != Done) {
            LOG_ERROR("Request #%u: %u", static_cast<unsigned>(i), _countedRequestsMap[i]);
        }
    }
}
