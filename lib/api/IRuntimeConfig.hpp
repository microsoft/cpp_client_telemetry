//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
#ifndef IRUNTIMECONFIG_HPP
#define IRUNTIMECONFIG_HPP

#include "ctmacros.hpp"

#include "ILogger.hpp"
#include "ILogConfiguration.hpp"

#include <string>
#include <map>

namespace MAT_NS_BEGIN
{
    ///@cond INTERNAL_DOCS

    class IRuntimeConfig {

    public:

        virtual Variant & operator[](const char* key) = 0;
        virtual bool HasConfig(const char* key) = 0;

        /// <summary>
        /// Gets the URI of the collector (where telemetry events are sent).
        /// </summary>
        /// <remarks>
        /// <b>Note:</b> Since this method is called for every event, 
        /// you can change the URI dynamically.
        /// </remarks>
        /// <returns>A string that contains the collector URI.</returns>
        virtual std::string GetCollectorUrl() = 0;

        /// <summary>
        /// Check used by uploader sequence to verify if URL is defined.
        /// </summary>
        /// <returns>true if URL is set, false otherwise.</returns>
        virtual bool IsCollectorUrlSet() = 0;

        /// <summary>
        /// Adds extension fields (created by the configuration provider) to an
        /// event.
        /// </summary>
        /// <remarks>
        /// Examples of extension fields are the current configuration ID (e.g., the ETag of
        /// the last received ECS configuration), or a comma-separated list of active
        /// experimentation bits (e.g., ECS experiment IDs).
        /// </remarks>
        /// <param name="extension">A map of extension fields to fill.</param>
        /// <param name="experimentationProject">A string that contains the name of the project.</param>
        /// <param name="eventName">A string that contains the name of the event.</param>
        virtual void DecorateEvent(std::map<std::string, std::string>& extension, std::string const& experimentationProject, std::string const& eventName) = 0;

        /// <summary>
        /// Return latency for the event based on event's parameters provided
        /// </summary>
        /// <remarks>
        /// This method is called for every event and thus latency can be changed
        /// dynamically.
        /// </remarks>
        /// <param name="tenantId">ID of tenant for which event should be sent;
        /// note that it is not a tenant token</param>
        /// <param name="eventName">Specific event name as used in
        /// EventProperties</param>
        /// <returns>Event latency value for the event.</returns>
        virtual EventLatency GetEventLatency(std::string const& tenantId = std::string(), std::string const& eventName = std::string()) = 0;

        /// <summary>
        /// Retrieve usage stats tenant token.
        /// </summary>
        /// <returns>A string that contains the tenant token.</returns>
        virtual std::string GetMetaStatsTenantToken() = 0;

        /// <summary>
        /// Get the interval for sending internal usage stats.
        /// </summary>
        /// <returns>An unsigned integer that contains the interval (measured in seconds).</returns>
        virtual unsigned GetMetaStatsSendIntervalSec() = 0;

        /// <summary>
        /// Gets the maximum size of the offline storage file. You can use this to trigger file trimming.
        /// See also <see cref="GetOfflineStorageResizeThresholdPct"/>.
        /// </summary>
         /// <returns>An unsigned integer that contains the size, in bytes.</returns>
        virtual unsigned GetOfflineStorageMaximumSizeBytes() = 0;

        /// <summary>
        /// Gets the percentage of events dropped when the maximum size of the database is exceeded.
        /// See also <see cref="GetOfflineStorageMaximumSizeBytes"/>.
        /// </summary>
        /// <remarks>
        /// The top <i>N</i> percent of events is sorted by priority, 
        /// and then by the timestamp of events that will be dropped.
        /// </remarks>
        /// <returns>An unsigned integer that contains the percentage of events that will be dropped.</returns>
        virtual unsigned GetOfflineStorageResizeThresholdPct() = 0;

        /// <summary>
        /// Gets the maximum number of retries after which an event that failed to be uploaded, 
        /// is discarded.
        /// </summary>
        /// <remarks>
        /// This method is called each time an event fails to be uploaded 
        /// (except if the failure was the result of an Internet connectivity problem).
        /// </remarks>
        /// <returns>An unsigned integer that contains the maximum number of retries.</returns>
        virtual unsigned GetMaximumRetryCount() = 0;

