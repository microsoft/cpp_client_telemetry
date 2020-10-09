//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "PlatformHelpers.h"
#include "SchemaStub.hpp"
#include "LoggerCX.hpp"

namespace Microsoft {
    namespace Applications 
    {
        class IPlatformEventReceiver;
        namespace Telemetry {
            namespace Windows
            {
                class WindowsEnvironmentInfo;
                
                interface class ILogger;
                ref class PlatformEventHandler;
                ref struct LogConfiguration;

                public delegate void SendEventTestCallback(EventProperties^ telemetryevent);

                public ref class LogManager sealed
                {
                public:
                    static ILogger^ Initialize(String^ tenantToken);
                    static ILogger^ Initialize(String^ tenantToken, LogConfiguration^ configuration);
                    static ILogger^ GetLogger();
                    static ILogger^ GetLogger(String^ tenantToken, String^ source);
                    static ILogger^ GetLogger(String^ source);

                    static void SetContext(String^ name, String^ value);
                    static void SetContext(String^ name, String^ value, PiiKind piiKind);

                    static property ISemanticContext^ SemanticContext
                    {
                        static ISemanticContext^ get();
                    }

                    static void ResumeTransmission();
                    static void PauseTransmission();
                    static void Flush();
                    static void FlushAndTeardown();
                    static void UploadNow();

                    static bool LoadTransmitProfiles(String^ json);
                    static void SetTransmitProfile(TransmitProfile profile);
#ifdef WIN10_CS
                    [::Windows::Foundation::Metadata::DefaultOverloadAttribute]
#endif
                    static void SetTransmitProfile(String ^ profileName);

                    static property SendEventTestCallback^ TestCallback
                    {
                        static SendEventTestCallback^ get();
                        static void set(SendEventTestCallback^ callback);
                    }

                private:
                    
                    static void reset();

                    LogManager();
                    ~LogManager();

                    static bool isInited;

                    static String^ startProfileName;
                    static String^ transmitProfiles;

                    static SendEventTestCallback^ m_eventCallaback;
                    static ITestCallback* m_testCallback;

#ifdef _WINRT_DLL
                    static IPlatformEventReceiver* m_eventReceiver;
                    static PlatformEventHandler^ m_platformEventHandler;
#endif
                };
            }
        }
    }
}
