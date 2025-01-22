//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "DataViewer_CAPI.hpp"


namespace MAT_NS_BEGIN {
    DataViewer_CAPI::DataViewer_CAPI(dataviewer_callback_fn_t callbackFn)
    : m_callbackFn(callbackFn),
      m_endpoint("DataViewer_CAPI")
    {

    }

    void DataViewer_CAPI::ReceiveData(const std::vector<uint8_t>& packetData) noexcept
    {
        m_callbackFn(packetData.data(), packetData.size());
    }
    
    const char* DataViewer_CAPI::GetName() const noexcept
    {
        return s_name;
    }

    bool DataViewer_CAPI::IsTransmissionEnabled() const noexcept
    {
        return true;
    }

    const std::string& DataViewer_CAPI::GetCurrentEndpoint() const noexcept
    {
        return m_endpoint;
    }

    const char* DataViewer_CAPI::s_name = "DataViewer_CAPI";
} MAT_NS_END