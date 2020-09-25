// Copyright (c) Microsoft. All rights reserved.
#include "common/Common.hpp"
#include "CheckForExceptionOrAbort.hpp"

#define HAVE_MAT_DEFAULTDATAVIEWER

#if defined __has_include
#  if __has_include ("modules/dataviewer/DefaultDataViewer.hpp")
#    include "modules/dataviewer/DefaultDataViewer.hpp"
#  else
   /* Compiling without Data Viewer */
#  undef HAVE_MAT_DEFAULTDATAVIEWER
#  endif
#endif

#ifdef HAVE_MAT_DEFAULTDATAVIEWER
#include "public/IHttpClient.hpp"
#include <future>

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
        m_request = nullptr;
        m_responseCallback = nullptr;
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

    std::function<void(std::string const&)> fnCancelRequestAsync;
    void CancelRequestAsync(std::string const& requestId) override
    {
        if (fnCancelRequestAsync)
        {
            fnCancelRequestAsync(requestId);
        }
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

class MockDefaultDataViewer : public MAT::DefaultDataViewer
{
public:
    MockDefaultDataViewer(const std::shared_ptr<IHttpClient>& httpClient, const char* machineFriendlyIdentifier)
        : DefaultDataViewer(httpClient, machineFriendlyIdentifier) {}

    using MAT::DefaultDataViewer::GetHttpClient;
    using MAT::DefaultDataViewer::GetMachineFriendlyIdentifier;
    using MAT::DefaultDataViewer::IsTransmissionEnabled;
    using MAT::DefaultDataViewer::SetTransmissionEnabled;
    using MAT::DefaultDataViewer::GetCurrentEndpoint;
};

TEST(DefaultDataViewerTests, Constructor_HttpClientNotPassed_HttpClientSetsOrThrowsInvalidArgumentBasedOnConfig)
{
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
    MockDefaultDataViewer viewer(nullptr, "Test");
    ASSERT_TRUE(viewer.GetHttpClient());
#else
    CheckForExceptionOrAbort<std::invalid_argument>({ MockDefaultDataViewer(nullptr, "Test"); });
#endif
}

TEST(DefaultDataViewerTests, Constructor_ValidMachineIdentifier_MachineIdentifierSetCorrectly)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    ASSERT_EQ(viewer.GetMachineFriendlyIdentifier(), "Test");
}

TEST(DefaultDataViewerTests, Constructor_InvalidMachineIdentifier_ThrowsInvalidArgument)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    CheckForExceptionOrAbort<std::invalid_argument>([&mockHttpClient]() { MockDefaultDataViewer(mockHttpClient, ""); });
    CheckForExceptionOrAbort<std::invalid_argument>([&mockHttpClient]() { MockDefaultDataViewer(mockHttpClient, "   "); });
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_ValidEndpoint_TransmissionEnabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("http://10.0.0.1");
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
    viewer.EnableRemoteViewer("HTTP://10.0.0.1");
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, GetEndpoint_CorrectEndpointReturned)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    ASSERT_EQ("", viewer.GetCurrentEndpoint());
    const std::string endpoint{"http://10.0.0.1"};
    viewer.EnableRemoteViewer(endpoint);
    ASSERT_EQ(endpoint, viewer.GetCurrentEndpoint());

    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}

//TODO Uncomment this test when the submodule has been updated.
TEST(DefaultDataViewerTests, IsValidRemoteEndpoint_InvalidEndpoint_ReturnsFalse)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint(""));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint(""));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("           "));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("TestEndpoint"));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("https://10.0.0.1"));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("HTTps://10.0.0.1"));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("http://14.0.0.1"));
    ASSERT_FALSE(viewer.IsValidRemoteEndpoint("http://192.168.999.999"));
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_NonRespondingEndpoint_TransmissionNotEnabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 404;
        response->m_result = HttpResult::HttpResult_NetworkFailure;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("http://10.0.0.1");

    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DisableViewer_TransmissionEnabled_TransmissionDisabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(true);

    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DisableViewer_TransmissionDisabled_TransmissionDisabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(false);

    ASSERT_FALSE(viewer.IsTransmissionEnabled());
    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DisableViewer_CallOnDisableNotification_NotificationCalled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");

    bool onDisableNotificationCalled { false };
    viewer.RegisterOnDisableNotification([&onDisableNotificationCalled]() noexcept
        {
            onDisableNotificationCalled = true;
        });

    viewer.DisableViewer();
    ASSERT_TRUE(onDisableNotificationCalled);
}