        /// <summary>
        /// Gets the backoff configuration for the number of event upload retries (in the case of upload errors).
        /// The term backoff refers to the length of time to wait after a failed send attempt&mdash;to reattempt another send.
        /// </summary>
        /// <remarks>
        /// Each time there is an upload failure the the backoff time is doubled (i.e., 1 s., 2 s., 4 s., etc.).
        /// <br><br>
        /// <b>Note:</b> When the configuration changes, the current backoff time is reset to the initial value.
        /// The supported policy is exponential backoff with jitter (introduced deviation).
        /// The configuration string is in the following format:
        ///
        ///     E,<initialDelayMs>,<maximumDelayMs>,<multiplier>,<jitter>
        ///
        /// where the delays are integers (in milliseconds), and the multiplier and jitter 
        /// values are floating-points.
        /// </remarks>
        /// <returns>A string that contains the backoff configuration.</returns>
        virtual std::string GetUploadRetryBackoffConfig() = 0;

        /// <summary>
        /// Determines whether the compression of HTTP requests is enabled.
        /// </summary>
        /// <remarks>
        /// This method is called every time packaged events are sent, and therefore, 
        /// you can enable/disable compression dynamically.
        /// </remarks>
        /// <returns>A boolean value that indicates that either compression is enabled (<i>true</i>), or not (<i>false</i>).</returns>
        virtual bool IsHttpRequestCompressionEnabled() = 0;

        /// <summary>
        /// Returns content encoding method for http request
        /// </summary>
        /// <returns>A string value (<i>deflate</i>) or (<i>gzip</i>).</returns>
        virtual const std::string& GetHttpRequestContentEncoding() const = 0;

        /// <summary>
        /// Gets the minimum bandwidth necessary to start an upload.
        /// </summary>
        /// <remarks>
        /// The returned value is used only if the
        /// telemetry library is configured to use a 
        /// <see cref="IBandwidthController"/> implementation. This method is called
        /// each time an upload is prepared.
        /// </remarks>
        /// <returns>An unsigned integer that contains the minimum bandwidth, in bytes per second.</returns>
        virtual unsigned GetMinimumUploadBandwidthBps() = 0;

        /// <summary>
        /// Gets the maximum payload size for an upload request.
        /// </summary>
        /// <remarks>
        /// The size limit is enforced on uncompressed request data, and does not take
        /// overhead (like HTTPS handshake or HTTP headers) into account. This method
        /// is called every time events are packaged for uploading.<br>
        /// <b>Note:</b> If the returned value stops the library from sending even just one
        /// event, then the limit is ignored in order to still send data.
        /// </remarks>
        /// <returns>An unsigned integer that contains the maximum payload size in bytes.</returns>
        virtual unsigned GetMaximumUploadSizeBytes() = 0;

        /// <summary>
        /// Set custom desired latency for an event
        /// </summary>
        /// <remarks>
        /// Event latency set through this API is only a suggestion with the
        /// lowest latency and might be ignored. It might be overridden by the
        /// configuration implementation (e.g. based on ECS) and the value set
        /// through this method might have no effect.
        /// </remarks>
        /// <param name="tenantId">Tenant ID for the specified event; note that it
        /// is not a tenant token</param>
        /// <param name="eventName">Specific event name as used in
        /// EventProperties</param>
        virtual void SetEventLatency(std::string const& tenantId, std::string const& eventName, EventLatency latency) = 0;

        /// <summary>
        /// Determines if clock skew is enabled.
        /// </summary>
        /// <returns>A boolean value that indicates that clock skew is either enabled (true), or not (false).</returns>
        virtual bool IsClockSkewEnabled() = 0;

        virtual uint32_t GetTeardownTime() = 0;

        /// <summary>
        /// Get UTC channel provider group ID
        /// </summary>
        /// <returns>Provider Group Id</returns>
        virtual const char* GetProviderGroupId() = 0;

        /// <summary>
        /// Get UTC channel provider name
        /// </summary>
        /// <returns>Provider Name</returns>
        virtual const char* GetProviderName() = 0;

        /// <summary>
        /// Get whether to skip registering the iKey with UTC
        /// </summary>
        virtual bool SkipIKeyRegistration() const = 0;

        virtual ~IRuntimeConfig() {};
    };

    /// @endcond

} MAT_NS_END
#endif

