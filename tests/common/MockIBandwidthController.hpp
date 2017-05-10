// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Common.hpp"
#include <aria/IBandwidthController.hpp>

namespace testing {


class MockIBandwidthController : public ARIASDK_NS::IBandwidthController
{
  public:
    MOCK_METHOD0(GetProposedBandwidthBps, unsigned());
};


} // namespace testing
