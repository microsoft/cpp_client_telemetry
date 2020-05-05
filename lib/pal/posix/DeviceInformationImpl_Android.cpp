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

#include "sysinfo_sources_impl.hpp"

#define DEFAULT_DEVICE_ID       "{deadbeef-fade-dead-c0de-cafebabefeed}"

namespace PAL_NS_BEGIN {

    class AndroidDeviceInformation;

    class AndroidDeviceInformationConnector {
        private:

        static PowerSource s_power_source;
        static std::string s_device_id;
        static std::string s_manufacturer;
        static std::string s_model;
        static std::mutex s_registered_mutex;
        static std::vector<AndroidDeviceInformation *> s_registered;

        public:

        static void registerDI(AndroidDeviceInformation &di);
        static void unregisterDI(AndroidDeviceInformation &di);
        static void updatePowerSource(PowerSource new_power_source);
        static void setDeviceId(std::string && id);
        static void setManufacturer(std::string && manufacturer);
        static void setModel(std::string && model);
    };

    PowerSource AndroidDeviceInformationConnector::s_power_source = PowerSource_Unknown;
    std::string AndroidDeviceInformationConnector::s_device_id;
    std::string AndroidDeviceInformationConnector::s_manufacturer;
    std::string AndroidDeviceInformationConnector::s_model;
    std::mutex AndroidDeviceInformationConnector::s_registered_mutex;
    std::vector<AndroidDeviceInformation *> AndroidDeviceInformationConnector::s_registered;

    ///// IDeviceInformation API
    DeviceInformationImpl::DeviceInformationImpl(IRuntimeConfig& configuration) :
        m_info_helper(),
        m_powerSource(PowerSource_Battery)
    {}

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
        AndroidDeviceInformation(IRuntimeConfig& configuration) : DeviceInformationImpl(configuration)
        {
            AndroidDeviceInformationConnector::registerDI(*this);
        }

        ~AndroidDeviceInformation()
        {
            AndroidDeviceInformationConnector::unregisterDI(*this);
        }

        void UpdatePowerSource(PowerSource new_power_source)
        {
            m_powerSource = new_power_source;
            m_info_helper.OnChanged(POWER_SOURCE, std::to_string(m_powerSource));
        }

        void SetDeviceInfo(std::string id, std::string manufacturer, std::string model)
        {
            m_device_id = id;
            m_manufacturer = manufacturer;
            m_model = model;
        }
    };

    std::shared_ptr<IDeviceInformation> DeviceInformationImpl::Create(IRuntimeConfig& configuration)
    {
        return std::make_shared<AndroidDeviceInformation>(configuration);
    }

    DeviceInformationImpl::~DeviceInformationImpl() {}

    void AndroidDeviceInformationConnector::registerDI(AndroidDeviceInformation &di)
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
        di.SetDeviceInfo(s_device_id, s_manufacturer, s_model);
    }

    void AndroidDeviceInformationConnector::unregisterDI(AndroidDeviceInformation &di)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        auto new_end = std::remove_if(s_registered.begin(), s_registered.end(), [&di] (AndroidDeviceInformation *r)->bool
        {
            return r == &di;
        });
        s_registered.erase(new_end, s_registered.end());
    }

    void AndroidDeviceInformationConnector::updatePowerSource(PowerSource new_power_source)
    {
        std::lock_guard<std::mutex> lock(s_registered_mutex);
        s_power_source = new_power_source;
        for (auto && di : s_registered) {
            di->UpdatePowerSource(s_power_source);
        }
    }

    void AndroidDeviceInformationConnector::setDeviceId(std::string&& id)
    {
        s_device_id = std::move(id);
    }

    void AndroidDeviceInformationConnector::setManufacturer(std::string&& manufacturer)
    {
        s_manufacturer = std::move(manufacturer);
    }

    void AndroidDeviceInformationConnector::setModel(std::string&& model)
    {
        s_model = std::move(model);
    }
} PAL_NS_END

extern "C"
JNIEXPORT void

JNICALL
Java_com_microsoft_applications_events_HttpClient_onPowerChange(JNIEnv* env,
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
            PAL::AndroidDeviceInformationConnector::updatePowerSource(new_power_source);
        }
    }

    extern "C" JNIEXPORT void JNICALL Java_com_microsoft_applications_events_HttpClient_setDeviceInfo(
        JNIEnv *env,
        jobject /* java_client */,
        jstring id,
        jstring manufacturer,
        jstring model
    )
    {
        auto start = env->GetStringUTFChars(id, nullptr);
        auto end = start + env->GetStringUTFLength(id);
        PAL::AndroidDeviceInformationConnector::setDeviceId(std::string(start, end));
        env->ReleaseStringUTFChars(id, start);

        start = env->GetStringUTFChars(manufacturer, nullptr);
        end = start + env->GetStringUTFLength(manufacturer);
        PAL::AndroidDeviceInformationConnector::setManufacturer(std::string(start, end));
        env->ReleaseStringUTFChars(manufacturer, start);

        start = env->GetStringUTFChars(model, nullptr);
        end = start + env->GetStringUTFLength(model);
        PAL::AndroidDeviceInformationConnector::setModel(std::string(start, end));
        env->ReleaseStringUTFChars(model, start);
    }
