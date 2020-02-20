// Copyright (c) Microsoft Corporation. All rights reserved.
#define LOG_MODULE DBG_API
#include "pal/PAL.hpp"
#include "pal/DeviceInformationImpl.hpp"

#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <locale>
#include <codecvt>

#include "sysinfo_sources.hpp"

#define DEFAULT_DEVICE_ID       "{deadbeef-fade-dead-c0de-cafebabefeed}"

namespace PAL_NS_BEGIN {

    class AndroidDeviceInformation;

    class AndroidPowerSourceConnector {
        private:

        static PowerSource s_power_source;
        static std::mutex s_registered_mutex;
        static std::vector<AndroidDeviceInformation *> s_registered;
        
        public:

        static void registerDI(AndroidDeviceInformation &di);
        static void unregisterDI(AndroidDeviceInformation &di);
        static void updatePowerSource(PowerSource new_power_source);
    };

    PowerSource AndroidPowerSourceConnector::s_power_source = PowerSource_Unknown;
    std::mutex AndroidPowerSourceConnector::s_registered_mutex;
    std::vector<AndroidDeviceInformation *> AndroidPowerSourceConnector::s_registered;

    ///// IDeviceInformation API
    DeviceInformationImpl::DeviceInformationImpl() :
                                m_info_helper()
    {
        // Since nothing looks at this, let's just say unknown
        m_os_architecture = OsArchitectureType_Unknown;

        std::string devId = aria_hwinfo.get("devId");
        m_device_id = (devId.empty()) ? DEFAULT_DEVICE_ID : devId;

        m_manufacturer = aria_hwinfo.get("devMake");

        m_model = aria_hwinfo.get("devModel");

        m_powerSource = PowerSource_Battery;

    }

    std::string DeviceInformationImpl::GetDeviceTicket() const
    {
        return m_deviceTicket;
    }

#ifdef ENABLE_LEGACY_CODE
    size_t DeviceInformationImpl::GetMemorySize() const
    {
        return 0;
    }
#endif

    class AndroidDeviceInformation : public DeviceInformationImpl {
        public:
        AndroidDeviceInformation()
        {
            AndroidPowerSourceConnector::registerDI(*this);
        }

        ~AndroidDeviceInformation()
        {
            AndroidPowerSourceConnector::unregisterDI(*this);
        }

        void UpdatePowerSource(PowerSource new_power_source)
        {
            m_powerSource = new_power_source;
            m_info_helper.OnChanged(POWER_SOURCE, std::to_string(m_powerSource));
        }
    };

    IDeviceInformation* DeviceInformationImpl::Create()
    {
        return new AndroidDeviceInformation();
    }

    DeviceInformationImpl::~DeviceInformationImpl() {}

    void AndroidPowerSourceConnector::registerDI(AndroidDeviceInformation &di)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        for (auto&& e : s_registered)
        {
            if (e == &di)
            {
                // only register di once
                return;
            }
        }
        s_registered.push_back(&di);
        di.UpdatePowerSource(s_power_source);
    }

    void AndroidPowerSourceConnector::unregisterDI(AndroidDeviceInformation &di)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        auto new_end = std::remove_if(s_registered.begin(), s_registered.end(), [&di] (AndroidDeviceInformation *r)->bool
        {
            return r == &di;
        });
        s_registered.erase(new_end, s_registered.end());
    }

    void AndroidPowerSourceConnector::updatePowerSource(PowerSource new_power_source)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        s_power_source = new_power_source;
        for (auto && di : s_registered) {
            di->UpdatePowerSource(s_power_source);
        }
    }
    
} PAL_NS_END

extern "C"
JNIEXPORT void

JNICALL
Java_com_microsoft_applications_events_httpClient_onPowerChange(JNIEnv* env,
	jobject /* java_client */,
    jboolean isCharging, 
    jboolean isLow)
    {
        PowerSource new_power_source = PowerSource_Charging;
        if (!isCharging)
        {
            if (isLow)
            {
                new_power_source = PowerSource_LowBattery;
            }
            else
            {
                new_power_source = PowerSource_Battery;
            }
            PAL::AndroidPowerSourceConnector::updatePowerSource(new_power_source);
        }
    }
