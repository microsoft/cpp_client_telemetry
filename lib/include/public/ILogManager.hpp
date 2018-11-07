#ifndef ILOGMANAGER_HPP
#define ILOGMANAGER_HPP
// Copyright (c) Microsoft. All rights reserved.

#include "Version.hpp"

#include <climits>
#include <cstdint>
#include <string>

#include "Enums.hpp"
#include "ILogger.hpp"
#include "IAuthTokensController.hpp"
#include "ISemanticContext.hpp"
#include "LogConfiguration.hpp"
#include <LogSessionData.hpp>

#include "TransmitProfiles.hpp"
#include "DebugEvents.hpp"

namespace ARIASDK_NS_BEGIN
{

    class IContextProvider {

    public:
        /// <summary>
        /// Gets the log session data.
        /// </summary>
        /// <returns>The log session data in a pointer to a LogSessionData object.</returns>
        virtual LogSessionData* GetLogSessionData() = 0;

        /// <summary>
        /// Set the Auth ticket controller
        /// </summary>
        virtual IAuthTokensController*  GetAuthTokensController() = 0;

    };

    /// <summary>
    /// This class controls transmission and storage subsystems
    /// </summary>
    class ARIASDK_LIBABI ILogController {

    public:

        /// <summary>
        /// Flushes any pending telemetry events in memory to disk, and tears-down the telemetry logging system.
        /// </summary>
        virtual void FlushAndTeardown() = 0;

        /// <summary>
        /// Flushes any pending telemetry events in memory to disk, to reduce possible data loss.
        /// This method can be expensive, so you should use it sparingly. The operating system blocks the calling thread
        /// and might flush the global file buffers (all buffered file system data) to disk, which can be
        /// time consuming.
        /// </summary>
        virtual status_t Flush() = 0;

        /// <summary>
        /// Pauses the transmission of events to the data collector.
        /// While paused, events continue to be queued on the client, cached either in memory or on disk.
        /// </summary>
        virtual status_t PauseTransmission() = 0;

        /// <summary>
        /// Resumes the transmission of events to the data collector.
        /// </summary>
        virtual status_t ResumeTransmission() = 0;

        /// <summary>
        /// Attempts to send any pending telemetry events that are currently cached either in memory, or on disk.
        /// </summary>
        virtual status_t UploadNow() = 0;

        /// <summary>
        /// Sets the transmit profile for event transmission - to one of the built-in profiles.
        /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
        /// based on which to determine how events are to be transmitted. 
        /// </summary>
        /// <param name="profile">Transmit profile, as one of the ::TransmitProfile enumeration values.</param>
        /// <returns>This method doesn't return a value - because it always succeeds.</returns>
        virtual status_t  SetTransmitProfile(TransmitProfile profile) = 0;

        /// <summary>
        /// Sets the transmit profile for event transmission.
        /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state, etc.).
        /// </summary>
        /// <param name="profile">A string that contains the transmit profile.</param>
        /// <returns>A boolean value that indicates success (true) or failure (false).</returns>
        virtual status_t  SetTransmitProfile(const std::string& profile) = 0;

        /// <summary>
        /// Loads transmit profiles formatted in JSON.
        /// </summary>
        /// <param name="profiles_json">A string that contains the transmit profiles in JSON.</param>
        /// <returns>A boolean value that indicates success (true) or failure (false) if the configuration is invalid.</returns>
        virtual status_t  LoadTransmitProfiles(const std::string& profiles_json) = 0;

        /// <summary>
        /// Resets transmission profiles to default settings.
        /// </summary>
        virtual status_t  ResetTransmitProfiles() = 0;

