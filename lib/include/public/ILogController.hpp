// Copyright (c) Microsoft. All rights reserved.

#pragma once
#include "Version.hpp"
#include "Enums.hpp"
#include "DebugEvents.hpp"
#include "ILogger.hpp"
#include <string>

// *INDENT-OFF*
namespace Microsoft {
    namespace Applications {
        namespace Telemetry {
            // *INDENT-ON*


            /// <summary>
            /// This class is used to manage the Telemetry logging system
            /// </summary>
            class ARIASDK_LIBABI ILogController
            {
            public:

                /// <summary>
                /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
                /// </summary>
                virtual ~ILogController() {}

                /// <summary>
                /// Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
                /// This function can be very expensive so should be used sparingly. OS will block the calling thread
                /// and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
                /// time consuming.
                /// </summary>
                virtual ACTStatus Flush() = 0;

                /// <summary>
                /// Try to send any pending telemetry events in memory or on disk.
                /// </summary>
                virtual ACTStatus UploadNow() = 0;

                /// <summary>
                /// Pauses the transmission of events to data collector.
                /// While pasued events will continue to be queued up on client side in cache (either in memory or on disk file).
                /// </summary>
                virtual ACTStatus PauseTransmission() = 0;

                /// <summary>
                /// Resumes the transmission of events to data collector.
                /// </summary>
                virtual ACTStatus ResumeTransmission() = 0;

                /// <summary>
                /// Sets transmit profile for event transmission to one of the built-in profiles.
                /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
                /// based on which to determine how events are to be transmitted. 
                /// </summary>
                /// <param name="profile">Transmit profile</param>
                /// <returns>This function doesn't return any value because it always succeeds.</returns>
                virtual ACTStatus  SetTransmitProfile(TransmitProfile profile) = 0;

                /// <summary>
                /// Sets transmit profile for event transmission.
                /// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
                /// based on which to determine how events are to be transmitted. 
                /// </summary>
                /// <param name="profile">Transmit profile</param>
                /// <returns>true if profile is successfully applied, false otherwise</returns>
                virtual ACTStatus  SetTransmitProfile(const std::string& profile) = 0;

                /// <summary>
                /// Load transmit profiles from JSON config
                /// </summary>
                /// <param name="profiles_json">JSON config (see example above)</param>
                /// <returns>true on successful profiles load, false if config is invalid</returns>
                virtual ACTStatus  LoadTransmitProfiles(const std::string& profiles_json) = 0;

                /// <summary>
                /// Reset transmission profiles to default settings
                /// </summary>
                virtual ACTStatus  ResetTransmitProfiles() = 0;

                /// <summary>
                /// Reset transmission profiles to default settings
                /// </summary>
                virtual const std::string& GetTransmitProfileName() = 0;

                /// <summary>
                /// Add Debug callback
                /// </summary>
                virtual void AddEventListener(DebugEventType type, DebugEventListener &listener) = 0;

                /// <summary>
                /// Remove Debug callback
                /// </summary>
                virtual void RemoveEventListener(DebugEventType type, DebugEventListener &listener) = 0;
            };
        }
    }
} // namespace Microsoft::Applications::Telemetry
