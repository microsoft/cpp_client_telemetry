
#ifndef ARIA_ENUMS_HPP
#define ARIA_ENUMS_HPP

//#include "Version.hpp"

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*

    enum SdkModeTypes
    {
        SdkModeTypes_Aria = 0, //This is default transmission mode
        SdkModeTypes_UTCAriaBackCompat = 1,
        SdkModeTypes_UTCCommonSchema = 2
    };

/// <summary>
/// Internal SDK debugging trace level
/// </summary>
    enum ACTTraceLevel {
        /// <summary>Debug messages</summary>
        ACTTraceLevel_Debug,
        /// <summary>Trace messages</summary>
        ACTTraceLevel_Trace,
        /// <summary>Informational messages</summary>
        ACTTraceLevel_Info,
        /// <summary>Warnings</summary>
        ACTTraceLevel_Warn,
        /// <summary>Errors</summary>
        ACTTraceLevel_Error,
        /// <summary>Fatal errors that lead to process termination</summary>
        ACTTraceLevel_Fatal
    };

/// <summary>
/// PII (Personal Identifiable Information) kind used to indicate an event property value
/// </summary>
            enum PiiKind
            {
                PiiKind_None                = 0,
                PiiKind_DistinguishedName   = 1,
                PiiKind_GenericData         = 2,
                PiiKind_IPv4Address         = 3,
                PiiKind_IPv6Address         = 4,
                PiiKind_MailSubject         = 5,
                PiiKind_PhoneNumber         = 6,
                PiiKind_QueryString         = 7,
                PiiKind_SipAddress          = 8,
                PiiKind_SmtpAddress         = 9,
                PiiKind_Identity            = 10,
                PiiKind_Uri                 = 11,
                PiiKind_Fqdn                = 12,
                PiiKind_IPv4AddressLegacy   = 13
            };


/// <summary>
/// Customer specific contenat kind used to indicate an event property value
/// </summary>
            enum CustomerContentKind {
                CustomerContentKind_None = 0,
                CustomerContentKind_GenericData = 1,
            };

/// <summary>
/// Api Types of an operation such as ServiceApi or ClientProxy
/// </summary.
            enum ApiType
            {
                ApiType_None                = 0,
                ApiType_ServiceApi          = 1,
                ApiType_ClientProxy         = 2
            };

/// <summary>
/// AggregateType indicates the type of aggregated metric being reported
/// </summary>
            enum AggregateType
            {
                AggregateType_Sum           = 0,
                AggregateType_Maximum       = 1,
                AggregateType_Minimum       = 2,
                AggregateType_SumOfSquares  = 3     //Sum of squares (to calculate variance)
            };

/// <summary>
/// AppLifeCycle identifies the current app state
/// </summary>
            enum AppLifecycleState
            {
                AppLifecycleState_Unknown   = 0,
                AppLifecycleState_Launch    = 1,
                AppLifecycleState_Exit      = 2,
                AppLifecycleState_Suspend   = 3,
                AppLifecycleState_Resume    = 4,
                AppLifecycleState_Foreground = 5,
                AppLifecycleState_Background = 6
            };

/// <summary>
/// User's state to report (final values TBD)
/// </summary>
            enum SessionState
            {
                /// Unknown
                        Session_Started = 0,
                /// Connected: user is connected to a service
                        Session_Ended = 1,
            };

/// <summary>
/// Type of actions that could occur on a page view
/// These are general abstraction of action types, each of which could correspond
/// to multiple raw action types. For eg. a Click action type could be resulted
/// either from a ButtonDown or from a TouchTap.
/// </summary>
            enum ActionType
            {
                ActionType_Unspecified      = 0,
                ActionType_Unknown          = 1,
                ActionType_Other            = 2,
                ActionType_Click            = 3,
                ActionType_Pan              = 5,
                ActionType_Zoom             = 6,
                ActionType_Hover            = 7
            };

