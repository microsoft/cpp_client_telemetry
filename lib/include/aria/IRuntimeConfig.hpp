// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "ILogger.hpp"
#include <string>
#include <map>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


class IRuntimeConfig {
  public:
    virtual ~IRuntimeConfig() {}

    /// <summary>
    /// Set the default config values
    /// </summary>
    /// <remarks>
    /// Called once by LogManager::Initialize() to provide the runtime config
    /// with the default values set in LogConfiguration or hardcoded in the
    /// library. The passed object will be kept alive and will not change until
    /// the last call to any other method of the custom IRuntimeConfig
    /// implementation, which can use the default values for settings it does
    /// not wish to change (e.g. collector URL) or currently override (e.g.
    /// maximum offline storage size).
    /// </remarks>
    /// <param name="defaultConfig">Reference to a default runtime config
    /// object</param>
    virtual void SetDefaultConfig(IRuntimeConfig& defaultConfig) = 0;

    /// <summary>
    /// Return URL of the collector where all telemetry events are to be sent
    /// </summary>
    /// <remarks>
    /// This method is called for every event and thus URL can be changed
    /// dynamically.
    /// </remarks>
    /// <returns>Collector URL string</returns>
    virtual std::string GetCollectorUrl() const = 0;

    /// <summary>
    /// Add any extension fields authored by the configuration provider to an
    /// event
    /// </summary>
    /// <remarks>
    /// Examples of such fields can be current configuration ID (e.g. ETag of
    /// the last received ECS configuration) or comma-separated list of active
    /// experimentation bits (e.g. ECS experiment IDs).
    /// </remarks>
    /// <param name="extension">Map of extension fields to fill</param>
    /// <param name="experimentationProject">Name of the project in
    /// experimentation system (e.g. ECS project name)</param>
    /// <param name="eventName">Name of the event</param>
    virtual void DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) const = 0;

    /// <summary>
    /// Return priority for the event based on event's parameters provided
    /// </summary>
    /// <remarks>
    /// This method is called for every event and thus priority can be changed
    /// dynamically.
    /// </remarks>
    /// <param name="tenantId">ID of tenant for which event should be sent;
    /// note that it is not a tenant token</param>
    /// <param name="eventName">Specific event name as used in
    /// EventProperties</param>
    /// <returns>EventPriority value for the event.</returns>
    virtual EventPriority GetEventPriority(std::string const& tenantId  = std::string(), std::string const& eventName = std::string()) const = 0;

    /// <summary>
    /// Return Aria tenant token for statistic events about the SDK itself
    /// </summary>
    /// <returns>Tenant token for meta-statistics</returns>
    virtual std::string GetMetaStatsTenantToken() const = 0;

    /// <summary>
    /// Return interval period for sending meta-statistics about operation of
    /// the Aria SDK itself
    /// </summary>
    /// <returns>Interval in seconds, 0 to disable (not recommended)</returns>
    virtual unsigned GetMetaStatsSendIntervalSec() const = 0;

    /// <summary>
    /// Return size of offline storage file that should trigger trimming
    /// </summary>
    /// <remarks>
    /// Since trimming is triggered only when the maximum size is exceeded,
    /// the storage size can be temporarily slightly above this configuration
    /// parameter.
    /// </remarks>
    /// <returns>Maximum storage file size in bytes</returns>
    virtual unsigned GetOfflineStorageMaximumSizeBytes() const = 0;

    /// <summary>
    /// Return percent of events to be dropped when maximum size of DB is
    /// exceeded
    /// </summary>
    /// <remarks>
    /// See also <see cref="GetOfflineStorageMaximumSizeBytes"/>. Top <c>N</c>
    /// percent of events sorted by priority and then by timestamp will be
    /// dropped.
    /// </remarks>
    /// <returns>Percentage of events to drop</returns>
    virtual unsigned GetOfflineStorageResizeThresholdPct() const = 0;

    /// <summary>
    /// Return number of retries after which an event that failed to be uploaded
    /// is discarded
    /// </summary>
    /// <remarks>
    /// This method is called each time that some events failed to be uploaded
    /// (except if the reason was internet connectivity problems which do not
    /// count as retries).
    /// </remarks>
    /// <returns>Maximum number of retries</returns>
    virtual unsigned GetMaximumRetryCount() const = 0;

    /// <summary>
    /// Specify backoff configuration for retries in case of upload errors
    /// </summary>
    /// <remarks>
    /// This method is called each time the backoff time is increased because
    /// of a retried transmission and thus the settings can be changed
    /// dynamically (the current backoff time will be reset to the initial
    /// value upon each config change though).
    /// The only supported policy is currently exponential backoff with jitter
    /// and the configuration string must be in format
    /// "<c>E,&lt;initialDelayMs&gt;,&lt;maximumDelayMs&gt;,&lt;multiplier&gt;,&lt;jitter&gt;</c>"
    /// where the delays are integers in milliseconds and the latter two
    /// values can be floating-point numbers.
    /// </remarks>
    /// <returns>String with textual backoff configuration</returns>
    virtual std::string GetUploadRetryBackoffConfig() const = 0;

    /// <summary>
    /// Detemine if compression of HTTP requests is enabled
    /// </summary>
    /// <remarks>
    /// This method is called every time packaged events are sent and thus
    /// compression can be enabled/disabled dynamically.
    /// </remarks>
    /// <returns>Boolean true if compression is enabled, false otherwise</returns>
    virtual bool IsHttpRequestCompressionEnabled() const = 0;

    /// <summary>
    /// Return the minimum necessary available bandwidth to start an upload
    /// </summary>
    /// <remarks>
    /// This method is called and the returned value is used only if the
    /// telemetry library is configured to use some
    /// <see cref="IBandwidthController"/> implementation. The method is called
    /// each time an upload is being prepared.
    /// </remarks>
    /// <returns>The minimum bandwidth in bytes (not bits) per second</returns>
    virtual unsigned GetMinimumUploadBandwidthBps() const = 0;

    /// <summary>
    /// Return the maximum size of payload in one upload request
    /// </summary>
    /// <remarks>
    /// The limit is enforced on uncompressed request data and does not take
    /// overhead like HTTPS handshake or HTTP headers into account. The method
    /// is called once every time some events are being packaged for uploading.
    /// If the returned value would stop the library from sending even just one
    /// event, the limit is ignored in order to still send data through.
    /// </remarks>
    /// <returns>Maximum per request payload size in bytes</returns>
    virtual unsigned GetMaximumUploadSizeBytes() const = 0;

    /// <summary>
    /// Set custom desired priority for an event
    /// </summary>
    /// <remarks>
    /// Event priority set through this API is only a suggestion with the
    /// lowest priority and might be ignored. It might be overriden by the
    /// configuration implementation (e.g. based on ECS) and the value set
    /// through this method might have no effect.
    /// </remarks>
    /// <param name="tenantId">Tenant ID for the specified event; note that it
    /// is not a tenant token</param>
    /// <param name="eventName">Specific event name as used in
    /// EventProperties</param>
    virtual void SetEventPriority(std::string const& tenantId, std::string const& eventName, EventPriority priority) = 0;

	/// <summary>
	/// Clock Skew enabled
	/// </summary>
	/// <remarks>
	virtual bool IsClockSkewEnabled() const = 0;
};


}}} // namespace Microsoft::Applications::Telemetry
