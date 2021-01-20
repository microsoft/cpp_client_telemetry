//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "Common.hpp"
#include "IHttpClient.hpp"

namespace testing {

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use override.
#endif

class MockIHttpClient : public MAT::IHttpClient {
  public:
    MockIHttpClient();
    virtual ~MockIHttpClient();

    MOCK_METHOD0(CreateRequest, MAT::IHttpRequest * ());
    MOCK_METHOD2(SendRequestAsync, void(MAT::IHttpRequest * request, MAT::IHttpResponseCallback * callback));
    MOCK_METHOD1(CancelRequestAsync, void(std::string const & id));
};

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

} // namespace testing

