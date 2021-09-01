//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef MAT_ENUMS_HPP
#define MAT_ENUMS_HPP

#include "ctmacros.hpp"

#include <string>
#include <vector>
#include <map>

#ifndef _WIN32
#include <sys/errno.h>
#endif

namespace MAT_NS_BEGIN
{

    /// <summary>
    /// The status_t enumeration contains a set of status code.
    /// </summary>
    enum status_t
    {
        /// <summary>General failure</summary>
        STATUS_EFAIL = -1,

        /// <summary>Success.</summary>
        STATUS_SUCCESS = 0,

        /// <summary>Permission denied</summary>
        STATUS_EPERM = EPERM,

        /// <summary>Already done / already in progress</summary>
        STATUS_EALREADY = EALREADY,

        /// <summary>Not implemented or no-op</summary>
        STATUS_ENOSYS = ENOSYS,

        /// <summary>Not supported.</summary>
        STATUS_ENOTSUP = ENOTSUP
    };

    enum DataCategory
    {
        DataCategory_PartC = 0, //This is default transmission mode
        DataCategory_PartB = 1,
        DataCategory_MAX = 2
    };


    enum TicketType
    {
        TicketType_MSA_Device = 0,
        TicketType_MSA_User = 1,
        TicketType_XAuth_Device = 2,
        TicketType_XAuth_User = 3,
        TicketType_AAD = 4,
        TicketType_AAD_User = 5,
        TicketType_AAD_JWT = 6,
    };

    /// <summary>
    /// The SdkModeTypes enumeration contains a set of values that specify transmission modes.
    /// </summary>
    enum SdkModeTypes
    {
        /// <summary>The default transmission mode.</summary>
        SdkModeTypes_CS = 0,
        /// <summary>Backward compatibility transmission mode.</summary>
        SdkModeTypes_UTCBackCompat = 1,
        /// <summary>Common schema transmission mode.</summary>
        SdkModeTypes_UTCCommonSchema = 2,
        /// <summary>Transmission to AppInsights endpoint.</summary>
        SdkModeTypes_AI = 3
    };

    /// <summary>
    /// The ACTTraceLevel enumeration contains a set of values that specify internal SDK debugging trace levels.
    /// </summary>
    enum ACTTraceLevel
    {
        /// <summary>Debug messages.</summary>
        ACTTraceLevel_Debug,
        /// <summary>Trace messages.</summary>
        ACTTraceLevel_Trace,
        /// <summary>Informational messages.</summary>
        ACTTraceLevel_Info,
        /// <summary>Warnings.</summary>
        ACTTraceLevel_Warn,
        /// <summary>Errors.</summary>
        ACTTraceLevel_Error,
        /// <summary>Fatal errors that lead to process termination.</summary>
        ACTTraceLevel_Fatal
    };

    /// <summary>
    /// The PiiKind enumeration contains a set of values that specify the kind of PII (Personal Identifiable Information) for tagging an event property.
    /// </summary>
    enum PiiKind
    {
        /// <summary>No PII kind.</summary>
        PiiKind_None = 0,
        /// <summary>An LDAP distinguished name.</summary>
        PiiKind_DistinguishedName = 1,
        /// <summary>Generic data.</summary>
        PiiKind_GenericData = 2,
        /// <summary>An IPV4 Internet address.</summary>
        PiiKind_IPv4Address = 3,
        /// <summary>An IPV6 Internet address.</summary>
        PiiKind_IPv6Address = 4,
        /// <summary>An e-mail subject.</summary>
        PiiKind_MailSubject = 5,
        /// <summary>A telephone number.</summary>
        PiiKind_PhoneNumber = 6,
        /// <summary>A query string.</summary>
        PiiKind_QueryString = 7,
        /// <summary>A SIP address</summary>
        PiiKind_SipAddress = 8,
        /// <summary>An e-mail address.</summary>
        PiiKind_SmtpAddress = 9,
        /// <summary>An identity.</summary>
        PiiKind_Identity = 10,
        /// <summary>A uniform resource indicator.</summary>
        PiiKind_Uri = 11,
        /// <summary>A fully-qualified domain name.</summary>
        PiiKind_Fqdn = 12,
        /// <summary>A legacy IPV4 Internet address.</summary>
        PiiKind_IPv4AddressLegacy = 13,

        CustomerContentKind_GenericData = 32
    };

    /// <summary>
    /// The ApiType enumeration contains a set of values that specify the type of API&mdash;such as a service API or a client proxy API.
    /// </summary>
    enum ApiType
    {
        /// <summary>No API type.</summary>
        ApiType_None = 0,
        /// <summary>Service API.</summary>
        ApiType_ServiceApi = 1,
        /// <summary>Client proxy API.</summary>
        ApiType_ClientProxy = 2
    };

