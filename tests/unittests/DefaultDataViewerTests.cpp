// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "../modules/dataviewer/DefaultDataViewer.hpp"
#include "public/IHttpClient.hpp"

using namespace testing;
using namespace MAT;

class MockHttpClient : public MAT::IHttpClient
{
public:
    MockHttpClient() {}

    IHttpRequest* CreateRequest() override
    {
        return new MAT::SimpleHttpRequest("MockSimpleHttpRequest");
    }

    std::function<void(MAT::IHttpRequest*, MAT::IHttpResponseCallback*)> funcSendRequestAsync;
    void SendRequestAsync(MAT::IHttpRequest* request, MAT::IHttpResponseCallback* callback) override
    {
        if (funcSendRequestAsync)
        {
            funcSendRequestAsync(request, callback);
        }
        else
        {
            m_request = std::shared_ptr<MAT::IHttpRequest>(request);
            m_responseCallback = std::shared_ptr<MAT::IHttpResponseCallback>(callback);
        }
    }

    void CancelRequestAsync(std::string const&) override
    {
    }

    void CancelAllRequests() override {}

    std::shared_ptr<MAT::IHttpRequest>& GetReceivedRequest() noexcept
    {
        return m_request;
    }

    std::shared_ptr<MAT::IHttpResponseCallback>& GetResponseCallback() noexcept
    {
        return m_responseCallback;
    }

private:
    std::shared_ptr<MAT::IHttpRequest> m_request;
    std::shared_ptr<MAT::IHttpResponseCallback> m_responseCallback;
};

TEST(DefaultDataViewerTests, Constructor_HttpClientNotPassed_HttpClientSetsOrThrowsBasedOnConfig)
{
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
    DefaultDataViewer viewer(nullptr, "Test");
    ASSERT_TRUE(viewer.GetHttpClient() != nullptr);
#else
    ASSERT_THROW(DefaultDataViewer(nullptr, "Test"), std::invalid_argument);
#endif
}

TEST(DefaultDataViewerTests, Constructor_MachineIdentifierValid_MachineIdentifierSetCorrectly)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    DefaultDataViewer viewer(mockHttpClient, "Test");
    ASSERT_EQ(viewer.GetMachineFriendlyIdentifier(), "Test");
}

TEST(DefaultDataViewerTests, Constructor_Transmission_Throws)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    ASSERT_THROW(DefaultDataViewer(mockHttpClient, nullptr), std::invalid_argument);
    ASSERT_THROW(DefaultDataViewer(mockHttpClient, ""), std::invalid_argument);
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_ValidEndpoint_TransmissionEnabled)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());

    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        response->m_result = HttpResult::HttpResult_OK;
        callback->OnHttpResponse(response.get());
    };
    
    DefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");
    
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_InvalidEndpoint_TransmissionEnabled)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());

    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 404;
        response->m_result = HttpResult::HttpResult_NetworkFailure;
        callback->OnHttpResponse(response.get());
    };
    
    DefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");
    
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DiableViewer_TransmissionEnabled_TransmissionDisabled)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());

    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        response->m_result = HttpResult::HttpResult_OK;
        callback->OnHttpResponse(response.get());
    };

    DefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");

    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DiableViewer_TransmissionDisabled_TransmissionDisabled)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    DefaultDataViewer viewer(mockHttpClient, "Test");

    ASSERT_FALSE(viewer.IsTransmissionEnabled());
    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, EnableLocalViewer_ThrowsLogicError)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    
    DefaultDataViewer viewer(mockHttpClient, "Test");

    ASSERT_THROW(viewer.EnableLocalViewer(), std::logic_error);
    ASSERT_THROW(viewer.EnableLocalViewer("AppId", "AppPackage"), std::logic_error);
}

TEST(DefaultDataViewerTests, RecieveData_TransmissionNotEnabled_SendsDataToHttpClient)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    bool wasSendRequestAsyncCalled { false };
    mockHttpClient->funcSendRequestAsync = [&wasSendRequestAsyncCalled](MAT::IHttpRequest*, MAT::IHttpResponseCallback*)
    {
        wasSendRequestAsyncCalled = true;
    };

    DefaultDataViewer viewer(mockHttpClient, "Test");

    viewer.ReceiveData(std::vector<std::uint8_t>{});
    ASSERT_FALSE(wasSendRequestAsyncCalled);
}

TEST(DefaultDataViewerTests, RecieveData_TransmissionEnabled_SendsDataToHttpClient)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    int sendRequestAsyncCalledCount { 0 };
    auto l_request = std::shared_ptr<MAT::SimpleHttpRequest>(new SimpleHttpRequest("1"));
    mockHttpClient->funcSendRequestAsync = [&sendRequestAsyncCalledCount, &l_request](MAT::IHttpRequest* request, MAT::IHttpResponseCallback* callback)
    {
        sendRequestAsyncCalledCount++;
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        response->m_result = HttpResult::HttpResult_OK;
        callback->OnHttpResponse(response.get());

        l_request->m_body = request->GetBody();
        l_request->m_headers = request->GetHeaders();
    };

    DefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");
    auto packet = std::vector<std::uint8_t> { 1, 2, 3 };
    viewer.ReceiveData(packet);
    ASSERT_EQ(sendRequestAsyncCalledCount, 2); // first to enable remote and second to send data

    ASSERT_EQ(l_request->GetBody(), packet);
    ASSERT_EQ(l_request->GetHeaders().get("Machine-Identifier"), "Test");
    ASSERT_EQ(l_request->GetHeaders().get("Content-Type"), "Application/bond-compact-binary");
    ASSERT_FALSE(l_request->GetHeaders().get("App-Name").empty());
    ASSERT_FALSE(l_request->GetHeaders().get("App-Platform").empty());
}

TEST(DefaultDataViewerTests, RecieveData_FailToSend_TransmissionDisabled)
{
    auto mockHttpClient = std::shared_ptr<MockHttpClient>(new MockHttpClient());
    bool isRemoteViewerEnabled { false };
    mockHttpClient->funcSendRequestAsync = [&isRemoteViewerEnabled](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        isRemoteViewerEnabled = !isRemoteViewerEnabled;
        if (isRemoteViewerEnabled)
        {
            auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("OK Response"));
            response->m_statusCode = 200;
            response->m_result = HttpResult::HttpResult_OK;
            callback->OnHttpResponse(response.get());
        }
        else
        {
            auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("Failure_Response"));
            response->m_statusCode = 404;
            response->m_result = HttpResult::HttpResult_NetworkFailure;
            callback->OnHttpResponse(response.get());
        }
    };

    DefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");
    auto packet = std::vector<std::uint8_t> { 1, 2, 3 };
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    viewer.ReceiveData(packet);
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}