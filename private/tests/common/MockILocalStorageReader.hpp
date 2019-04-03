#if 0
// Copyright (c) Microsoft. All rights reserved.

#pragma once

#include "Common.hpp"
#include "controlplane/ILocalStorage.hpp"
#include <gmock/gmock.h>

namespace testing
{
    class MockILocalStorageReader : public MAT::ControlPlane::ILocalStorageReader
    {
    public:
        MOCK_CONST_METHOD1(ReadTenantData, MAT::ControlPlane::TenantDataPtr(const MAT::GUID_t& ariaTenantId));
        MOCK_METHOD1(RegisterChangeEventHandler, void(MAT::ControlPlane::ILocalStorageChangeEventHandler * handler));
        MOCK_METHOD1(UnregisterChangeEventHandler, void(MAT::ControlPlane::ILocalStorageChangeEventHandler * handler));
    };

    class NotifiableMockILocalStorageReader : public MockILocalStorageReader
    {
    public:
        MAT::ControlPlane::ILocalStorageChangeEventHandler * m_handler;

        NotifiableMockILocalStorageReader()
        {
            m_handler = nullptr;
        }

        void RegisterChangeEventHandler(MAT::ControlPlane::ILocalStorageChangeEventHandler * handler) override
        {
            m_handler = handler;
            MockILocalStorageReader::RegisterChangeEventHandler(handler);
        }

        void UnregisterChangeEventHandler(MAT::ControlPlane::ILocalStorageChangeEventHandler * handler) override
        {
            m_handler = nullptr;
            MockILocalStorageReader::UnregisterChangeEventHandler(handler);
        }
    };
} // namespace testing
#endif
