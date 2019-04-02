// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Common.hpp"
#include "IHttpClient.hpp"

namespace testing {


class MockIHttpClient : public MAT::IHttpClient {
  public:
    MockIHttpClient();
    virtual ~MockIHttpClient();

    MOCK_METHOD0(CreateRequest, MAT::IHttpRequest * ());
    MOCK_METHOD2(SendRequestAsync, void(MAT::IHttpRequest * request, MAT::IHttpResponseCallback * callback));
    MOCK_METHOD1(CancelRequestAsync, void(std::string const & id));
};


} // namespace testing
