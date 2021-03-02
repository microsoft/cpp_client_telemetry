//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef DEBUGEVENTS_HPP
#define DEBUGEVENTS_HPP

#include "ctmacros.hpp"

#include <cstdint>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <functional>
#include <algorithm>
#include <chrono>

#ifndef __cplusplus_cli
#include <atomic>
#endif

#ifdef _MANAGED
#include <msclr/lock.h>
public ref class DebugEventLock {
public:
    static Object ^ lock = gcnew Object();
};
#define DE_LOCKGUARD(macro_mutex) msclr::lock l(DebugEventLock::lock);
#else
#include <mutex>
#define DE_LOCKGUARD(macro_mutex)                   \
    std::lock_guard<std::recursive_mutex> TOKENPASTE2(__guard_, __LINE__) (macro_mutex);
#endif

#include <cstdio>
#include <cstdlib>

namespace MAT_NS_BEGIN
{

    /// <summary>
    /// DebugEventType enumeration contains a set of event types supported by SDK
    /// </summary>
    typedef enum DebugEventType
    {
        /// <summary>API call: logEvent.</summary>
        EVT_LOG_EVENT           = 0x01000000,
        /// <summary>API call: logAppLifecycle.</summary>
        EVT_LOG_LIFECYCLE       = 0x01000001,
        /// <summary>API call: logFailure.</summary>
        EVT_LOG_FAILURE         = 0x01000002,
        /// <summary>API call: logPageView.</summary>
        EVT_LOG_PAGEVIEW        = 0x01000004,
        /// <summary>API call: logPageAction.</summary>
        EVT_LOG_PAGEACTION      = 0x01000005,
        /// <summary>API call: logSampledMetric.</summary>
        EVT_LOG_SAMPLEMETR      = 0x01000006,
        /// <summary>API call: logAggregatedMetric.</summary>
        EVT_LOG_AGGRMETR        = 0x01000007,
        /// <summary>API call: logTrace.</summary>
        EVT_LOG_TRACE           = 0x01000008,
        /// <summary>API call: logUserState.</summary>
        EVT_LOG_USERSTATE       = 0x01000009,
        /// <summary>API call: logSession.</summary>
        EVT_LOG_SESSION         = 0x0100000A,

        /// <summary>Event(s) added to queue.</summary>
        EVT_ADDED               = 0x01001000,
        /// <summary>Event(s) cached in offline storage.</summary>
        EVT_CACHED              = 0x02000000,
        /// <summary>Event(s) dropped.</summary>
        EVT_DROPPED             = 0x03000000,
        /// <summary>Event(s) filtered.</summary>
        EVT_FILTERED            = 0x03000001,

        /// <summary>Event(s) sent.</summary>
        EVT_SENT                = 0x04000000,
        /// <summary>Event(s) being uploaded.</summary>
        EVT_SENDING             = 0x04000000,
        /// <summary>Event(s) send failed.</summary>
        EVT_SEND_FAILED         = 0x04000001,
        /// <summary>Event(s) send retry.</summary>
        EVT_SEND_RETRY          = 0x04000002,
        /// <summary>Event(s) retry drop.</summary>
        EVT_SEND_RETRY_DROPPED  = 0x04000003,
        /// <summary>Event(s) skip UTC registration.</summary>
        EVT_SEND_SKIP_UTC_REGISTRATION = 0x04000004,
        /// <summary>Event(s) rejected, e.g.
        /// Failed regexp check or missing event name.
        /// </summary>
        EVT_REJECTED            = 0x05000000,

        /// <summary>HTTP client state events.</summary>
        EVT_HTTP_STATE          = 0x09000000,

        /// BEGIN: deprecated events group
        ///
        /// Events below might have been reported by previous
        /// version of SDK (Aria-v1), and/or by earlier versions
        /// of 1DS C++ SDK. Please use generic EVT_HTTP_STATE
        /// event instead.
        ///
        /// <summary>HTTP stack failure.</summary>
        EVT_CONN_FAILURE        = 0x0A000000,
        /// <summary>HTTP stack failure.</summary>
        EVT_HTTP_FAILURE        = 0x0A000001,
        /// <summary>Compression failed.</summary>
        EVT_COMPRESS_FAILED     = 0x0A000002,
        /// <summary>HTTP stack unknown host.</summary>
        EVT_UNKNOWN_HOST        = 0x0A000003,
        /// END: deprecated events group

        /// <summary>HTTP response error.</summary>
        EVT_HTTP_ERROR          = 0x0B000000,
        /// <summary>HTTP response 200 OK.</summary>
        EVT_HTTP_OK             = 0x0C000000,

        /// <summary>Network state change.</summary>
        EVT_NET_CHANGED         = 0x0D000000,
        
        /// <summary>Storage full.</summary>
        EVT_STORAGE_FULL        = 0x0E000000,
        /// <summary>Storage failed.</summary>
        EVT_STORAGE_FAILED      = 0x0E000001,

        /// <summary>Ticket Expired</summary>
        EVT_TICKET_EXPIRED      = 0x0F000000,
        /// <summary>Unknown error.</summary>
        EVT_UNKNOWN             = 0xDEADBEEF,

        /// <summary>TODO: Allow us to monitor all events types rather than just specific event types.</summary>
        //EVT_MASK_ALL        = 0xFFFFFFFF // We don't allow the 'all' handler at this time.
    } DebugEventType;

