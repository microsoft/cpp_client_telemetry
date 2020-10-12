//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

public interface ILogManager extends AutoCloseable {
  public ILogger getLogger(String tenantToken);
  public ILogConfiguration getLogConfigurationCopy();
}

