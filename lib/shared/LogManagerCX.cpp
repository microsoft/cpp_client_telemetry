//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#define LOG_MODULE DBG_API

#include "pch.h"
#include "LogManagerCX.hpp"
#include "LoggerCX.hpp"
#include "SemanticContextCX.hpp"
#include "LogConfigurationCX.hpp"
#include "EventPropertiesCX.hpp"

#include "TimedEvent.hpp"
#include "ITestCallback.h"

#include "LogManager.hpp"

#ifdef _WINRT_DLL
#include "WindowsRTPlatformEventHandler.h"
#include "LogManagerEventReceiver.h"
#endif

#include "LogManager.hpp"

namespace MAT_NS_BEGIN { DEFINE_LOGMANAGER(LogManager, ModuleLogConfiguration); } MAT_NS_END

namespace MATW_NS_BEGIN {

    const string OfflineStorageFileExtension = ".storage";

#if !((_MANAGED == 1) || (_M_CEE == 1))
    /* static data members of managed types must be defined within the class definition */
    bool LogManager::isInited = false;
#endif

#ifdef _WINRT_DLL
    SendEventTestCallback^      LogManager::m_eventCallaback        = nullptr;
    PlatformEventHandler^       LogManager::m_platformEventHandler  = nullptr;
    ITestCallback*              LogManager::m_testCallback          = nullptr;
    IPlatformEventReceiver*     LogManager::m_eventReceiver         = nullptr;
    String^                     LogManager::startProfileName;
    String^                     LogManager::transmitProfiles;
#endif

    class LogManagerTestCallback : public ITestCallback
    {
    public:
        virtual void TestCallback(const MAT::EventProperties& evt)
        {
            if (IsTestCallbackSet())
            {
                LogManager::TestCallback(platform_new EventProperties(evt));
            }
        }

        virtual bool IsTestCallbackSet()
        {
            return LogManager::TestCallback != nullptr;
        }

        LogManagerTestCallback()
        {
        }
    };

    ILogger^ LogManager::Initialize(String^ tenantToken)
    {
        auto configuration = platform_new LogConfiguration();

        return Initialize(tenantToken, configuration);
    }

    ILogger^ LogManager::Initialize(String^ tenantToken, LogConfiguration^ configuration)
    {
        //LOG_TRACE("LogManager::Initialize token=%s, configuration=[...]",FromPlatformString(tenantToken).c_str() );
        m_testCallback = new LogManagerTestCallback();

        configuration->TenantToken = tenantToken;
        auto token = FromPlatformString(tenantToken);

#ifdef _WINRT_DLL
        //LOG_TRACE("running as _WINRT_DLL");	
        using namespace ::Windows::Storage;
        m_eventReceiver = new LogManagerEventReceiver(configuration);
        m_platformEventHandler = ref new PlatformEventHandler();
        m_platformEventHandler->RegisterReceiver(m_eventReceiver);

        // If consumer doesn't specify the offline storage path, enable the default one for WinRT
        if (IsPlatformStringEmpty(configuration->OfflineStorage))
        {
            // Get apps local folder
            StorageFolder^ localFolder = ApplicationData::Current->LocalFolder;
            // Offline storage path
            Platform::String^ offlineFile = ToPlatformString(MAT::tenantTokenToId(token) + OfflineStorageFileExtension);
            Platform::String^ offlinePath = localFolder->Path->ToString() + "\\" + offlineFile;
            // Print dir and file path
            std::string path = FromPlatformString(offlinePath).c_str();
            //LOG_TRACE("dir  = %s", FromPlatformString(localFolder->Path).c_str());
            //LOG_TRACE("path = %s", FromPlatformString(offlinePath).c_str());
            // Set configuration parameter
            configuration->OfflineStorage = offlinePath;
        }
#endif
        isInited = true;

        // Pass down transmit profiles and initial profile name to core
        if (startProfileName == nullptr)
        {
            startProfileName = "";
        }
        if (!startProfileName->Equals(""))
        {
            configuration->StartProfileName = startProfileName;
        }
        if (transmitProfiles == nullptr)
        {
            transmitProfiles = "";
        }
        if (!transmitProfiles->Equals(""))
        {
            configuration->TransmitProfiles = transmitProfiles;
        }

        configuration->ToLogConfigurationCore();
        return platform_new Logger(MAT::LogManager::Initialize(token));
    }