    /// <summary>
    /// The AggregateType enumeration contains a set of values that specify the type of aggregated metric.
    /// </summary>
    enum AggregateType
    {
        /// <summary>The arithmetic sum.</summary>
        AggregateType_Sum = 0,
        /// <summary>The maximum.</summary>
        AggregateType_Maximum = 1,
        /// <summary>The minimum.</summary>
        AggregateType_Minimum = 2,
        /// <summary>The sum of the squares used to calculate the variance.</summary>
        AggregateType_SumOfSquares = 3
    };

    /// <summary>
    /// The AppLifecycleState enumeration contains a set of values that specify the lifecycle state of the application.
    /// </summary>
    enum AppLifecycleState
    {
        /// <summary>Lifecycle state unknown.</summary>
        AppLifecycleState_Unknown = 0,
        /// <summary>The application launched.</summary>
        AppLifecycleState_Launch = 1,
        /// <summary>The application exited.</summary>
        AppLifecycleState_Exit = 2,
        /// <summary>The application suspended.</summary>
        AppLifecycleState_Suspend = 3,
        /// <summary>The application resumed.</summary>
        AppLifecycleState_Resume = 4,
        /// <summary>The application came back into the foreground.</summary>
        AppLifecycleState_Foreground = 5,
        /// <summary>The application went into the background.</summary>
        AppLifecycleState_Background = 6
    };

    /// <summary>
    /// The SessionState enumeration contains a set of values that specify the user's session state.
    /// </summary>
    enum SessionState
    {
        /// <summary>The user's session started.</summary>
        Session_Started = 0,
        /// <summary>The user's session ended.</summary>
        Session_Ended = 1,
    };

    /// <summary>
    /// The ActionType enumeration contains a set of values that specify the
    /// type of action that a user can perform on a page view. 
    /// They are a general abstraction of action types, each of which corresponds
    /// to multiple raw action types. For example, a click action type can be the result of 
    /// either a button down or a touch tap.
    /// </summary>
    enum ActionType
    {
        /// <summary>The action type is unspecified.</summary>
        ActionType_Unspecified = 0,
        /// <summary>The action type is unknown.</summary>
        ActionType_Unknown = 1,
        /// <summary>The action type is other.</summary>
        ActionType_Other = 2,
        /// <summary>A mouse click.</summary>
        ActionType_Click = 3,
        /// <summary>A pan.</summary>
        ActionType_Pan = 5,
        /// <summary>A zoom.</summary>
        ActionType_Zoom = 6,
        /// <summary>A hover.</summary>
        ActionType_Hover = 7
    };

    /// <summary>
    /// The RawActionType enumeration contains a set of values that specify the type of physical action that users can perform on a page view.
    /// </summary>
    enum RawActionType
    {
        /// <summary>Raw action type unspecified.</summary>
        RawActionType_Unspecified = 0,
        /// <summary>Raw action type unknown.</summary>
        RawActionType_Unknown = 1,
        /// <summary>Raw action type other.</summary>
        RawActionType_Other = 2,
        /// <summary>Left button double-click.</summary>
        RawActionType_LButtonDoubleClick = 11,
        /// <summary>Left button down.</summary>
        RawActionType_LButtonDown = 12,
        /// <summary>Left button up.</summary>
        RawActionType_LButtonUp = 13,
        /// <summary>Middle button double-click.</summary>
        RawActionType_MButtonDoubleClick = 14,
        /// <summary>Middle button down.</summary>
        RawActionType_MButtonDown = 15,
        /// <summary>Middle button up.</summary>
        RawActionType_MButtonUp = 16,
        /// <summary>Mouse hover.</summary>
        RawActionType_MouseHover = 17,
        /// <summary>Mouse wheel.</summary>
        RawActionType_MouseWheel = 18,
        /// <summary>Mouse move.</summary>
        RawActionType_MouseMove = 20,
        /// <summary>Right button double-click.</summary>
        RawActionType_RButtonDoubleClick = 22,
        /// <summary>Right button down.</summary>
        RawActionType_RButtonDown = 23,
        /// <summary>Right button up.</summary>
        RawActionType_RButtonUp = 24,
        /// <summary>Touch tap.</summary>
        RawActionType_TouchTap = 50,
        /// <summary>Touch double-tap.</summary>
        RawActionType_TouchDoubleTap = 51,
        /// <summary>Touch long-press.</summary>
        RawActionType_TouchLongPress = 52,
        /// <summary>Touch scroll.</summary>
        RawActionType_TouchScroll = 53,
        /// <summary>Touch pan.</summary>
        RawActionType_TouchPan = 54,
        /// <summary>Touch flick.</summary>
        RawActionType_TouchFlick = 55,
        /// <summary>Touch pinch.</summary>
        RawActionType_TouchPinch = 56,
        /// <summary>Touch zoom.</summary>
        RawActionType_TouchZoom = 57,
        /// <summary>Touch rotate.</summary>
        RawActionType_TouchRotate = 58,
        /// <summary>Keyboard press.</summary>
        RawActionType_KeyboardPress = 100,
        /// <summary>Keyboard Enter.</summary>
        RawActionType_KeyboardEnter = 101
    };

