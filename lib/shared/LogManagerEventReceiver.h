#pragma once
#include "IPlatformEventReceiver.h"
#include "LogConfigurationCX.hpp"
#include "PlatformHelpers.h"

namespace Microsoft {
    namespace Applications {
        namespace Events  {
            namespace Windows
            {
                class LogManagerEventReceiver : public IPlatformEventReceiver
                {
                public:
                    /// <summary>IPlatformEventReceiver member. Saves unsend events into the offline storage and stops threads.
                    /// It is save to exit the application after this method is called.</summary>
                    virtual void Suspend()
                    {
                        if (m_configuration->AutoLogAppSuspend)
                        {
//                            MAT::LogManager::GetLogger(FromPlatformString(m_configuration->TenantToken))->LogAppLifecycle((Events ::AppLifecycleState)AppLifeCycleState::Suspend,
//                                Events ::EventProperties(""));
                        }

                        // We may need tear-down to avoid AUF initialization issues.
                        MAT::LogManager::Flush();
                    }

                    /// <summary>IPlatformEventReceiver member. Re-initializes the telemetry.</summary>
                    virtual void Resume()
                    {
                        //  We only need to initialize if we do FlushAndTeardown.
                        //Events ::LogManager::Initialize(FromPlatformString(m_configuration->TenantToken),
                        //    m_configuration->ToLogConfigurationCore());

                        if (m_configuration->AutoLogAppResume)
                        {
//                            MAT::LogManager::GetLogger(FromPlatformString(m_configuration->TenantToken))->LogAppLifecycle((Events ::AppLifecycleState)AppLifeCycleState::Resume,
//                                Events ::EventProperties(""));
                        }
                    }

                    /// <summary>IPlatformEventReceiver member. Logs the failure event.</summary>
                    virtual void UnhandledException(int code, const std::string& message)
                    {
                        if (m_configuration->AutoLogUnhandledException)
                        {
                            MAT::LogManager::GetLogger(FromPlatformString(m_configuration->TenantToken))->LogEvent("Exception code: " + std::to_string((long long)code) + message +
                                "UnhandledException");
                        }

                        // Attempting to save all unsent events into the offline storage.
                        MAT::LogManager::FlushAndTeardown();
                    }

                    LogManagerEventReceiver(LogConfiguration^ configuration) : m_configuration(configuration)
                    {
                    }

                private:
                    LogConfiguration^ m_configuration;
                };
            }
        }
    }
}

