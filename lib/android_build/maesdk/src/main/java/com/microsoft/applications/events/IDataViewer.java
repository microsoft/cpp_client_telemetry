//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;

/**
 * Receives copies of packets uploaded by the SDK.
 *
 * <p>Implementations must return a stable, unique name for the lifetime of the registration, and
 * must be thread-safe: the SDK does not serialize callbacks, so {@link #receiveData} and {@link
 * #isTransmissionEnabled()} may run concurrently on the same viewer. Callbacks occur on an SDK
 * worker thread and should return promptly.
 *
 * <p>Unregistering and {@link ILogManager#close} do not wait for a callback already in progress,
 * so a viewer may receive one final packet after removal returns. The SDK keeps the viewer alive
 * for that callback, so implementations need only discard it.
 *
 * <p>Implementations must not reenter the SDK from within a callback: do not register or
 * unregister viewers, and do not close the owning {@link ILogManager}.
 */
@Keep
public interface IDataViewer {

  /** Receives an encoded telemetry packet after it has been prepared for upload. */
  void receiveData(byte[] packetData);

  /**
   * Returns the stable, unique name used to register this viewer.
   *
   * <p>May be called while the SDK holds an internal lock: return a precomputed value and do not
   * call back into the SDK.
   */
  String getName();

  /**
   * Returns whether this viewer is currently accepting packet callbacks.
   *
   * <p>The SDK queries this before each dispatch and skips {@link #receiveData} while it returns
   * {@code false}, regardless of other viewers. Because the query and the delivery are separate
   * calls, a viewer disabled concurrently with a dispatch under way may still receive that packet.
   */
  boolean isTransmissionEnabled();

  /** Returns the endpoint currently used by this viewer, or an empty string when disabled. */
  String getCurrentEndpoint();
}
