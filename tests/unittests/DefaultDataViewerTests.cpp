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

    void Reset() noexcept
    {
        m_request = nullptr;
        m_responseCallback = nullptr;
        funcSendRequestAsync = nullptr;
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
};

std::shared_ptr<MockHttpClient> mockHttpClient = std::make_shared<MockHttpClient>();

TEST(DefaultDataViewerTests, Constructor_HttpClientNotPassed_HttpClientSetsOrThrowsBasedOnConfig)
{
#ifdef HAVE_MAT_DEFAULT_HTTP_CLIENT
    MockDefaultDataViewer viewer(nullptr, "Test");
    ASSERT_TRUE(viewer.GetHttpClient());
#else
    ASSERT_THROW(MockDefaultDataViewer(nullptr, "Test"), std::invalid_argument);
#endif
}

TEST(DefaultDataViewerTests, Constructor_ValidMachineIdentifier_MachineIdentifierSetCorrectly)
{
    mockHttpClient->Reset();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    ASSERT_EQ(viewer.GetMachineFriendlyIdentifier(), "Test");
}

TEST(DefaultDataViewerTests, Constructor_InvalidMachineIdentifier_ThrowsInvalidArgument)
{
    CheckForExceptionOrAbort<std::invalid_argument>([]() { MockDefaultDataViewer(mockHttpClient, ""); });
    CheckForExceptionOrAbort<std::invalid_argument>([]() { MockDefaultDataViewer(mockHttpClient, "   "); });
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_ValidEndpoint_TransmissionEnabled)
{
    mockHttpClient->Reset();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 200;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");

    ASSERT_TRUE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_NullOrEmptryEndpoint_ThrowsInvalidArgument)
{
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    CheckForExceptionOrAbort<std::invalid_argument>([&viewer]() { viewer.EnableRemoteViewer(""); });
    CheckForExceptionOrAbort<std::invalid_argument>([&viewer]() { viewer.EnableRemoteViewer("           "); });
}

TEST(DefaultDataViewerTests, EnableRemoteViewer_InvalidEndpoint_TransmissionNotEnabled)
{
    mockHttpClient->Reset();
    mockHttpClient->funcSendRequestAsync = [](MAT::IHttpRequest*, MAT::IHttpResponseCallback* callback)
    {
        auto response = std::unique_ptr<MAT::SimpleHttpResponse>(new SimpleHttpResponse("1"));
        response->m_statusCode = 404;
        response->m_result = HttpResult::HttpResult_NetworkFailure;
        callback->OnHttpResponse(response.get());
    };

    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.EnableRemoteViewer("TestEndpoint");

    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DisableViewer_TransmissionEnabled_TransmissionDisabled)
{
    mockHttpClient->Reset();
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

TEST(DefaultDataViewerTests, DiableViewer_TransmissionDisabled_TransmissionDisabled)
{
    mockHttpClient->Reset();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");
    viewer.SetTransmissionEnabled(false);

    ASSERT_FALSE(viewer.IsTransmissionEnabled());
    viewer.DisableViewer();
    ASSERT_FALSE(viewer.IsTransmissionEnabled());
}

TEST(DefaultDataViewerTests, DiableViewer_CallOnDisableNotification_NotificationCalled)
{
    mockHttpClient->Reset();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");

    bool onDisableNotificationCalled { false };
    viewer.RegisterOnDisableNotification([&onDisableNotificationCalled]() noexcept
        {
            onDisableNotificationCalled = true;
        });

    viewer.DisableViewer();
    ASSERT_TRUE(onDisableNotificationCalled);
}

TEST(DefaultDataViewerTests, DiableViewer_CallMultipleOnDisableNotifications_NotificationsCalled)
{
    mockHttpClient->Reset();
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
    mockHttpClient->Reset();
    MockDefaultDataViewer viewer(mockHttpClient, "Test");

    CheckForExceptionOrAbort<std::logic_error>([&viewer]() { viewer.EnableLocalViewer(); });
    CheckForExceptionOrAbort<std::logic_error>([&viewer]() { viewer.EnableLocalViewer("AppId", "AppPackage"); });
}

TEST(DefaultDataViewerTests, ReceiveData_TransmissionNotEnabled_DoesntSendsDataToHttpClient)
{
    mockHttpClient->Reset();
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
    mockHttpClient->Reset();
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
    mockHttpClient->Reset();
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
    mockHttpClient->Reset();
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
    mockHttpClient->Reset();
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
#endif