    /// <summary>The DebugEvent class represents a debug event object.</summary>
    class DebugEvent
    {

    public:
        /// <summary>The debug event sequence number.</summary>
        uint64_t seq;
        /// <summary>The debug event timestamp.</summary>
        uint64_t ts;
        /// <summary>The debug event type.</summary>
        DebugEventType type;
        /// <summary>[optional] Parameter 1 (depends on debug event type).</summary>
        size_t param1;
        /// <summary>[optional] Parameter 2 (depends on debug event type).</summary>
        size_t param2;
        /// <summary>[optional] The debug event data (depends on debug event type).</summary>
        void* data;
        /// <summary>[optional] The size of the debug event data (depends on debug event type).</summary>
        size_t size;

        /// <summary>DebugEvent The default DebugEvent constructor.</summary>
        DebugEvent() : seq(0), ts(0), type(EVT_UNKNOWN), param1(0), param2(0), data(NULL), size(0) {};

        DebugEvent(DebugEventType type) : seq(0), ts(0), type(type), param1(0), param2(0), data(NULL), size(0) {};

        DebugEvent(DebugEventType type, size_t param1, size_t param2 = 0, void* data = nullptr, size_t size = 0) :
            seq(0), ts(0), type(type), param1(param1), param2(param2), data(data), size(size) {};
    };

    /// <summary>
    /// The DebugEventListener class allows applications to register Microsoft Telemetry SDK debug callbacks
    /// for debugging and unit testing (not recommended for use in a production environment).
    /// 
    /// Customers can implement this abstract class to track when certain events
    /// happen under the hood in the Microsoft Telemetry SDK. The callback is synchronously executed
    /// within the context of the Microsoft Telemetry worker thread.
    /// </summary>
    class MATSDK_LIBABI DebugEventListener
    {

    public:
        /// <summary>The DebugEventListener constructor.</summary>
        virtual void OnDebugEvent(DebugEvent &evt) = 0;

        /// <summary>The DebugEventListener destructor.</summary>
        virtual ~DebugEventListener() noexcept = default;
    };

    class MATSDK_LIBABI DebugEventDispatcher
    {
    public:

        /// <summary>Dispatches the specified event to a client callback.</summary>
        virtual bool DispatchEvent(DebugEvent evt) = 0;

        /// <summary>The DebugEventDispatcher destructor.</summary>
        virtual ~DebugEventDispatcher() noexcept = default;
    };

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )
#endif
    /// <summary>The DebugEventSource class represents a debug event source.</summary>
    class MATSDK_LIBABI DebugEventSource: public DebugEventDispatcher
    {
    public:
        /// <summary>The DebugEventSource constructor.</summary>
        DebugEventSource() : seq(0) {}

        /// <summary>Adds an event listener for the specified debug event type.</summary>
        virtual void AddEventListener(DebugEventType type, DebugEventListener &listener);

        /// <summary>Removes previously added debug event listener for the specified type.</summary>
        virtual void RemoveEventListener(DebugEventType type, DebugEventListener &listener);

        /// <summary>Dispatches the specified event to a client callback.</summary>
        virtual bool DispatchEvent(DebugEvent evt) override;

        /// <summary>Attach cascaded DebugEventSource to forward all events to</summary>
        virtual bool AttachEventSource(DebugEventSource & other);

        /// <summary>Detach cascaded DebugEventSource to forward all events to</summary>
        virtual bool DetachEventSource(DebugEventSource & other);

    protected:
#ifndef _MANAGED
        /// <summary>
        /// Native code lock used for executing singleton state-management methods in a thread-safe manner.
        /// Managed code uses a different DebugEventLock.
        /// </summary>
        static std::recursive_mutex& stateLock()
        {
            // Magic static is thread-safe in C++
            static std::recursive_mutex lock;
            return lock;
        }
#endif

        /// <summary>A collection of debug event listeners.</summary>
        std::map<unsigned, std::vector<DebugEventListener*> > listeners;

        /// <summary>A collection of cascaded debug event sources.</summary>
        std::set<DebugEventSource*> cascaded;

        uint64_t seq;
    };
#ifdef _MSC_VER
#pragma warning( pop )
#endif

} MAT_NS_END

#endif

