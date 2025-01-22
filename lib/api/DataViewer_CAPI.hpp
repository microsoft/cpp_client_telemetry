//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DATAVIEWER_CAPI_HPP
#define DATAVIEWER_CAPI_HPP


#include "IDataViewer.hpp"
#include "mat.h"

namespace MAT_NS_BEGIN {
    class DataViewer_CAPI : public IDataViewer {
    public:
        DataViewer_CAPI(dataviewer_callback_fn_t callbackFn);
        void ReceiveData(const std::vector<uint8_t>& packetData) noexcept override;

        const char* GetName() const noexcept override;
        bool IsTransmissionEnabled() const noexcept override;
        const std::string& GetCurrentEndpoint() const noexcept override;

    private:
        dataviewer_callback_fn_t m_callbackFn;
        static const char* s_name;
        std::string m_endpoint;
    };
} MAT_NS_END

#endif // DATAVIEWER_CAPI_HPP