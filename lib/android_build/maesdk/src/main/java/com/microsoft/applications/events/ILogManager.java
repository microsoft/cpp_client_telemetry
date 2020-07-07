package com.microsoft.applications.events;

public interface ILogManager extends AutoCloseable {
  public ILogger getLogger(String tenantToken);
  public ILogConfiguration getLogConfigurationCopy();
}