    ILogger^ LogManager::GetLogger()
    {
        //LOG_TRACE("LogManager::GetLogger()");
        return platform_new Logger(MAT::LogManager::GetLogger());

    }

    ILogger^ LogManager::GetLogger(String^ tenantToken, String^ source)
    {
        //LOG_TRACE("LogManager::GetLogger[2]");
        return platform_new Logger(MAT::LogManager::GetLogger(FromPlatformString(tenantToken), FromPlatformString(source)));
    }

    ILogger^ LogManager::GetLogger(String^ source)
    {
        //LOG_TRACE("LogManager::GetLogger[1]");
        return platform_new Logger(MAT::LogManager::GetLogger(FromPlatformString(source)));
    }

    void LogManager::PauseTransmission()
    {
        //LOG_TRACE("LogManager::PauseTransmission()");
        MAT::LogManager::PauseTransmission();
    }

    void LogManager::Flush()
    {
        //LOG_TRACE("LogManager::Flush()");
        MAT::LogManager::Flush();
    }

    void LogManager::FlushAndTeardown()
    {
        MAT::LogManager::FlushAndTeardown();
        reset();
    }

    void LogManager::UploadNow()
    {
        MAT::LogManager::UploadNow();
    }

    void LogManager::ResumeTransmission()
    {
        //LOG_TRACE("LogManager::ResumeTransmission()");
        MAT::LogManager::ResumeTransmission();
    }

    bool LogManager::LoadTransmitProfiles(String^ json)
    {
        if (!isInited)
        {
            transmitProfiles = json;
            // Caller must ensure that the supplied JSON string is valid.
            return true;
        }

        //LOG_TRACE("LogManager::LoadTransmitProfiles()");
        const std::string profiles = FromPlatformString(json);
        return MAT::LogManager::LoadTransmitProfiles(profiles);
    }
    void LogManager::SetTransmitProfile(TransmitProfile profile)
    {
        //LOG_TRACE("LogManager::SetTransmitProfile()");
        MAT::LogManager::SetTransmitProfile((Events::TransmitProfile)profile);
    }

    void LogManager::SetTransmitProfile(String^ profileName)
    {
        if (!isInited)
        {
            startProfileName = profileName;
            return;
        }
        //LOG_TRACE("LogManager::SetTransmitProfile()");
        const std::string name = FromPlatformString(profileName);
        MAT::LogManager::SetTransmitProfile(name);
    }


    void LogManager::SetContext(String^ name, String^ value)
    {
        //LOG_TRACE("LogManager::SetContext[2]");
        MAT::LogManager::SetContext(FromPlatformString(name), FromPlatformString(value));
    }

    void LogManager::SetContext(String ^ name, String ^ value, PiiKind piiKind)
    {
        //LOG_TRACE("LogManager::SetContext[3]");
        MAT::LogManager::SetContext(FromPlatformString(name), FromPlatformString(value), (Events::PiiKind)piiKind);
    }

    ISemanticContext^ LogManager::SemanticContext::get()
    {
        //LOG_TRACE("LogManager::SemanticContext::get()");
        return platform_new SemanticContextImpl(MAT::LogManager::GetSemanticContext());
    }

    void LogManager::TestCallback::set(SendEventTestCallback^ callback)
    {
        if (m_eventCallaback != callback)
        {
            m_eventCallaback = callback;
        }
    }

    SendEventTestCallback^ LogManager::TestCallback::get()
    {
        return m_eventCallaback;
    }

    LogManager::LogManager()
    {
        reset();
    }

    LogManager::~LogManager()
    {
    }

    void LogManager::reset()
    {
        isInited = false;
        startProfileName = "";
        transmitProfiles = "";
    }

} MATW_NS_END