/// <summary>
/// enum corresponds to a physical action that user has performed on a page view
/// </summary>
            enum RawActionType
            {
                RawActionType_Unspecified           = 0,
                RawActionType_Unknown               = 1,
                RawActionType_Other                 = 2,
                RawActionType_LButtonDoubleClick    = 11,
                RawActionType_LButtonDown           = 12,
                RawActionType_LButtonUp             = 13,
                RawActionType_MButtonDoubleClick    = 14,
                RawActionType_MButtonDown           = 15,
                RawActionType_MButtonUp             = 16,
                RawActionType_MouseHover            = 17,
                RawActionType_MouseWheel            = 18,
                RawActionType_MouseMove             = 20,
                RawActionType_RButtonDoubleClick    = 22,
                RawActionType_RButtonDown           = 23,
                RawActionType_RButtonUp             = 24,
                RawActionType_TouchTap              = 50,
                RawActionType_TouchDoubleTap        = 51,
                RawActionType_TouchLongPress        = 52,
                RawActionType_TouchScroll           = 53,
                RawActionType_TouchPan              = 54,
                RawActionType_TouchFlick            = 55,
                RawActionType_TouchPinch            = 56,
                RawActionType_TouchZoom             = 57,
                RawActionType_TouchRotate           = 58,
                RawActionType_KeyboardPress         = 100,
                RawActionType_KeyboardEnter         = 101
            };

/// <summary>
/// enum corresponds to a physical device that user used to perform action on a page view
/// </summary>
            enum InputDeviceType
            {
                InputDeviceType_Unspecified = 0,
                InputDeviceType_Unknown     = 1,
                InputDeviceType_Other       = 2,
                InputDeviceType_Mouse       = 3,
                InputDeviceType_Keyboard    = 4,
                InputDeviceType_Touch       = 5,
                InputDeviceType_Stylus      = 6,
                InputDeviceType_Microphone  = 7,
                InputDeviceType_Kinect      = 8,
                InputDeviceType_Camera      = 9
            };

/// <summary>
/// Levels of trace events that represents printf style logging generated by an application
/// </summary>
            enum TraceLevel
            {
                TraceLevel_None             = 0,
                TraceLevel_Error            = 1,
                TraceLevel_Warning          = 2,
                TraceLevel_Information      = 3,
                TraceLevel_Verbose          = 4
            };

/// <summary>
/// User's state to report (final values TBD)
/// </summary>
            enum UserState
            {
                /// Unknown
                        UserState_Unknown           = 0,
                /// Connected: user is connected to a service
                        UserState_Connected         = 1,
                /// Reachable: user is reachable for a service like push notification
                        UserState_Reachable         = 2,
                /// SignedIn: user is signed in
                        UserState_SignedIn          = 3,
                /// SignedOut: user is signed out
                        UserState_SignedOut         = 4
            };


/// <summary>
/// The OS achitecture type, such as "x86" or "x64". Note that it will be X86 for 32-bit OS even if the processor architecture is x64.
/// </summary>
            enum OsArchitectureType
            {
                // Unknown or not available
                        OsArchitectureType_Unknown  = 0,

                // OS architecture in 32-bit (x86) mode
                        OsArchitectureType_X86      = 1,

                // OS architecture in 64-bit (x64) mode
                        OsArchitectureType_X64      = 2,

                // OS architecture for ARM-family
                        OsArchitectureType_Arm      = 3
            };

            enum PowerSource
            {
                PowerSource_Any = -1,
                PowerSource_Unknown = 0,
                PowerSource_Battery = 1,
                PowerSource_Charging = 2,
                PowerSource_LowBattery = 3 /* Reserved for future use */
            };

/// <summary>
/// The Network cost values for a device.
/// </summary>
            enum NetworkCost
            {
                NetworkCost_Any = -1,
                NetworkCost_Unknown = 0,
                NetworkCost_Unmetered = 1,
                NetworkCost_Metered = 2,
                NetworkCost_Roaming = 3,
                NetworkCost_OverDataLimit = 3 /* [deprecated]: OverDataLimit should not be used */
            };

