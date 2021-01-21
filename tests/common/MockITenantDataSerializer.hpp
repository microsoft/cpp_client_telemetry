//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//

#pragma once
#include "common/Common.hpp"
#include <ControlPlane/ILocalStorage.hpp>
#include "gmock/gmock.h"

using namespace MAT;
using namespace MAT::ControlPlane;

namespace testing
{
#pragma warning(push)
#pragma warning(disable:4373)

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"  // GMock MOCK_METHOD* macros don't use the override keyword.
#endif

    class MockITenantDataSerializer : public ITenantDataSerializer
    {
    public:
        ~MockITenantDataSerializer() {}

        MOCK_CONST_METHOD1(SerializeTenantData, std::string(TenantDataPtr tenantData));
        MOCK_CONST_METHOD1(DeserializeTenantData, TenantDataPtr(const std::string& string));
    };

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#pragma warning(pop)
} // namespace testing