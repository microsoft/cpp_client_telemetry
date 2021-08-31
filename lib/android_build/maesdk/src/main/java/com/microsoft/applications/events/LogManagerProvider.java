//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;
import java.util.Date;
import java.util.UUID;

public class LogManagerProvider {
  public static ILogManager createLogManager(ILogConfiguration config) {
    return new LogManagerImpl(nativeCreateLogManager(config));
  }

  protected static native long nativeCreateLogManager(ILogConfiguration config);

  static class LogManagerImpl implements ILogManager {
    @Keep
    long nativeLogManager = 0;

    private LogManagerImpl() {}

    LogManagerImpl(long nativeLogManager) {
      this.nativeLogManager = nativeLogManager;
    }

    private native long nativeGetLogger(String token, String source, String scope);

    @Override
    public ILogger getLogger(String token, String source, String scope) {
      long nativeLogger = nativeGetLogger(token, source, scope);
      if (nativeLogger == 0) {
        throw new NullPointerException("Null native logger pointer");
      }
      return new Logger(nativeLogger);
    }

    @Override
    public ILogConfiguration getLogConfigurationCopy() {
      return nativeGetLogConfigurationCopy(nativeLogManager);
    }

    protected native ILogConfiguration nativeGetLogConfigurationCopy(long nativeLogManager);

    @Override
    public void close() {
      nativeClose(nativeLogManager);
      nativeLogManager = -1;
    }

    protected native void nativeClose(long nativeLogManager);

    protected native void nativeFlushAndTeardown(long nativeLogManager);

    @Override
    public void flushAndTeardown() {
      nativeFlushAndTeardown(nativeLogManager);
    }

    protected native int nativeFlush(long nativeLogManager);

    @Override
    public Status flush() {
      return Status.getEnum(nativeFlush(nativeLogManager));
    }

    protected native int nativeUploadNow(long nativeLogManager);

    @Override
    public Status uploadNow() {
      return Status.getEnum(nativeUploadNow(nativeLogManager));
    }

    protected native int nativePauseTransmission(long nativeLogManager);

    @Override
    public Status pauseTransmission() {
      return Status.getEnum(nativePauseTransmission(nativeLogManager));
    }

    protected native int nativeResumeTransmission(long nativeLogManager);

    @Override
    public Status resumeTransmission() {
      return Status.getEnum(nativeResumeTransmission(nativeLogManager));
    }

    protected native int nativeSetTransmitProfileTP(long nativeLogManager, int profile);

    @Override
    public Status setTransmitProfile(TransmitProfile profile) {
      return Status.getEnum(nativeSetTransmitProfileTP(nativeLogManager, profile.getValue()));
    }

    protected native int nativeSetTransmitProfileS(long nativeLogManager, String profile);

    @Override
    public Status setTransmitProfile(String profile) {
      return Status.getEnum(nativeSetTransmitProfileS(nativeLogManager, profile));
    }

    protected native int nativeLoadTransmitProfiles(long nativeLogManager, String json);

    @Override
    public Status loadTransmitProfiles(String profilesJson) {
      return Status.getEnum(nativeLoadTransmitProfiles(nativeLogManager, profilesJson));
    }

    protected native int nativeResetTransmitProfiles(long nativeLogManager);

    @Override
    public Status resetTransmitProfiles() {
      return Status.getEnum(nativeResetTransmitProfiles(nativeLogManager));
    }

    protected native String nativeGetTransmitProfileName(long nativeLogManager);

    @Override
    public String getTransmitProfileName() {
      return nativeGetTransmitProfileName(nativeLogManager);
    }

    protected native long nativeGetSemanticContext(long nativeLogManager);

    @Override
    public ISemanticContext getSemanticContext() {
      return new SemanticContext(nativeGetSemanticContext(nativeLogManager));
    }

    protected native int nativeSetContextString(
        long nativeLogManager, String name, String value, int piiKind);