TEST(DefaultDataViewerTests, DisableViewer_CallMultipleOnDisableNotifications_NotificationsCalled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");

    int onDisableNotificationCalled { 0 };
    std::function<void()> notificationA = [&onDisableNotificationCalled]() noexcept
    {
        onDisableNotificationCalled++;
    };
    std::function<void()> notificationB = [&onDisableNotificationCalled]() noexcept
    {
        onDisableNotificationCalled++;
    };
    std::function<void()> notificationC = [&onDisableNotificationCalled]() noexcept
    {
        onDisableNotificationCalled++;
    };

    viewer.RegisterOnDisableNotification(notificationA);
    viewer.RegisterOnDisableNotification(notificationB);
    viewer.RegisterOnDisableNotification(notificationC);

    viewer.DisableViewer();
    ASSERT_EQ(onDisableNotificationCalled, 3);
}

TEST(DefaultDataViewerTests, EnableLocalViewer_ThrowsLogicError)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");

    CheckForExceptionOrAbort<std::logic_error>([&viewer]() { viewer.EnableLocalViewer(); });
    CheckForExceptionOrAbort<std::logic_error>([&viewer]() { viewer.EnableLocalViewer("AppId", "AppPackage"); });
}

TEST(DefaultDataViewerTests, ReceiveData_TransmissionNotEnabled_DoesntSendsDataToHttpClient)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    bool wasSendRequestAsyncCalled { false };
    mockHttpClient->funcSendRequestAsync = [&wasSendRequestAsyncCalled](MAT::IHttpRequest*, MAT::IHttpResponseCallback*)
    {
        wasSendRequestAsyncCalled = true;
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.ReceiveData(std::vector<uint8_t>{});
    ASSERT_FALSE(wasSendRequestAsyncCalled);
}

TEST(DefaultDataViewerTests, ReceiveData_TransmissionEnabled_SendsCorrectBodyToHttpClient)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    int sendRequestAsyncCalledCount { 0 };
    auto requestToValidate = std::shared_ptr<MAT::SimpleHttpRequest>(new SimpleHttpRequest("1"));
    mockHttpClient->funcSendRequestAsync = [&sendRequestAsyncCalledCount, &requestToValidate](MAT::IHttpRequest* request, MAT::IHttpResponseCallback* callback)
    {
        sendRequestAsyncCalledCount++;
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());

        requestToValidate->m_body = request->GetBody();
        requestToValidate->m_headers = request->GetHeaders();
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(true);
    auto packet = std::vector<uint8_t> { 1, 2, 3 };
    viewer.ReceiveData(packet);

    ASSERT_EQ(sendRequestAsyncCalledCount, 1);
    ASSERT_EQ(requestToValidate->GetBody(), packet);
}

TEST(DefaultDataViewerTests, ReceiveData_TransmissionEnabled_SendsCorrectHeadersToHttpClient)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    int sendRequestAsyncCalledCount { 0 };
    auto requestToValidate = std::shared_ptr<MAT::SimpleHttpRequest>(new SimpleHttpRequest("1"));
    mockHttpClient->funcSendRequestAsync = [&sendRequestAsyncCalledCount, &requestToValidate](MAT::IHttpRequest* request, MAT::IHttpResponseCallback* callback)
    {
        sendRequestAsyncCalledCount++;
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());

        requestToValidate->m_body = request->GetBody();
        requestToValidate->m_headers = request->GetHeaders();
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(true);
    viewer.ReceiveData(std::vector<uint8_t> { });

    ASSERT_EQ(sendRequestAsyncCalledCount, 1);
    ASSERT_EQ(requestToValidate->GetHeaders().get("Machine-Identifier"), "Test");
    ASSERT_EQ(requestToValidate->GetHeaders().get("Content-Type"), "Application/bond-compact-binary");
    ASSERT_FALSE(requestToValidate->GetHeaders().get("App-Name").empty());
    ASSERT_FALSE(requestToValidate->GetHeaders().get("App-Platform").empty());
}

