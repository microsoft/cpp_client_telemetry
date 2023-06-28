//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public abstract class ILogConfiguration {

  public abstract Long getLong(LogConfigurationKey key);

  public abstract String getString(LogConfigurationKey key);

  public abstract Boolean getBoolean(LogConfigurationKey key);

  public abstract ILogConfiguration getLogConfiguration(LogConfigurationKey key);

  public abstract Object getObject(LogConfigurationKey key);

  public abstract Object getObject(String key);

  public abstract boolean set(LogConfigurationKey key, Boolean value);

  public abstract boolean set(LogConfigurationKey key, Long value);

  public abstract boolean set(LogConfigurationKey key, String value);

  public abstract boolean set(LogConfigurationKey key, ILogConfiguration value);

  public abstract void set(String key, Object value);

  public abstract ILogConfiguration roundTrip();

  public static native ILogConfiguration getDefaultConfiguration();
}

