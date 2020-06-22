package com.microsoft.applications.events;

public interface ILogController {

  /**
   * Flushes any pending telemetry events in memory to disk, and tears-down the telemetry logging system.
   */
  void FlushAndTeardown();

  /**
   * Flushes any pending telemetry events in memory to disk, to reduce possible data loss.
   *
   * This method can be expensive, so you should use it sparingly. The operating system blocks the calling thread
   * and might flush the global file buffers (all buffered file system data) to disk, which can be
   * time consuming.
   */
   Status Flush();

  /**
   * Pauses the transmission of events to the data collector.
   *
   * While paused, events continue to be queued on the client, cached either in memory or on disk.
   */

  Status PauseTransmission();

  /**
   * Resumes the transmission of events to the data collector.
   */
  Status ResumeTransmission();

  /**
   * Attempts to send any pending telemetry events that are currently cached
   * either in memory or on disk.
   */
  Status UploadNow();

  /**
   * Sets the transmit profile for event transmission - to one of the built-in profiles.
   *
   * A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
   * based on which to determine how events are to be transmitted.
   *
   * @param profile transmit profile, as one of the TransmitProfile enumeration values.</param>
   * @return status (always Status.SUCCESS in the present implementation).
   */
  Status SetTransmitProfile(TransmitProfile profile);

  /**
   * Sets the transmit profile for event transmission.
   *
   * A transmit profile is a collection of hardware and system settings
   * (like network connectivity, power state, etc.).
   *
   * @param profile the name of the desired profile.
   */
  Status SetTransmitProfile(final String profile);

  /**
   * Parse transmit profiles from a JSON string
   *
   * @param profiles_json The JSON representation of the profiles.
   */
  Status LoadTransmitProfiles(final String profiles_json);

  /// <summary>
  /// Loads transmit profiles.
  /// </summary>
  /// <param name="profiles">A collection of transmit profiles</param>
  /// <returns>A boolean value that indicates success (true) or failure (false) if the configuration is invalid.</returns>
  /// virtual status_t  LoadTransmitProfiles(const std::vector<TransmitProfileRules>& profiles) noexcept = 0;

  /// <summary>
  /// Resets transmission profiles to default settings.
  /// </summary>
  /**
   * Reset transmit profiles to their default (discarding any custom profiles).
   */
  Status ResetTransmitProfiles();

  /**
   * Get the name of the profile that is currently selected.
   *
   * @return the name
   */
  String GetTransmitProfileName();
}