TEST(DefaultDataViewerTests, ReceiveData_PacketGoesOutOfScope_SendsCorrectPacketToClient)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    int sendRequestAsyncCalledCount { 0 };
    auto requestToValidate = std::shared_ptr<MAT::SimpleHttpRequest>(new SimpleHttpRequest("1"));
    mockHttpClient->funcSendRequestAsync = [&sendRequestAsyncCalledCount, &requestToValidate](MAT::IHttpRequest* request, MAT::IHttpResponseCallback* callback)
    {
        sendRequestAsyncCalledCount++;
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());

        requestToValidate->m_body = request->GetBody();
        requestToValidate->m_headers = request->GetHeaders();
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(true);

    {
        auto packet = std::vector<uint8_t> { 1, 2, 3 };
        viewer.ReceiveData(packet);
    }

    ASSERT_EQ(sendRequestAsyncCalledCount, 1);
    ASSERT_EQ(requestToValidate->GetBody(), (std::vector<uint8_t> { 1, 2, 3 }));
}

TEST(DefaultDataViewerTests, ReceiveData_FailToSend_TransmissionDisabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("Failure_Response"));
        response->m_statusCode = 404;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(true);
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    viewer.ReceiveData(std::vector<uint8_t> { 1, 2, 3 });
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_ReceiveData_DataReceivedCorrectly)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    auto httpCalls{0};
    mockHttpClient->funcSendRequestAsync = [&httpCalls](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback) {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());
        httpCalls++;
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("http://10.0.0.1");
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    viewer.ReceiveData(std::vector<uint8_t>{1, 2, 3});
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
    ASSERT_EQ(httpCalls, 2);
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_SendRequestTimeout_TransmissionEnabledOnRetry)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    std::future<void> discardFuture;
    mockHttpClient->funcSendRequestAsync = [&discardFuture](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback) {
        discardFuture = std::async(std::launch::async, [callback]() {
            std::this_thread::sleep_for(std::chrono::seconds(35));
            auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
            response->m_statusCode = 200;
            callback->OnHttpResponse(response.get());
        });
    };

    auto cancelRequestCalled = false;
    mockHttpClient->fnCancelRequestAsync = [&cancelRequestCalled](std::string const&)
    {
        cancelRequestCalled = true;
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("http://10.0.0.1");
    ASSERT_FALSE(viewer.IsTransmissionEnabled());

    // This sleep is for test only as we are mocking out the HttpClient
    // which does not convey CancelRequest information.
    // As such, we are validating cancelled is called correctly below.
    ASSERT_TRUE(cancelRequestCalled);
    std::this_thread::sleep_for(std::chrono::seconds(10));

    mockHttpClient->funcSendRequestAsync = [&discardFuture](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback) {
        discardFuture = std::async(std::launch::async, [callback]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
            response->m_statusCode = 200;
            callback->OnHttpResponse(response.get());
        });
    };
    viewer.EnableRemoteViewer("http://10.0.0.1");
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_SendRequestTakes20Seconds_TransmissionEnabled)
{
    auto mockHttpClient = std::make_shared<MockHttpClient>();
    std::future<void> discardFuture;
    mockHttpClient->funcSendRequestAsync = [&discardFuture](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback) {
        discardFuture = std::async(std::launch::async, [callback]() {
            std::this_thread::sleep_for(std::chrono::seconds(20));
            auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
            response->m_statusCode = 200;
            callback->OnHttpResponse(response.get());
        });
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("http://10.0.0.1");
    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}
#endif
