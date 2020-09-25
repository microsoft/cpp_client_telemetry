package com.microsoft.applications.events;

public interface ILogConfiguration {

  public Long getLong(LogConfigurationKey key);

  public String getString(LogConfigurationKey key);

  public Boolean getBoolean(LogConfigurationKey key);

  public ILogConfiguration getLogConfiguration(LogConfigurationKey key);

  public Object getObject(LogConfigurationKey key);

  public Object getObject(String key);

  public boolean set(LogConfigurationKey key, Boolean value);

  public boolean set(LogConfigurationKey key, Long value);

  public boolean set(LogConfigurationKey key, String value);

  public boolean set(LogConfigurationKey key, ILogConfiguration value);

  public void set(String key, Object value);

  public ILogConfiguration roundTrip();
}
