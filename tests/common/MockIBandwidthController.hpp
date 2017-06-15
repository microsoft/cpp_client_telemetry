// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "utils/Utils.hpp"
#include <IBandwidthController.hpp>

namespace testing {


class MockIBandwidthController : public ARIASDK_NS::IBandwidthController
{
  public:
    MOCK_METHOD0(GetProposedBandwidthBps, unsigned());
};


} // namespace testing
