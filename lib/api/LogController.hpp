// Copyright (c) Microsoft. All rights reserved.

#ifndef ARIA_LOG_CONTROLLER_HPP
#define ARIA_LOG_CONTROLLER_HPP
#include "Version.hpp"
#include "Enums.hpp"
#include "ILogController.hpp"
#include <string>
#include <mutex>

// *INDENT-OFF*
namespace Microsoft { namespace Applications { namespace Telemetry {
// *INDENT-ON*


/// <summary>
/// This class is used to manage the Telemetry logging system
/// </summary>
class LogController : public ILogController
{
  public:

    LogController();
    /// <summary>
    /// Destroy the telemetry logging system instance. Calls `FlushAndTeardown()` implicitly.
    /// </summary>
    virtual ~LogController();

    /// <summary>
    /// Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
    /// This function can be very expensive so should be used sparingly. OS will block the calling thread
    /// and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
    /// time consuming.
    /// </summary>
    virtual ACTStatus Flush();

	/// <summary>
	/// Try to send any pending telemetry events in memory or on disk.
	/// </summary>
	virtual ACTStatus UploadNow();

    /// <summary>
    /// Pauses the transmission of events to data collector.
    /// While pasued events will continue to be queued up on client side in cache (either in memory or on disk file).
    /// </summary>
    virtual ACTStatus PauseTransmission();

    /// <summary>
    /// Resumes the transmission of events to data collector.
    /// </summary>
    virtual ACTStatus ResumeTransmission();

	/// <summary>
	/// Sets transmit profile for event transmission to one of the built-in profiles.
	/// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
	/// based on which to determine how events are to be transmitted. 
	/// </summary>
	/// <param name="profile">Transmit profile</param>
	/// <returns>This function doesn't return any value because it always succeeds.</returns>
	virtual ACTStatus  SetTransmitProfile(TransmitProfile profile);

  	/// <summary>
	/// Sets transmit profile for event transmission.
	/// A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
	/// based on which to determine how events are to be transmitted. 
	/// </summary>
	/// <param name="profile">Transmit profile</param>
	/// <returns>true if profile is successfully applied, false otherwise</returns>
	virtual ACTStatus  SetTransmitProfile(const std::string& profile) ;

	/// <summary>
	/// Load transmit profiles from JSON config
	/// </summary>
	/// <param name="profiles_json">JSON config (see example above)</param>
	/// <returns>true on successful profiles load, false if config is invalid</returns>
	virtual ACTStatus  LoadTransmitProfiles(const std::string& profiles_json) ;

	/// <summary>
	/// Reset transmission profiles to default settings
	/// </summary>
	virtual ACTStatus  ResetTransmitProfiles();

	/// <summary>
	/// Reset transmission profiles to default settings
	/// </summary>
	virtual const std::string& GetTransmitProfileName();
    
    /// <summary>
    /// Add Debug callback
    /// </summary>
    virtual void AddEventListener(DebugEventType type, DebugEventListener &listener);

    /// <summary>
    /// Remove Debug callback
    /// </summary>
    virtual void RemoveEventListener(DebugEventType type, DebugEventListener &listener);

  private:
      ARIASDK_LOG_DECL_COMPONENT_CLASS();
      std::mutex m_lock;
};


}}} // namespace Microsoft::Applications::Telemetry
#endif //ARIA_LOG_CONTROLLER_HPP