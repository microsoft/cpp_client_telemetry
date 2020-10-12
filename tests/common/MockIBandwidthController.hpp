//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
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

