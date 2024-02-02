//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.Date;
import java.util.UUID;

public interface ILogManager extends AutoCloseable {

  public ILogger getLogger(String token, String source, String scope);

  public ILogConfiguration getLogConfigurationCopy();

  public void flushAndTeardown();

  public Status flush();

  public Status uploadNow();

  public Status pauseTransmission();

  public Status resumeTransmission();

  public Status setTicketToken(TicketType type, final String tokenValue);

  public Status setTransmitProfile(TransmitProfile profile);

  public Status setTransmitProfile(String profile);

  public Status loadTransmitProfiles(String profilesJson);

  public Status resetTransmitProfiles();

  public String getTransmitProfileName();

  public ISemanticContext getSemanticContext();

  public Status setContext(final String name, final String value, final PiiKind piiKind);

  public Status setContext(final String name, final int value, final PiiKind piiKind);

  public Status setContext(final String name, final long value, final PiiKind piiKind);

  public Status setContext(final String name, final double value, final PiiKind piiKind);

  public Status setContext(final String name, final boolean value, final PiiKind piiKind);

  public Status setContext(final String name, final Date value, final PiiKind piiKind);

  public Status setContext(final String name, final UUID value, final PiiKind piiKind);

  public boolean initializeDiagnosticDataViewer(String machineIdentifier, String endpoint);

  public void disableViewer();

  public boolean isViewerEnabled();

  public String getCurrentEndpoint();

  public LogSessionData getLogSessionData();

  public void setLevelFilter(int defaultLevel, int[] allowedLevels);

  public void addEventListener(DebugEventType eventType, DebugEventListener listener);

  public void removeEventListener(DebugEventType eventType, DebugEventListener listener);

  public boolean registerPrivacyGuard();

  public boolean unregisterPrivacyGuard();

  public boolean registerSignals();

  public boolean unregisterSignals();

  public void pauseActivity();
  public void resumeActivity();
  public void waitPause();
  public boolean startActivity();
  public void endActivity();
}
