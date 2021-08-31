//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#include "pal/PAL.hpp"
#include "pal/SystemInformationImpl.hpp"

#include <sys/utsname.h>
#include "sysinfo_sources_impl.hpp"
#include <string>

namespace PAL_NS_BEGIN {

    SystemInformationImpl::SystemInformationImpl(IRuntimeConfig& configuration) : m_info_helper()
    {
        auto sysInfo = sysinfo_sources_impl::GetSysInfo();
        m_user_timezone = sysInfo.get("tz");
        m_app_id = sysInfo.get("appId");
        m_os_name = sysInfo.get("osName");
        m_os_major_version = sysInfo.get("osVer");
        m_os_full_version = sysInfo.get("osRel");
    }

    SystemInformationImpl::~SystemInformationImpl()
    {
    }

    int SystemInformationImpl::RegisterInformationChangedCallback(IPropertyChangedCallback* pCallback)
    {
        return m_info_helper.RegisterInformationChangedCallback(pCallback);
    }

    void SystemInformationImpl::UnRegisterInformationChangedCallback(int callbackToken)
    {
        m_info_helper.UnRegisterInformationChangedCallback(callbackToken);
    }

    std::shared_ptr<ISystemInformation> SystemInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<SystemInformationImpl>(configuration);
    }

} PAL_NS_END

