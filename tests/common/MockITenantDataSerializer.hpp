// Copyright (c) Microsoft. All rights reserved.

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
    class MockITenantDataSerializer : public ITenantDataSerializer
    {
    public:
        ~MockITenantDataSerializer() {}

        MOCK_CONST_METHOD1(SerializeTenantData, std::string(TenantDataPtr tenantData));
        MOCK_CONST_METHOD1(DeserializeTenantData, TenantDataPtr(const std::string& string));
    };
#pragma warning(pop)
} // namespace testing