    /// <summary>
    /// The InputDeviceType enumeration contains a set of values that specify a physical device that a user can use to perform an action on a page view.
    /// </summary>
    enum InputDeviceType
    {
        /// <summary>Device type unspecified.</summary>
        InputDeviceType_Unspecified = 0,
        /// <summary>Device type unknown.</summary>
        InputDeviceType_Unknown = 1,
        /// <summary>Other.</summary>
        InputDeviceType_Other = 2,
        /// <summary>Mouse.</summary>
        InputDeviceType_Mouse = 3,
        /// <summary>Keyboard.</summary>
        InputDeviceType_Keyboard = 4,
        /// <summary>Touch.</summary>
        InputDeviceType_Touch = 5,
        /// <summary>Stylus.</summary>
        InputDeviceType_Stylus = 6,
        /// <summary>Microphone.</summary>
        InputDeviceType_Microphone = 7,
        /// <summary>Kinect.</summary>
        InputDeviceType_Kinect = 8,
        /// <summary>Camera.</summary>
        InputDeviceType_Camera = 9
    };

    /// <summary>
    /// The TraceLevel enumeration contains a set of values that specify various levels of trace events&mdash;that represent 
    /// printf-style logging details generated by an application.
    /// </summary>
    enum TraceLevel
    {
        /// <summary>No trace event level.</summary>
        TraceLevel_None = 0,
        /// <summary>Error level.</summary>
        TraceLevel_Error = 1,
        /// <summary>Warning level.</summary>
        TraceLevel_Warning = 2,
        /// <summary>Information level.</summary>
        TraceLevel_Information = 3,
        /// <summary>Verbose level.</summary>
        TraceLevel_Verbose = 4
    };

    /// <summary>
    /// The UserState enumeration contains a set of values that specify the user's state. For example, connected.
    /// </summary>
    enum UserState
    {
        /// <summary>User state unknown.</summary>
        UserState_Unknown = 0,
        /// <summary>The user is connected to a service.</summary>
        UserState_Connected = 1,
        /// <summary>The user is reachable for a service like push notification.</summary>
        UserState_Reachable = 2,
        /// <summary>The user is signed-into a service.</summary>
        UserState_SignedIn = 3,
        /// <summary>The user is signed-out of a service.</summary>
        UserState_SignedOut = 4
    };


    /// <summary>
    /// The OsArchitectureType enumeration contains a set of values that specify the type of operating system architecture, 
    /// such as <i>x86</i> or <i>x64</i>. <b>Note:</b> This will be <i>X86</i> for a 32-bit OS even if the processor architecture is x64.
    /// </summary>
    enum OsArchitectureType
    {
        /// <summary>The OS architecture is either unknown or is unavailable.</summary>
        OsArchitectureType_Unknown = 0,
        /// <summary>32-bit (x86) mode.</summary>
        OsArchitectureType_X86 = 1,
        /// <summary>64-bit (x64) mode.</summary>
        OsArchitectureType_X64 = 2,
        /// <summary>ARM processor family.</summary>
        OsArchitectureType_Arm = 3
    };

    /// <summary>
    /// The PowerSource enumeration contains a set of values that specify the state of the device's power source.
    /// </summary>
    enum PowerSource
    {
        /// <summary>Any power source.</summary>
        PowerSource_Any = -1,
        /// <summary>Power source unknown.</summary>
        PowerSource_Unknown = 0,
        /// <summary>Running on battery power.</summary>
        PowerSource_Battery = 1,
        /// <summary>The battery is charging.</summary>
        PowerSource_Charging = 2,
        /// <summary>The battery charge level is low.</summary>
        PowerSource_LowBattery = 3 /* Reserved for future use */
    };

    /// <summary>
    /// The NetworkCost enumeration contains a set of values that specify the kind of network cost for a connected device.
    /// </summary>
    enum NetworkCost
    {
        /// <summary>Any network cost.</summary>
        NetworkCost_Any = -1,
        /// <summary>Network cost unknown.</summary>
        NetworkCost_Unknown = 0,
        /// <summary>Unmetered.</summary>
        NetworkCost_Unmetered = 1,
        /// <summary>Metered.</summary>
        NetworkCost_Metered = 2,
        /// <summary>The device is roaming.</summary>
        NetworkCost_Roaming = 3,
        /// <summary>[deprecated]: Do no use this value.</summary>
        NetworkCost_OverDataLimit = 3
    };

