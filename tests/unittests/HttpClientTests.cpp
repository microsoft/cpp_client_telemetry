//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include "common/Common.hpp"
#include "common/HttpServer.hpp"
#include "http/HttpClientFactory.hpp"

using namespace testing;
using namespace MAT;

#define getMATSDKLogComponent  ::testing::getMATSDKLogComponent

class HttpClientTests : public ::testing::Test,
                        public HttpServer::Callback,
                        public IHttpResponseCallback
{
  protected:
    HttpServer                           _server;
    int                                  _port;
    std::string                          _hostname;
    std::shared_ptr<IHttpClient>         _client;
    std::vector<IHttpResponse*>          _responses;

    enum RequestState { Planned, Sent, Processed, Done };
    std::vector<RequestState>            _countedRequests;
    std::mutex                           _lock;

  public:
    HttpClientTests()
    {
        _client = HttpClientFactory::Create();
    }

    void Clear()
    {
        for (auto &v : _responses)
            delete v;
        _responses.clear();
        _countedRequests.clear();
    }

    bool responseReceived()
    {
        return (_responses.size() > 0);
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

        Clear();
    }

    virtual void TearDown() override
    {
        _server.stop();
        _client.reset();
        Clear();
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
            if (id >= 0 && static_cast<size_t>(id) < _countedRequests.size()) {
                _countedRequests[id] = Processed;
            }
            inResponse.headers["Content-Type"] = "text/plain";
            inResponse.content = request.uri.substr(7);
            return 200;
        }

        return 0;
    }

    /**
     * This method temporarily copies SimpleHttpResponse to a responses buffer.
     */
    virtual SimpleHttpResponse* clone(IHttpResponse* inResponse)
    {
        SimpleHttpResponse *src = static_cast<SimpleHttpResponse*>(inResponse);
        SimpleHttpResponse *dst = new SimpleHttpResponse("");
        dst->m_id = src->m_id;
        dst->m_result = src->m_result;
        dst->m_statusCode = src->m_statusCode;
        dst->m_headers = src->m_headers;
        dst->m_body = src->m_body;
        return dst;
    }

    virtual void OnHttpResponse(IHttpResponse* inResponse) override
    {
        std::lock_guard<std::mutex> lock(_lock);
        _responses.push_back(clone(inResponse));
    }

};

std::vector<uint8_t> Binary(std::string const& str)
{
    return std::vector<uint8_t>(str.data(), str.data() + str.size());
}

//---

TEST_F(HttpClientTests, HandlesSimpleRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/200");
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
    _response.release();
}

TEST_F(HttpClientTests, HandlesErrorRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/simple/404");
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 404u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetBody(), Eq(Binary("It works!")));
    _response.release();

}

TEST_F(HttpClientTests, HandlesPostRequest)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    while (!responseReceived())
        PAL::sleep(100);

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
    _response.release();
}

TEST_F(HttpClientTests, HandlesLocalErrors)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("://trololo!");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_LocalFailure);
    _response.release();
}

TEST_F(HttpClientTests, HandlesDnsError)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://domain.name.doesnt.exist");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
    _response.release();
}

TEST_F(HttpClientTests, HandlesConnectionError)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://localhost:4");
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 200 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_NetworkFailure);
    _response.release();
}

TEST_F(HttpClientTests, HandlesCancellation)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetUrl("http://" + _hostname + "/echo/");
    _client->SendRequestAsync(request.release(), this);
    _client->CancelRequestAsync(requestId);

    for (int i = 0; i < 20 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_Aborted);
    _response.release();
}

TEST_F(HttpClientTests, Handles100Continue)
{
    Clear();
    std::unique_ptr<IHttpRequest> request(_client->CreateRequest());
    std::string requestId = request->GetId();
    request->SetMethod("POST");
    request->GetHeaders().set("Expect", "100-continue");
    request->GetHeaders().set("Content-Type", "application/octet-stream");
    request->SetUrl("http://" + _hostname + "/echo/");
    auto body = Binary("Some\xBB\x11naryContent");
    request->SetBody(body);
    _client->SendRequestAsync(request.release(), this);

    for (int i = 0; i < 20 && !responseReceived(); i++) {
        PAL::sleep(100);
    }

    std::unique_ptr<IHttpResponse> _response(_responses[0]);
    ASSERT_THAT(_response.get(), NotNull());
    EXPECT_THAT(_response->GetId(), requestId);
    EXPECT_THAT(_response->GetResult(), HttpResult_OK);
    EXPECT_THAT(_response->GetStatusCode(), 200u);
    EXPECT_THAT(_response->GetHeaders().get("Host"), _hostname);
    EXPECT_THAT(_response->GetHeaders().get("Content-Type"), Eq("application/octet-stream"));
    EXPECT_THAT(_response->GetBody(), Eq(Binary("Some\xBB\x11naryContent")));
    _response.release();
}

TEST_F(HttpClientTests, SurvivesManyRequests)
{
    Clear();

    size_t Count = 100;
    for (size_t i = 0; i < Count; i++) {
        IHttpRequest* request = _client->CreateRequest();
        // _requests.push_back(request);
        request->SetMethod("POST");
        request->GetHeaders().set("expect", "100-continue");
        request->GetHeaders().set("content-type", "application/octet-stream");
        std::ostringstream url;
        url << "http://" << _hostname << "/count/" << i;
        request->SetUrl(url.str());
        auto body = Binary("content");
        request->SetBody(body);
        _countedRequests.push_back(Sent);
        _client->SendRequestAsync(request, this);
        // slowdown if responses are not coming back because SDK itself
        // throttles its HTTP requests at a rate of no more than 2
        // per second
        if (_responses.size() + 2 < i )
            PAL::sleep(100);
    }

    while (_responses.size() < Count) {
        PAL::sleep(100);
    }

    // Verify the count of completed requests by the number of completed responses
    for (auto &v : _responses)
    {
        int id = atoi(std::string(reinterpret_cast<char const*>(v->GetBody().data()), v->GetBody().size()).c_str());
        _countedRequests[id] = Done;
    }

    // Count the number of requests that were not done
    auto it = std::find(_countedRequests.begin(), _countedRequests.end(), Sent);
    EXPECT_THAT(it, _countedRequests.end());

}
#endif // HAVE_MAT_DEFAULT_HTTP_CLIENT