        /// <summary>
        /// Gets the name of the current transmit profile.
        /// </summary>
        virtual const std::string& GetTransmitProfileName() = 0;

    };

    /// <summary>
    /// This class is used to manage the Events logging system
    /// </summary>
    class ARIASDK_LIBABI ILogManager :
        public ILogController,
        public IContextProvider,
        public DebugEventDispatcher
    {
    public:

        /// <summary>
        /// Dispatches event to this ILogManager instance.
        /// </summary>
        /// <param name="evt">DebugEvent</param>
        /// <returns></returns>
        virtual bool DispatchEvent(DebugEvent evt) override = 0;

        /// <summary>
        /// Dispatches broadcast event to all active ILogManager instances.
        /// </summary>
        /// <param name="evt">DebugEvent</param>
        /// <returns></returns>
        static bool DispatchEventBroadcast(DebugEvent evt);

        /// <summary>
        /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
        /// </summary>
        virtual ~ILogManager() {}

        ///
        virtual void Configure() = 0;

        /// Retrieve an ISemanticContext interface through which to specify context information
        /// such as device, system, hardware and user information.
        /// Context information set via this API will apply to all logger instance unless they
        /// are overwritten by individual logger instance.
        /// </summary>
        /// <returns>ISemanticContext interface pointer</returns>
        virtual ISemanticContext& GetSemanticContext() = 0;

        /// <summary>
        /// Adds or  = 0s a property of the custom context for the telemetry logging system.
        /// Context information set here applies to events generated by all ILogger instances
        /// unless it is overwritten on a particular ILogger instance.
        /// </summary>
        /// <param name="name">Name of the context property</param>
        /// <param name="value">String value of the context property</param>
        /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
        virtual status_t SetContext(std::string const& name, std::string const& value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the custom context for the telemetry logging system.
        /// Context information set here applies to events generated by all ILogger instances 
        /// unless it is overwritten on a particular ILogger instance.
        /// </summary>
        /// <param name="name">Name of the context property</param>
        /// <param name="value">Value of the context property</param>
        /// <param name='piiKind'>PIIKind of the context with PiiKind_None as the default</param>
        virtual status_t SetContext(const std::string& name, const char *value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">Double value of the property</param>
        virtual status_t  SetContext(const std::string& name, double value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">64-bit Integer value of the property</param>
        virtual status_t  SetContext(const std::string& name, int64_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">8-bit Integer value of the property</param>
        virtual status_t SetContext(const std::string& name, int8_t  value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">16-bit Integer value of the property</param>
        virtual status_t SetContext(const std::string& name, int16_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">32-bit Integer value of the property</param>
        virtual  status_t SetContext(const std::string& name, int32_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">8-bit unsigned integer value of the property</param>
        virtual  status_t SetContext(const std::string& name, uint8_t  value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">16-bit unsigned integer value of the property</param>
        virtual  status_t SetContext(const std::string& name, uint16_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">32-bit unsigned integer value of the property</param>
        virtual  status_t SetContext(const std::string& name, uint32_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.<br>
        /// All integer types other than int64_t are currently being converted to int64_t
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">64-bit unsigned integer value of the property</param>
        virtual  status_t SetContext(const std::string& name, uint64_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">Boolean value of the property</param>
        virtual status_t  SetContext(const std::string& name, bool value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or  = 0s a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">.NET time ticks</param>
        virtual status_t  SetContext(const std::string& name, time_ticks_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Adds or overrides a property of the global context.
        /// </summary>
        /// <param name="name">Name of the property</param>
        /// <param name="value">GUID</param>
        virtual status_t  SetContext(const std::string& name, GUID_t value, PiiKind piiKind = PiiKind_None) = 0;

        /// <summary>
        /// Retrieves the ILogger interface of a Logger instance through which you can log telemetry events.
        /// It associates the ILogger interface with the specified experimentation project (e.g. ECS) for retrieving configuration context information.
        /// </summary>
        /// <param name="tenantToken">A string that contains the tenant token associated with this application.</param>
        /// <param name="source">A string that contains the name of the source of events.</param>
        /// <param name="experimentationProject">A string that contains the name of the experimentation project  
        /// whose configuration IDs are associated with events sent by this ILogger.</param>
        /// <returns>A pointer to the ILogger instance.</returns>
        virtual ILogger* GetLogger(std::string const& tenantToken, std::string const& source = std::string(), std::string const& experimentationProject = std::string()) = 0;

        /// <summary>Retrieves the current LogManager instance configuration</summary>
        virtual ILogConfiguration& GetLogConfiguration() = 0;

        /// <summary>
        /// Adds the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void AddEventListener(DebugEventType type, DebugEventListener &listener) = 0;

        /// <summary>
        /// Removes the event listener.
        /// </summary>
        /// <param name="type">The type.</param>
        /// <param name="listener">The listener.</param>
        virtual void RemoveEventListener(DebugEventType type, DebugEventListener &listener) = 0;

        /// <summary>
        /// Attach cascaded DebugEventSource to forward all debug events to
        /// </summary>
        virtual bool AttachEventSource(DebugEventSource & other) = 0;

        /// <summary>
        /// Detach cascaded DebugEventSource to forward all debug events to
        /// </summary>
        virtual bool DetachEventSource(DebugEventSource & other) = 0;

        /// <summary>
        /// Gets the log session data.
        /// </summary>
        /// <returns>The log session data in a pointer to a LogSessionData object.</returns>
        virtual LogSessionData* GetLogSessionData() override = 0;

        /// <summary>
        /// Retrieves the ILogController interface of LogManager to control transmission pipe.
        /// </summary>
        /// <returns>Pointer to the ILogController interface</returns>
        virtual ILogController* GetLogController() = 0;

        /// <summary>
        /// Set the Auth ticket controller
        /// </summary>
        virtual IAuthTokensController*  GetAuthTokensController() override = 0;

        /// <summary>
        /// Set tenant-specific event exclusion filter
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="filterStrings">The events to exclude from uploads, specified as an array of strings</param>
        /// <param name="filterCount">The number of strings in filterStrings</param>
        /// <returns>A positive value on success, a negative value on failure. Never returns 0</returns>
        /// </summary>
        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, uint32_t filterCount) = 0;

        /// <summary>
        /// Set tenant-specific event exclusion filter
        /// <param name="tenantToken">Token of the tenant with which the application is associated for collecting telemetry</param>
        /// <param name="filterStrings">The events to exclude from uploads, specified as an array of strings</param>
        /// <param name="filterRates">The rate at which events are excluded, specified as an array of integers representing a percentage</param>
        /// <param name="filterCount">The number of strings, integers in filterStrings, filterRates</param>
        /// <returns>A positive value on success, a negative value on failure. Never returns 0</returns>
        /// </summary>
        virtual status_t SetExclusionFilter(const char* tenantToken, const char** filterStrings, const uint32_t* filterRates, uint32_t filterCount) = 0;

    };

} ARIASDK_NS_END

#endif
