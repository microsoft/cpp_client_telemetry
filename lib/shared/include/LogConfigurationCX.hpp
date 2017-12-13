#pragma once

#include "LogManager.hpp"
#include "PlatformHelpers.h"
#include "SchemaStub.hpp"

namespace MAT = Microsoft::Applications::Telemetry;

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
                    ACTTraceLevel_Info  = MAT::ACTTraceLevel::ACTTraceLevel_Info,
                    /// <summary>Warnings.</summary>
                    ACTTraceLevel_Warn  = MAT::ACTTraceLevel::ACTTraceLevel_Warn,
                    /// <summary>Errors.</summary>
                    ACTTraceLevel_Error = MAT::ACTTraceLevel::ACTTraceLevel_Error,
                    /// <summary>Fatal errors that lead to process termination.</summary>
                    ACTTraceLevel_Fatal = MAT::ACTTraceLevel::ACTTraceLevel_Fatal
                };

				public enum class SdkModeTypes {
                    /// <summary>The default transmission mode.</summary>
					SdkModeTypes_Aria              =  MAT::SdkModeTypes::SdkModeTypes_Aria,
                    /// <summary>Backward compatibility transmission mode.</summary>
					SdkModeTypes_UTCAriaBackCompat = MAT::SdkModeTypes::SdkModeTypes_UTCAriaBackCompat,
                    /// <summary>Common schema transmission mode.</summary>
					SdkModeTypes_UTCCommonSchema   = MAT::SdkModeTypes::SdkModeTypes_UTCCommonSchema
				};

                public ref struct LogConfiguration sealed
                {
                public:					
#ifndef _WINRT_DLL  /* C# .NET implementation */
                   static String^ CollectorUrlUnitedStates  = L"https://us.pipe.aria.microsoft.com/Collector/3.0/";
                   static String^ CollectorUrlGermany       = L"https://de.pipe.aria.microsoft.com/Collector/3.0/";
                   static String^ CollectorUrlAustralia     = L"https://au.pipe.aria.microsoft.com/Collector/3.0/";
                   static String^ CollectorUrlJapan         = L"https://jp.pipe.aria.microsoft.com/Collector/3.0/";
                   static String^ CollectorUrlEurope        = L"https://eu.pipe.aria.microsoft.com/Collector/3.0/";
#else               /* WinRT .winmd linkage implementation*/
                    static property String^ CollectorUrlUnitedStates { String ^get() { return L"https://us.pipe.aria.microsoft.com/Collector/3.0/"; } }
                    static property String^ CollectorUrlGermany      { String ^get() { return L"https://de.pipe.aria.microsoft.com/Collector/3.0/"; } }
                    static property String^ CollectorUrlAustralia    { String ^get() { return L"https://au.pipe.aria.microsoft.com/Collector/3.0/"; } }
                    static property String^ CollectorUrlJapan        { String ^get() { return L"https://jp.pipe.aria.microsoft.com/Collector/3.0/"; } }
                    static property String^ CollectorUrlEurope       { String ^get() { return L"https://eu.pipe.aria.microsoft.com/Collector/3.0/"; } }
#endif
                    LogConfiguration()
                    {
                        // Default configuration options.
                        AutoLogAppSuspend = true;
                        AutoLogAppResume = true;
                        AutoLogUnhandledException = true;
                        SdkMode = SdkModeTypes::SdkModeTypes_Aria;
                        MaxTeardownUploadTimeInSec = 0;
                        MaxPendingHTTPRequests = 16;
                        MaxDBFlushQueues = 3;
                    }

                    property String^ CollectorURL;                   
                    property bool AutoLogAppSuspend;
                    property bool AutoLogAppResume;
                    property bool AutoLogUnhandledException;
                    property String^ OfflineStorage;
                    property SdkModeTypes SdkMode;
                    property unsigned int TraceLevelMask;
                    property ACTTraceLevel MinTraceLevel;
                    property unsigned int MaxTeardownUploadTimeInSec;
                    property unsigned int MaxPendingHTTPRequests;
                    property unsigned int MaxDBFlushQueues;

                internal:
                    void ToLogConfigurationCore();

                    property String^ TenantToken;
					
                };
            }
        }
    }
}
