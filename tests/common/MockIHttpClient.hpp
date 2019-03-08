// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Common.hpp"
#include "IHttpClient.hpp"

namespace testing {


class MockIHttpClient : public ARIASDK_NS::IHttpClient {
  public:
    MockIHttpClient();
    virtual ~MockIHttpClient();

    MOCK_METHOD0(CreateRequest, ARIASDK_NS::IHttpRequest * ());
    MOCK_METHOD2(SendRequestAsync, void(ARIASDK_NS::IHttpRequest * request, ARIASDK_NS::IHttpResponseCallback * callback));
    MOCK_METHOD1(CancelRequestAsync, void(std::string const & id));
};


} // namespace testing
