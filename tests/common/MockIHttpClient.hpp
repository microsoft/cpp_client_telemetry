// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Common.hpp"
#include "IHttpClient.hpp"

namespace testing {


class MockIHttpClient : public MAT::IHttpClient {
  public:
    MockIHttpClient();
    virtual ~MockIHttpClient();

	 virtual std::unique_ptr<IHttpRequest> CreateRequest()
	 {
		 return std::unique_ptr<IHttpRequest>(CreateRequestProxy());
	 }

    MOCK_METHOD0(CreateRequestProxy, IHttpRequest*());
    MOCK_METHOD2(SendRequestAsync, void(MAT::IHttpRequest& request, MAT::IHttpResponseCallback * callback));
    MOCK_METHOD1(CancelRequestAsync, void(std::string const & id));
};


} // namespace testing
