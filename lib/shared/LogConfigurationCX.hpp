//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#pragma once

#include "LogManager.hpp"
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"
#include <LogManager.hpp>

namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            namespace Windows
            {

                public enum class ACTTraceLevel {
                    /// <summary>Debug messages.</summary>
                    ACTTraceLevel_Debug = MAT::ACTTraceLevel::ACTTraceLevel_Debug,
                    /// <summary>Trace messages.</summary>
                    ACTTraceLevel_Trace = MAT::ACTTraceLevel::ACTTraceLevel_Trace,
                    /// <summary>Informational messages.</summary>
                    ACTTraceLevel_Info = MAT::ACTTraceLevel::ACTTraceLevel_Info,
                    /// <summary>Warnings.</summary>
                    ACTTraceLevel_Warn = MAT::ACTTraceLevel::ACTTraceLevel_Warn,
                    /// <summary>Errors.</summary>
                    ACTTraceLevel_Error = MAT::ACTTraceLevel::ACTTraceLevel_Error,
                    /// <summary>Fatal errors that lead to process termination.</summary>
                    ACTTraceLevel_Fatal = MAT::ACTTraceLevel::ACTTraceLevel_Fatal
                };

                public enum class SdkModeTypes {
                    /// <summary>The default transmission mode.</summary>
                    SdkModeTypes_CS = MAT::SdkModeTypes::SdkModeTypes_CS,
                    /// <summary>Backward compatibility transmission mode.</summary>
                    SdkModeTypes_UTCBackCompat = MAT::SdkModeTypes::SdkModeTypes_UTCBackCompat,
                    /// <summary>Common schema transmission mode.</summary>
                    SdkModeTypes_UTCCommonSchema = MAT::SdkModeTypes::SdkModeTypes_UTCCommonSchema
                };

                public ref struct LogConfiguration sealed
                {
                public:
#ifndef _WINRT_DLL  /* C# .NET implementation */
                    static String^ CollectorUrlDefault      = L"https://mobile.events.data.microsoft.com/OneCollector/1.0/";
                    static String^ CollectorUrlUnitedStates = L"https://us-mobile.events.data.microsoft.com/OneCollector/1.0/";
                    static String^ CollectorUrlGermany      = L"https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/";
                    static String^ CollectorUrlAustralia    = L"https://au-mobile.events.data.microsoft.com/OneCollector/1.0/";
                    static String^ CollectorUrlJapan        = L"https://jp-mobile.events.data.microsoft.com/OneCollector/1.0/";
                    static String^ CollectorUrlEurope       = L"https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/";
#else               /* WinRT .winmd linkage implementation*/
                    static property String^ CollectorUrlDefault      { String ^get() { return L"https://mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
                    static property String^ CollectorUrlUnitedStates { String ^get() { return L"https://us-mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
                    static property String^ CollectorUrlGermany      { String ^get() { return L"https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
                    static property String^ CollectorUrlAustralia    { String ^get() { return L"https://au-mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
                    static property String^ CollectorUrlJapan        { String ^get() { return L"https://jp-mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
                    static property String^ CollectorUrlEurope       { String ^get() { return L"https://eu-mobile.events.data.microsoft.com/OneCollector/1.0/"; } }
#endif
                    LogConfiguration()
                    {
                        // Default configuration options.
                        AutoLogAppSuspend = true;
                        AutoLogAppResume = true;
                        AutoLogUnhandledException = true;
                        SdkMode = SdkModeTypes::SdkModeTypes_CS;
                        MaxTeardownUploadTimeInSec = 0;
                        MaxPendingHTTPRequests = 16;
                        MaxDBFlushQueues = 3;
                        CollectorURL = CollectorUrlDefault;
                        StartProfileName = "";
                        TransmitProfiles = "";
                        CacheFileSizeLimitInBytes = 3 * 1024 * 1024;
                        EnableDBDropIfFull = false;
                    }

                    property String^ CollectorURL;
                    property bool AutoLogAppSuspend;
                    property bool AutoLogAppResume;
                    property bool AutoLogUnhandledException;
                    property String^ OfflineStorage;
                    property unsigned int CacheFileSizeLimitInBytes;
                    property bool EnableDBDropIfFull;
                    property SdkModeTypes SdkMode;
                    property unsigned int TraceLevelMask;
                    property ACTTraceLevel MinTraceLevel;
                    property unsigned int MaxTeardownUploadTimeInSec;
                    property unsigned int MaxPendingHTTPRequests;
                    property unsigned int MaxDBFlushQueues;
                    property String^ TransmitProfiles;
                    property String^ StartProfileName;

                internal:
                    void ToLogConfigurationCore();

                    property String^ TenantToken;

                };
            }
        }
    }
}

