//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;

/**
 * Receives copies of packets uploaded by the SDK.
 *
 * <p>Implementations must return a stable, unique name for the lifetime of the registration.
 * Callbacks can occur on an SDK worker thread and should return promptly. Implementations must not
 * reenter the SDK from within a callback: do not register or unregister viewers, and do not close
 * the owning {@link ILogManager}, because closing unregisters every viewer while the callback is
 * still in progress.
 */
@Keep
public interface IDataViewer {

  /** Receives an encoded telemetry packet after it has been prepared for upload. */
  void receiveData(byte[] packetData);

  /** Returns the stable, unique name used to register this viewer. */
  String getName();

  /** Returns whether this viewer is currently accepting packet callbacks. */
  boolean isTransmissionEnabled();

  /** Returns the endpoint currently used by this viewer, or an empty string when disabled. */
  String getCurrentEndpoint();
}
