//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MOCKITENANTDATASERIALIZER_HPP
#define MOCKITENANTDATASERIALIZER_HPP
#include "common/Common.hpp"
#include <ControlPlane/ILocalStorage.hpp>
#include "gmock/gmock.h"
#include "mat/CompilerWarnings.hpp"

using namespace MAT;
using namespace MAT::ControlPlane;

namespace testing
{
    class MockITenantDataSerializer : public ITenantDataSerializer
    {
    public:
        ~MockITenantDataSerializer() noexcept = default;

MAT_PUSH_WARNINGS
MAT_DISABLE_WARNING_INCONSISTENT_MISSING_OVERRIDE
        MOCK_CONST_METHOD1(SerializeTenantData, std::string(TenantDataPtr tenantData));
        MOCK_CONST_METHOD1(DeserializeTenantData, TenantDataPtr(const std::string& string));
MAT_POP_WARNINGS
    };

} // namespace testing

#endif // #ifndef MOCKITENANTDATASERIALIZER_HPP