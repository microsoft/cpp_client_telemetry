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
//#include "utils/Utils.hpp"
//#include "pal/PAL.hpp"

#ifdef _WINRT_DLL
#include "WindowsRTPlatformEventHandler.h"
#include "LogManagerEventReceiver.h"
#endif

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows {
                    const string OfflineStorageFileExtension = ".storage";

#if !((_MANAGED == 1) || (_M_CEE == 1))
                    /* static data members of managed types must be defined within the class definition */
                    bool LogManager::isInited = false;
#endif

                    void LogManager::checkup() {
                        if (!LogManager::isInited) {
                            //ARIASDK_LOG_DETAIL("LogManager is not initialized!");
                        }
                    }

#ifdef _WINRT_DLL
                    // This generates redefinition error in VS2010.
                    SendEventTestCallback^ LogManager::m_eventCallaback = nullptr;
                    PlatformEventHandler^ LogManager::m_platformEventHandler = nullptr;

                    ITestCallback* LogManager::m_testCallback = nullptr;
                    IPlatformEventReceiver* LogManager::m_eventReceiver = nullptr;
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
                        //ARIASDK_LOG_DETAIL("LogManager::Initialize token=%s, configuration=[...]",FromPlatformString(tenantToken).c_str() );
                        m_testCallback = new LogManagerTestCallback();

                        configuration->TenantToken = tenantToken;
                        auto token = FromPlatformString(tenantToken);

#ifdef _WINRT_DLL
                        //ARIASDK_LOG_DETAIL("running as _WINRT_DLL");	
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
                            Platform::String^ offlineFile = ToPlatformString(TokenToTenant(token) + OfflineStorageFileExtension);
                            Platform::String^ offlinePath = localFolder->Path->ToString() + "\\" + offlineFile;
                            // Print dir and file path
                            std::string path = FromPlatformString(offlinePath).c_str();
                            //ARIASDK_LOG_DETAIL("dir  = %s", FromPlatformString(localFolder->Path).c_str());
                            //ARIASDK_LOG_DETAIL("path = %s", FromPlatformString(offlinePath).c_str());
                            // Set configuration parameter
                            configuration->OfflineStorage = offlinePath;
                        }
#endif
                        isInited = true;
                        
                        configuration->ToLogConfigurationCore();
                        return platform_new Logger(MAT::LogManager::Initialize(token));
                    }

                    ILogger^ LogManager::GetLogger()
                    {
                        checkup();
                        //ARIASDK_LOG_DETAIL("LogManager::GetLogger()");
                        return platform_new Logger(MAT::LogManager::GetLogger());

                    }

                    ILogger^ LogManager::GetLogger(String^ tenantToken, String^ source)
                    {
                        checkup();
                        //ARIASDK_LOG_DETAIL("LogManager::GetLogger[2]");
                        return platform_new Logger(MAT::LogManager::GetLogger(FromPlatformString(tenantToken), FromPlatformString(source)));
                    }

                    ILogger^ LogManager::GetLogger(String^ source)
                    {
                        checkup();
                        //ARIASDK_LOG_DETAIL("LogManager::GetLogger[1]");
                        return platform_new Logger(MAT::LogManager::GetLogger(FromPlatformString(source)));
                    }

                    void LogManager::PauseTransmission()
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::PauseTransmission()");
                        MAT::LogManager::PauseTransmission();
                    }

                    void LogManager::Flush()
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::Flush()");
                        MAT::LogManager::Flush();
                    }

                    void LogManager::FlushAndTeardown()
                    {
                        MAT::LogManager::FlushAndTeardown();
                        isInited = false;
                    }

                    void LogManager::UploadNow()
                    {
                        MAT::LogManager::UploadNow();
                    }

                    void LogManager::ResumeTransmission()
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::ResumeTransmission()");
                        MAT::LogManager::ResumeTransmission();
                    }

                    void LogManager::SetTransmitProfile(TransmitProfile profile)
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::SetTransmitProfile()");
                        MAT::LogManager::SetTransmitProfile((Telemetry::TransmitProfile)profile);
                    }

                    void LogManager::SetTransmitProfile(String^ profileName)
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::SetTransmitProfile()");
                        const std::string name = FromPlatformString(profileName);
                        MAT::LogManager::SetTransmitProfile(name);
                    }


                    void LogManager::SetContext(String^ name, String^ value)
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::SetContext[2]");
                        MAT::LogManager::SetContext(FromPlatformString(name), FromPlatformString(value));
                    }

                    void LogManager::SetContext(String ^ name, String ^ value, PiiKind piiKind)
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::SetContext[3]");
                        MAT::LogManager::SetContext(FromPlatformString(name), FromPlatformString(value), (Telemetry::PiiKind)piiKind);
                    }

                    ISemanticContext^ LogManager::SemanticContext::get()
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::SemanticContext::get()");
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
                        isInited = false;
                        //ARIASDK_LOG_DETAIL("LogManager::LogManager()");
                    }

                    LogManager::~LogManager()
                    {
                        //ARIASDK_LOG_DETAIL("LogManager::~LogManager()");
                        isInited = false;
                    }
                }
            }
    }
}