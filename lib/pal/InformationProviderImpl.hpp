//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef INFORMATIONPROVIDERIMPL_HPP
#define INFORMATIONPROVIDERIMPL_HPP

#include "pal/PAL.hpp"
#include "IInformationProvider.hpp"

#include <vector>
#include <mutex>
#include <string>

namespace PAL_NS_BEGIN {

    class InformatonProviderImpl : public IInformationProvider
    {
    public:
        InformatonProviderImpl();
        ~InformatonProviderImpl();

        // IInformationProvider API
        int RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback) override;
        void UnRegisterInformationChangedCallback(int callbackToken) override;

        // Helper
        void OnChanged(std::string const& propertyName, std::string const& propertyValue);

    private:
        std::mutex m_lock;
        std::vector<IPropertyChangedCallback*> m_callbacks;
        int m_registeredCount;
    };

} PAL_NS_END

#endif

