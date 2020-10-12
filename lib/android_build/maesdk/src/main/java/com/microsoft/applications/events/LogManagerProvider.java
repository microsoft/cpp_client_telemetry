//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public class LogManagerProvider {
  public static ILogManager createLogManager(ILogConfiguration config) {
    return new LogManagerImpl(nativeCreateLogManager(config));
  }

  protected static native long nativeCreateLogManager(ILogConfiguration config);

  static class LogManagerImpl implements ILogManager {
    long nativeLogManager = 0;

    private LogManagerImpl() {}

    LogManagerImpl(long nativeLogManager) {
      this.nativeLogManager = nativeLogManager;
    }

    @Override
    public ILogger getLogger(String tenantToken) {
      return null;
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
  }
}

