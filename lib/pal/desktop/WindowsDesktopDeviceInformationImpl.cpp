//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#define LOG_MODULE DBG_API
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif
#include "pal/PAL.hpp"
#include "pal/DeviceInformationImpl.hpp"
#include "utils/StringUtils.hpp"
#include "WindowsEnvironmentInfo.hpp"

MATSDK_LOG_INST_COMPONENT_NS("DeviceInfo", "Win32 Desktop Device Information")

#include <winsock2.h>
#include <iphlpapi.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <locale>
#include <codecvt>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "AdvAPI32.Lib")

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

#define SYSINFO         "SYSTEM\\CurrentControlSet\\Control\\SystemInformation"
#define MANUFACTURER    "SystemManufacturer"
#define MODEL           "SystemProductName"
#define HARDWARE_ID     "ComputerHardwareId"

using namespace MAT;

namespace PAL_NS_BEGIN {

    /* Value returned on computers with no network adapter available */
    static const char *devIdDefault = "{deadbeef-fade-dead-c0de-cafebabefeed}";
    static const char *manufacturer = "Unknown Manufacturer";
    static const char *model = "Unknown Model";

    /**
     * Returns the GUID of the 1st network adapter.
     */
    std::string getDeviceId()
    {
        std::string devId = devIdDefault;
        ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

    retry_bigger_buffer:
        PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *)MALLOC(ulOutBufLen);
        if (pAdapterInfo != NULL)
        {
            pAdapterInfo->AdapterName[0] = 0;
            DWORD result = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
            if (result == ERROR_BUFFER_OVERFLOW)
            {
                /* The buffer to receive the adapter information is too small.
                 * This value is returned if the buffer size indicated by the ulOutBufLen parameter
                 * is too small to hold the adapter information... When this error code is returned,
                 * the ulOutBufLen parameter contains the required buffer size, so we retry with
                 * suggested value of ulOutBufLen, assuming it is bigger than the default.
                 */
                if (ulOutBufLen > sizeof(IP_ADAPTER_INFO))
                {
                    FREE(pAdapterInfo);
                    goto retry_bigger_buffer;
                }
            }
            if ((result == ERROR_SUCCESS) && (pAdapterInfo->AdapterName[0] != 0))
            {
                std::string adapterName{ toLower(pAdapterInfo->AdapterName) };
                devId = adapterName;
            }
            FREE(pAdapterInfo);
        }
        return devId;
    }

    // Helper functions.
    PowerSource GetCurrentPowerSource()
    {
        PowerSource result = PowerSource_Unknown;
        SYSTEM_POWER_STATUS systemPowerStatus;
        if (GetSystemPowerStatus(&systemPowerStatus)) {
            // ACLineStatus - The AC power status.This member can be one of the following values.
            // Value    Meaning
            // 0        Offline
            // 1        Online
            // 255      Unknown status
            if (systemPowerStatus.ACLineStatus == 1) {
                result = PowerSource_Charging;
            } else
                if (systemPowerStatus.ACLineStatus == 0) {
                    result = PowerSource_Battery;
                }
        }
        return result;
    }

    std::string DeviceInformationImpl::GetDeviceTicket() const
    {
        return m_deviceTicket;
    }

    ///// IDeviceInformation API
    DeviceInformationImpl::DeviceInformationImpl(MAT::IRuntimeConfig& /*configuration*/) :
            m_info_helper(),
            m_registeredCount(0)
    {
        m_os_architecture = WindowsEnvironmentInfo::GetProcessorArchitecture();

        m_device_id = getDeviceId();

        char buff[256] = { 0 };
        DWORD size = sizeof(buff);

        // Detect manufacturer
        m_manufacturer = manufacturer;
        if (ERROR_SUCCESS == RegGetValueA(HKEY_LOCAL_MACHINE, (SYSINFO), (MANUFACTURER), RRF_RT_REG_SZ, NULL, &buff, &size)) {
            const std::string tmp(buff);
            m_manufacturer = tmp;
        }
        LOG_TRACE("Device Manufacturer=%s", m_manufacturer.c_str());

        // Detect model
        size = sizeof(buff);
        m_model = model;
        if (ERROR_SUCCESS == RegGetValueA(HKEY_LOCAL_MACHINE, (SYSINFO), (MODEL), RRF_RT_REG_SZ, NULL, &buff, &size)) {
            std::string tmp(buff);
            m_model = tmp;
        }
        LOG_TRACE("Device Model=%s", m_model.c_str());

        m_powerSource = GetCurrentPowerSource();

    }
    DeviceInformationImpl::~DeviceInformationImpl()
    {
    }

    std::shared_ptr<IDeviceInformation> DeviceInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<DeviceInformationImpl>(configuration);
    }

} PAL_NS_END

