//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once
#include "IPlatformEventReceiver.h"
#include "LogConfigurationCX.hpp"
#include "PlatformHelpers.h"

namespace Microsoft {
    namespace Applications {
        namespace Telemetry  {
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
                            MAT::LogManager::GetLogger()->LogAppLifecycle((MAT::AppLifecycleState)AppLifeCycleState::Suspend,
                                MAT::EventProperties(""));
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
                            MAT::LogManager::GetLogger()->LogAppLifecycle((MAT::AppLifecycleState)AppLifeCycleState::Resume,
                                MAT::EventProperties(""));
                        }
                    }

                    /// <summary>IPlatformEventReceiver member. Logs the failure event.</summary>
                    virtual void UnhandledException(int code, const std::string& message)
                    {
                        if (m_configuration->AutoLogUnhandledException)
                        {
                            MAT::LogManager::GetLogger()->LogFailure("Exception code: " + std::to_string((long long)code), message,
                                "UnhandledException", "", MAT::EventProperties(""));
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