    /// <summary>
    /// The NetworkType enumeration contains a set of values that specify the type of network that a device is connected to.
    /// </summary>
    enum NetworkType
    {
        /// <summary>Any type of network.</summary>
        NetworkType_Any = -1,
        /// <summary>The type of network is unknown.</summary>
        NetworkType_Unknown = 0,
        /// <summary>A wired network.</summary>
        NetworkType_Wired = 1,
        /// <summary>A Wi-fi network.</summary>
        NetworkType_Wifi = 2,
        /// <summary>A wireless wide-area network.</summary>
        NetworkType_WWAN = 3
    };

    /// <summary>
    /// The EventPriority enumeration contains a set of values that specify the priority for an event.
    /// </summary>
    enum EventPriority
    {
        /// <summary>The event priority is not specified.</summary>
        EventPriority_Unspecified = -1,
        /// <summary>The event will not be transmitted.</summary>
        EventPriority_Off = 0,
        /// <summary>low priority.</summary>
        EventPriority_Low = 1,
        /// <summary>Same as EventPriority_Low.</summary>
        EventPriority_MIN = EventPriority_Low,
        /// <summary>Normal priority.</summary>
        EventPriority_Normal = 2,
        /// <summary>High priority.</summary>
        EventPriority_High = 3,
        /// <summary>The event will be transmitted as soon as possible.</summary>
        EventPriority_Immediate = 4,
        /// <summary>Same as EventPriority_Immediate.</summary>
        EventPriority_MAX = EventPriority_Immediate
    };


    /// <summary>
    /// Latency for an event to be transmitted
    /// </summary>
    enum EventLatency
    {
        /// Unspecified: Event Latency is not specified
        EventLatency_Unspecified = -1,

        /// Off: Latency is not to be transmitted
        EventLatency_Off = 0,

        /// Normal: Latency is to be transmitted at low priority
        EventLatency_Normal = 1,

        /// Cost Deffered: Latency is to be transmitted at cost deferred priority
        EventLatency_CostDeferred = 2,

        /// RealTime: Latency is to be transmitted at real time priority
        EventLatency_RealTime = 3,

        /// Max: Latency is to be transmitted as soon as possible
        EventLatency_Max = 4,
    };


    /// <summary>
    /// Persistence for an event
    /// </summary>
    enum EventPersistence
    {
        /// Normal
        EventPersistence_Normal = 1,

        /// Critical: priority upload and last to be evicted from offline storage
        EventPersistence_Critical = 2,

        /// DoNotStoreOnDisk: do not store event in offline storage
        EventPersistence_DoNotStoreOnDisk = 3
    };

    typedef struct
    {
        EventPriority priority;
        EventLatency  latency;
        uint64_t      flags;
    } EventAttributes;

    /// <summary>
    /// HTTP request result codes
    /// </summary>
    enum HttpResult
    {
        /// Response has been received successfully, HTTP status code is
        // available as returned by the target server.
        HttpResult_OK = 0,

        /// Request has been aborted by the caller. The server might or
        /// might not have already received and processed the request.
        HttpResult_Aborted = 1,

        /// Local conditions have prevented the request from being sent
        /// (invalid request parameters, out of memory, internal error etc.).
        HttpResult_LocalFailure = 2,

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
        REJECTED_REASON_SERVER_DECLINED,
        /// <summary>Server declined.</summary>
        REJECTED_REASON_TENANT_KILLED,
        /// <summary>Reject reason OK.</summary>
        REJECTED_REASON_OK,
        /// <summary>Reject reason count.</summary>
        REJECTED_REASON_COUNT
    };

    constexpr const static unsigned gc_NumRejectedReasons = REJECTED_REASON_COUNT;

    typedef enum {
        ACT_STATS_ROLLUP_KIND_START,
        ACT_STATS_ROLLUP_KIND_STOP,
        ACT_STATS_ROLLUP_KIND_ONGOING
    } RollUpKind;

    // Event dropped due to system limitation or failure
    enum EventDroppedReason
    {
        DROPPED_REASON_OFFLINE_STORAGE_SAVE_FAILED,
        DROPPED_REASON_OFFLINE_STORAGE_OVERFLOW,
        DROPPED_REASON_SERVER_DECLINED_4XX,
        DROPPED_REASON_SERVER_DECLINED_5XX,
        DROPPED_REASON_SERVER_DECLINED_OTHER,
        DROPPED_REASON_RETRY_EXCEEDED,
        DROPPED_REASON_COUNT
    };

    constexpr const static unsigned gc_NumDroppedReasons = DROPPED_REASON_COUNT;

} MAT_NS_END

#endif //EVENTPRIORITY_H