/// <summary>
/// The Network type values that a device is connected to.
/// </summary>
            enum NetworkType
            {
                NetworkType_Any = -1,
                NetworkType_Unknown = 0,
                NetworkType_Wired = 1,
                NetworkType_Wifi = 2,
                NetworkType_WWAN = 3
            };

/// <summary>
/// Priority for an event to be transmitted
/// </summary>
            enum EventPriority
            {
                /// Unspecified: Event priority is not specified
                        EventPriority_Unspecified   = -1,

                /// Off: Event is not to be transmitted
                        EventPriority_Off           = 0,

                /// Low: Event is to be transmitted at low priority
                        EventPriority_Low           = 1,
                        EventPriority_MIN           = EventPriority_Low,

                /// Normal: Event is to be transmitted at normal priority
                        EventPriority_Normal        = 2,

                /// High: Event is to be transmitted at high priority
                        EventPriority_High          = 3,

                /// Immediate: Event is to be transmitted as soon as possible
                        EventPriority_Immediate     = 4,
                        EventPriority_MAX           = EventPriority_Immediate
            };

/// <summary>
/// HTTP request result codes
/// </summary>
            enum HttpResult
            {
                /// Response has been received successfully, HTTP status code is
                // available as returned by the target server.
                        HttpResult_OK             = 0,

                /// Request has been aborted by the caller. The server might or
                /// might not have already received and processed the request.
                        HttpResult_Aborted        = 1,

                /// Local conditions have prevented the request from being sent
                /// (invalid request parameters, out of memory, internal error etc.).
                        HttpResult_LocalFailure   = 2,

                /// Network conditions somewhere between the local machine and
                /// the target server have caused the request to fail
                /// (connection failed, connection dropped abruptly etc.).
                        HttpResult_NetworkFailure = 3
            };

/// <summary>
/// Transmit profiles to choose from for event transmission that could favor low transmission
/// latency or device resource consumption.
/// </summary>
            enum TransmitProfile
            {
                /// <summary>Favors low transmission latency, but may consume more data bandwidth and power.</summary>
                        TransmitProfile_RealTime = 0,
                /// <summary>Favors near real-time transmission latency. Automatically balances transmission
                /// latency with data bandwidth and power consumption.</summary>
                        TransmitProfile_NearRealTime = 1,
                /// <summary>Favors device performance by conserving both data bandwidth and power consumption.</summary>
                        TransmitProfile_BestEffort = 2
            };

        /// <summary>Event rejected due to legit reasoning</summary>
            enum EventRejectedReason
            {
                /// <summary>Validation failed.</summary>
                REJECTED_REASON_VALIDATION_FAILED,
                /// <summary>Old record version.</summary>
                REJECTED_REASON_OLD_RECORD_VERSION,
                /// <summary>Invalid client message type.</summary>
                REJECTED_REASON_INVALID_CLIENT_MESSAGE_TYPE,
                /// <summary>Required argument missing.</summary>
                REJECTED_REASON_REQUIRED_ARGUMENT_MISSING,
                /// <summary>Event name missing.</summary>
                REJECTED_REASON_EVENT_NAME_MISSING,
                /// <summary>Event size limit exceeded.</summary>
                REJECTED_REASON_EVENT_SIZE_LIMIT_EXCEEDED,
                /// <summary>Event banned.</summary>
                REJECTED_REASON_EVENT_BANNED,
                /// <summary>Event expired.</summary>
                REJECTED_REASON_EVENT_EXPIRED,
                /// <summary>Server declined.</summary>
                REJECTED_REASON_SERVER_DECLINED_4XX,
                /// <summary>Reject reason count.</summary>
                REJECTED_REASON_COUNT
            };
            
            const static unsigned gc_NumRejectedReasons = REJECTED_REASON_COUNT;


        }}} // namespace Microsoft::Applications::Telemetry

#endif //EVENTPRIORITY_H