    @Override
    public Status setContext(final String name, final String value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextString(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextInt(
        long nativeLogManager, String name, int value, int piiKind);

    @Override
    public Status setContext(final String name, final int value, final PiiKind piiKind) {
      return Status.getEnum(nativeSetContextInt(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextLong(
        long nativeLogManager, String name, long value, int piiKind);

    @Override
    public Status setContext(final String name, final long value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextLong(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextDouble(
        long nativeLogManager, String name, double value, int piiKind);

    @Override
    public Status setContext(final String name, final double value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextDouble(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextBoolean(
        long nativeLogManager, String name, boolean value, int piiKind);

    @Override
    public Status setContext(final String name, final boolean value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextBoolean(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextDate(
        long nativeLogManager, String name, Date value, int piiKind);

    @Override
    public Status setContext(final String name, final Date value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextDate(nativeLogManager, name, value, piiKind.getValue()));
    }

    protected native int nativeSetContextUUID(
        long nativeLogManager, String name, String value, int piiKind);

    @Override
    public Status setContext(final String name, final UUID value, final PiiKind piiKind) {
      return Status.getEnum(
          nativeSetContextUUID(nativeLogManager, name, value.toString(), piiKind.getValue()));
    }

    protected native boolean nativeInitializeDDV(
        long nativeLogManager, String machineIdentifier, String endpoint);

    @Override
    public boolean initializeDiagnosticDataViewer(String machineIdentifier, String endpoint) {
      return nativeInitializeDDV(nativeLogManager, machineIdentifier, endpoint);
    }

    protected native void nativeDisableViewer(long nativeLogManager);

    @Override
    public void disableViewer() {
      nativeDisableViewer(nativeLogManager);
    }

    protected native boolean nativeIsViewerEnabled(long nativeLogManager);

    @Override
    public boolean isViewerEnabled() {
      return nativeIsViewerEnabled(nativeLogManager);
    }

    protected native String nativeGetCurrentEndpoint(long nativeLogManager);

    @Override
    public String getCurrentEndpoint() {
      return nativeGetCurrentEndpoint(nativeLogManager);
    }

    protected static class LogSessionDataImpl implements LogSessionData {
      @Keep
      private long m_first_time;
      @Keep
      private String m_uuid;

      public LogSessionDataImpl() {
        m_first_time = 0;
        m_uuid = null;
      }

      @Override
      public long getSessionFirstTime() {
        return m_first_time;
      }

      @Override
      public String getSessionSDKUid() {
        return m_uuid;
      }
    }

    protected native void nativeGetLogSessionData(long nativeLogManager, LogSessionDataImpl result);

    @Override
    public LogSessionData getLogSessionData() {
      LogSessionDataImpl result = new LogSessionDataImpl();
      nativeGetLogSessionData(nativeLogManager, result);
      return result;
    }

    protected native void nativeSetLevelFilter(
        long nativeLogManager, int defaultLevel, int[] allowedLevels);

    @Override
    public void setLevelFilter(int defaultLevel, int[] allowedLevels) {
      nativeSetLevelFilter(nativeLogManager, defaultLevel, allowedLevels);
    }

    public native long nativeAddEventListener(long nativeLogManager, long eventType, DebugEventListener listener, long currentIdentity);

    @Override
    public void addEventListener(DebugEventType eventType, DebugEventListener listener) {
      listener.nativeIdentity = nativeAddEventListener(nativeLogManager, eventType.value(), listener, listener.nativeIdentity);
    }

    public native void nativeRemoveEventListener(long nativeLogManager, long eventType, long identity);

    @Override
    public void removeEventListener(DebugEventType eventType, DebugEventListener listener) {
      nativeRemoveEventListener(nativeLogManager, eventType.value(), listener.nativeIdentity);
    }

    private native boolean nativeRegisterPrivacyGuard(long nativeLogManager);

    /**
     * Register the default instance of Privacy Guard with current LogManager instance.
     * @return `true` if Privacy Guard is initialized and was registered successfully, `false` otherwise.
     */
    @Override
    public boolean registerPrivacyGuard() {
      return PrivacyGuard.isInitialized() && nativeRegisterPrivacyGuard(nativeLogManager);
    }

    private native boolean nativeUnregisterPrivacyGuard(long nativeLogManager);

    /**
     * Unregister the default instance of Privacy Guard with current LogManager instance.
     * @return `true` if Privacy Guard is initialized and was unregistered successfully, `false` otherwise.
     */
    @Override
    public boolean unregisterPrivacyGuard() {
      return PrivacyGuard.isInitialized() && nativeUnregisterPrivacyGuard(nativeLogManager);
    }
  }
}
