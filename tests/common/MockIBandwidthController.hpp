//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "utils/Utils.hpp"
#include "IBandwidthController.hpp"

namespace testing {


class MockIBandwidthController : public MAT::IBandwidthController
{
  public:
    MOCK_METHOD0(GetProposedBandwidthBps, unsigned());
};


} // namespace testing

