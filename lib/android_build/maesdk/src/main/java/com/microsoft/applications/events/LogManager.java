//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import androidx.annotation.Keep;
import java.util.Date;
import java.util.NavigableSet;
import java.util.TreeMap;
import java.util.UUID;
import java.util.Vector;

public class LogManager {
  private LogManager() {};

  /**
   * ILogConfiguration (limited to VariantMap, no modules)
   *
   * <p>Includes a type-checked enum for known keys.
   *
   * <p>The C++ side can translate values of type Boolean, Long, String, ILogConfiguration (nested
   * maps) and arrays of these types (including nested arrays).
   */
  public static class LogConfigurationImpl extends ILogConfiguration {
    TreeMap<String, Object> configMap;

    LogConfigurationImpl() {
      configMap = new TreeMap<String, Object>();
    }

    public TreeMap<String, Object> getConfigMap() {
      return configMap;
    }

    @Override
    public boolean equals(Object other) {
      if (!this.getClass().isAssignableFrom(other.getClass())) {
        return false;
      }

      LogConfigurationImpl castOther = (LogConfigurationImpl) other;
      if (this.configMap.size() != castOther.configMap.size()) {
        return false;
      }

      NavigableSet<String> set = configMap.navigableKeySet();
      for (String k : set) {
        if (!castOther.configMap.containsKey(k)) {
          return false;
        }
        if (castOther.configMap.get(k) != configMap.get(k)) {
          return false;
        }
      }
      return true;
    }

    /**
     * Intended for unit tests: does this instance contain all of the non-null-valued key-value
     * pairs of the other instance?
     *
     * @param subset the subset configuration
     * @return true if the other is castable to ILogConfigurationImpl and all of its non-null
     *     content appears in this ILogConfigurationImpl
     */
    public boolean valueContainsAll(LogConfigurationImpl subset, StringBuffer failure) {
      NavigableSet<String> keySet = subset.configMap.navigableKeySet();
      for (String k : keySet) {
        Object v = subset.configMap.get(k);
        if (v == null) {
          continue;
        }
        if (!configMap.containsKey(k)) {
          failure.append(String.format("Key %s missing from superset", k));
          return false;
        }
        Object superV = configMap.get(k);
        if (superV == null) {
          failure.append(String.format("Value for key %s is null in superset", k));
          return false;
        }
        if (superV == v) {
          continue;
        }
        if (!superV.getClass().isAssignableFrom(v.getClass())) {
          failure.append(
              String.format(
                  "Value for key %s is class %s in superset, %s in subset",
                  k, superV.getClass().getName(), v.getClass().getName()));
          return false;
        }
        if (LogConfigurationImpl.class.isAssignableFrom(superV.getClass())) {
          LogConfigurationImpl superMap = (LogConfigurationImpl) superV;
          StringBuffer subFailure = new StringBuffer();
          if (superMap.valueContainsAll((LogConfigurationImpl) v, subFailure)) {
            continue;
          }
          failure.append(String.format("Sub-map %s: %s", k, subFailure));
          return false;
        }
        if (superV.getClass().isArray()) {
          if (!v.getClass().isArray()) {
            failure.append(String.format("Super array %s: %s, sub %s", k, superV, v));
            return false;
          }
          Object[] superVA = (Object[]) superV;
          Object[] vA = (Object[]) v;
          if (superVA.length != vA.length) {
            failure.append(
                String.format("Super array length %s: %d, sub %d", k, superVA.length, vA.length));
            return false;
          }
          for (int i = 0; i < superVA.length; i += 1) {
            Object superE = superVA[i];
            Object subE = vA[i];
            if (subE == null) {
              continue;
            }
            if (superE == null) {
              failure.append(String.format("Super %s[%d] is null", k, i));
              return false;
            }
            if (!superE.getClass().isAssignableFrom(subE.getClass())) {
              failure.append(
                  String.format(
                      "Value for key %s[%d] is class %s in superset, %s in subset",
                      k, i, superE.getClass().getName(), subE.getClass().getName()));
              return false;
            }
            if (LogConfigurationImpl.class.isAssignableFrom(superE.getClass())) {
              LogConfigurationImpl superMap = (LogConfigurationImpl) superE;
              StringBuffer subFailure = new StringBuffer();
              if (superMap.valueContainsAll((LogConfigurationImpl) subE, subFailure)) {
                continue;
              }
              failure.append(String.format("Sub-map %s: %s", k, subFailure));
              return false;
            }
            if (!superE.equals(superE.getClass().cast(subE))) {
              failure.append(String.format("not equal %s[%d]: %s != %s", k, i, superE, subE));
              return false;
            }
          }
          continue;
        }
        if (!superV.equals(superV.getClass().cast(v))) {
          String s = String.format("key %s, superset value %s, subset %s", k, superV, v);
          failure.append(s);
          return false;
        }
      }
      return true;
    }

    @Override
    public int hashCode() {
      int result = 0;
      NavigableSet<String> set = configMap.navigableKeySet();
      for (String k : set) {
        result ^= k.hashCode();
        Object v = configMap.get(k);
        if (v != null) {
          result ^= v.hashCode();
        }
      }
      return result;
    }

    @Override
    public String toString() {
      return configMap.toString();
    }

    /**
     * Retrieve the generic Object value for a key.
     *
     * @param key
     * @return the value for that key, or null if there is no entry.
     */
    @Keep
    @Override
    public Object getObject(String key) {
      if (key == null) {
        return null;
      }
      return configMap.get(key);
    }

    /**
     * Retrieve the generic Object value for an enum key.
     *
     * @param key
     * @return the value for that key, or null if there is no entry.
     */
    @Override
    public Object getObject(LogConfigurationKey key) {
      return configMap.get(key.getKey());
    }

    /**
     * Retrieve a Long value
     *
     * @param key
     * @return value, or null if the value can't be cast to Long
     */
    @Override
    public Long getLong(LogConfigurationKey key) {
      Object v = getObject(key);
      if (Long.class.isInstance(v)) {
        return Long.class.cast(v);
      }
      return null;
    }

    /**
     * Retrieve a String value
     *
     * @param key
     * @return value, or null if the value can't be cast to String
     */
    @Override
    public String getString(LogConfigurationKey key) {
      Object v = getObject(key);
      if (String.class.isInstance(v)) {
        return String.class.cast(v);
      }
      return null;
    }

    /**
     * Retrieve a Boolean value
     *
     * @param key
     * @return value, or null if the value can't be cast to Boolean
     */
    @Override
    public Boolean getBoolean(LogConfigurationKey key) {
      Object v = getObject(key);
      if (Boolean.class.isInstance(v)) {
        return Boolean.class.cast(v);
      }
      return null;
    }

    /**
     * Retrieve a nested ILogConfiguration value
     *
     * @param key
     * @return value, or null if the value can't be cast to ILogConfiguration
     */
    @Override
    public ILogConfiguration getLogConfiguration(LogConfigurationKey key) {
      Object v = getObject(key);
      if (ILogConfiguration.class.isInstance(v)) {
        return ILogConfiguration.class.cast(v);
      }
      return null;
    }

    /**
     * Set a Boolean value
     *
     * @param key
     * @param value
     * @return true if we saved the value, false if this is not a boolean-typed key.
     */
    @Override
    public boolean set(LogConfigurationKey key, Boolean value) {
      if (key.getValueType() == Boolean.class) {
        configMap.put(key.getKey(), value);
        return true;
      }
      return false;
    }

    /**
     * Set a Long value
     *
     * @param key
     * @param value
     * @return true if we saved the value, false if this is not a long-typed key.
     */
    @Override
    public boolean set(LogConfigurationKey key, Long value) {
      if (key.getValueType() == Long.class) {
        configMap.put(key.getKey(), value);
        return true;
      }
      return false;
    }

    /**
     * Set a String value
     *
     * @param key
     * @param value
     * @return true if we saved the value, false if this is not a String-typed key.
     */
    @Override
    public boolean set(LogConfigurationKey key, String value) {
      if (key.getValueType() == String.class) {
        configMap.put(key.getKey(), value);
        return true;
      }
      return false;
    }

    /**
     * Set a nested ILogConfiguration value
     *
     * @param key
     * @param value
     * @return true if we saved the value, false if this is not an ILogConfiguration-typed key.
     */
    @Override
    public boolean set(LogConfigurationKey key, ILogConfiguration value) {
      if (key.getValueType() == ILogConfiguration.class) {
        configMap.put(key.getKey(), value);
        return true;
      }
      return false;
    }

    /**
     * Set any value (not type checked)
     *
     * @param key
     * @param value
     */
    @Keep
    @Override
    public void set(String key, Object value) {
      configMap.put(key, value);
    }

    @Override
    public native ILogConfiguration roundTrip();

    /**
     * Return all keys as an array of String
     *
     * @return Sorted array of keys
     */
    @Keep
    public String[] getKeyArray() {
      NavigableSet<String> set = configMap.navigableKeySet();
      String[] result = new String[set.size()];
      int index = 0;
      for (String k : set) {
        result[index] = k;
        index += 1;
      }
      return result;
    }
  }

  /**
   * Track instances of Logger. On FlushAndTeardown(), set all instance native pointers to zero to
   * avoid mysterious use-after-free disasters. Note that the native FlushAndTeardown will delete
   * the corresponding native instances; all we have to do is zero out our now-dangling pointers.
   */
  static Vector<Logger> loggers = new Vector<Logger>();

  /** Called by Logger to register itself on construction */
  public static synchronized void registerLogger(Logger logger) {
    if (logger != null) {
      loggers.add(logger);
    }
  }

  /** Called by Logger on close() to deregister itself. ILogger extends AutoCloseable. */
  public static synchronized void removeLogger(Logger logger) {
    if (logger != null) {
      for (int index = loggers.indexOf(logger); index >= 0; index = loggers.indexOf(logger)) {
        loggers.set(index, loggers.lastElement());
        loggers.setSize(loggers.size() - 1);
      }
    }
  }

  private static native long nativeInitializeWithoutTenantToken();

  /**
   * Initializes the telemetry logging system with default configuration and HTTPClient.
   *
   * @return A logger instance instantiated with the default tenantToken.
   */
  public static ILogger initialize() {
    long logger = nativeInitializeWithoutTenantToken();
    if (logger == 0) return null;
    else return new Logger(logger);
  }

  private static native long nativeInitializeWithTenantToken(String tenantToken);

  /**
   * Initializes the telemetry logging system with the specified tenantToken.
   *
   * @param tenantToken Token of the tenant with which the application is associated for collecting
   *     telemetry
   * @return A logger instance instantiated with the tenantToken.
   */
  public static ILogger initialize(final String tenantToken) {
    if (tenantToken == null || tenantToken.trim().isEmpty())
      throw new IllegalArgumentException("tenantToken is null or empty");

    long logger = nativeInitializeWithTenantToken(tenantToken);
    if (logger == 0) return null;
    else return new Logger(logger);
  }

  private static native long nativeInitializeConfig(
      String tenantToken, ILogConfiguration configuration);

  /**
   * Initializes the telemetry logging system with the specified tenantToken (which may be empty)
   * and ILogConfiguration object.
   *
   * @return a new instance of Logger
   */
  public static ILogger initialize(final String tenantToken, ILogConfiguration configuration) {
    String protectedToken = tenantToken;
    if (protectedToken == null) {
      protectedToken = "";
    }
    long logger = nativeInitializeConfig(protectedToken, configuration);
    if (logger == 0) {
      return null;
    }
    return new Logger(logger);
  }

  private static native int nativeFlushAndTeardown();

  /**
   * Flush any pending telemetry events in memory to disk and tear down the telemetry logging
   * system. Invalidates all Logger instances.
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static synchronized Status flushAndTeardown() {
    for (Logger logger : loggers) {
      logger.clearNative();
    }
    loggers.clear();
    return Status.getEnum(nativeFlushAndTeardown());
  }

  private static native int nativeFlush();

  /**
   * Flush any pending telemetry events in memory to disk to reduce possible data loss as seen
   * necessary. This function can be very expensive so should be used sparingly. OS will block the
   * calling thread and might flush the global file buffers, i.e. all buffered filesystem data, to
   * disk, which could be time consuming.
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status flush() {
    return Status.getEnum(nativeFlush());
  }

  private static native int nativeUploadNow();

  /**
   * Try to send any pending telemetry events in memory or on disk.
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status uploadNow() {
    return Status.getEnum(nativeUploadNow());
  }

  private static native int nativePauseTransmission();

  /**
   * Pauses the transmission of events to data collector. While paused events will continue to be
   * queued up on client side in cache (either in memory or on disk file).
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status pauseTransmission() {
    return Status.getEnum(nativePauseTransmission());
  }

  private static native int nativeResumeTransmission();

  /**
   * Resumes the transmission of events to data collector.
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status resumeTransmission() {
    return Status.getEnum(nativeResumeTransmission());
  }

  private static native int nativeSetIntTicketToken(int type, final String tokenValue);

  /**
   * Sets the token ID with the value.
   *
   * @param type Type of token(like AAD etc)
   * @param tokenValue Value of the token
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setTicketToken(TicketType type, final String tokenValue) {
    if (type == null) throw new IllegalArgumentException("type is null");

    return Status.getEnum(nativeSetIntTicketToken(type.getValue(), tokenValue));
  }

  private static native int nativeSetIntTransmitProfile(int profile);

  /**
   * Sets transmit profile for event transmission to one of the built-in profiles. A transmit
   * profile is a collection of hardware and system settings (like network connectivity, power
   * state) based on which to determine how events are to be transmitted.
   *
   * @param profile Transmit profile
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setTransmitProfile(TransmitProfile profile) {
    if (profile == null) throw new IllegalArgumentException("profile is null");

    return Status.getEnum(nativeSetIntTransmitProfile(profile.getValue()));
  }

  private static native int nativeSetTransmitProfileString(String profile);

  /**
   * Sets transmit profile for event transmission. A transmit profile is a collection of hardware
   * and system settings (like network connectivity, power state) based on which to determine how
   * events are to be transmitted.
   *
   * @param profile Transmit profile
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setTransmitProfile(String profile) {
    if (profile == null || profile.trim().isEmpty())
      throw new IllegalArgumentException("profile is null or empty");

    return Status.getEnum(nativeSetTransmitProfileString(profile));
  }

  private static native int nativeLoadTransmitProfilesString(String profilesJson);

  /**
   * Load transmit profiles from JSON config
   *
   * @param profilesJson JSON config
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status loadTransmitProfiles(String profilesJson) {
    if (profilesJson == null || profilesJson.trim().isEmpty())
      throw new IllegalArgumentException("profilesJson is null or empty");

    return Status.getEnum(nativeLoadTransmitProfilesString(profilesJson));
  }

  private static native int nativeResetTransmitProfiles();

  /**
   * Reset transmission profiles to default settings
   *
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status resetTransmitProfiles() {
    return Status.getEnum(nativeResetTransmitProfiles());
  }

  /** @return Transmit profile name based on built-in profile enum */
  public static native String getTransmitProfileName();

  private static native long nativeGetSemanticContext();

  /**
   * Retrieve an ISemanticContext interface through which to specify context information such as
   * device, system, hardware and user information. Context information set via this API will apply
   * to all logger instance unless they are overwritten by individual logger instance.
   *
   * @return ISemanticContext interface pointer
   */
  public static ISemanticContext getSemanticContext() {

    long semanticContext = nativeGetSemanticContext();
    if (semanticContext == 0) return null;
    else return new SemanticContext(semanticContext);
  }

  private static native int nativeSetContextStringValue(String name, String value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final String value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value String value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final String value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextStringValue(name, value, piiKind.getValue()));
  }

  private static native int nativeSetContextIntValue(String name, int value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Int value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final int value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value Int value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final int value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextIntValue(name, value, piiKind.getValue()));
  }

  private static native int nativeSetContextLongValue(String name, long value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Long value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final long value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value Long value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final long value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextLongValue(name, value, piiKind.getValue()));
  }

  private static native int nativeSetContextDoubleValue(String name, double value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Double value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final double value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value Double value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final double value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextDoubleValue(name, value, piiKind.getValue()));
  }

  private static native int nativeSetContextBoolValue(String name, boolean value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Boolean value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final boolean value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value boolean value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final boolean value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextBoolValue(name, value, piiKind.getValue()));
  }

  private static native int nativeSetContextTimeTicksValue(String name, long value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value TimeTicks value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  private static Status setContext(
      final String name, final TimeTicks value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(
        nativeSetContextTimeTicksValue(name, value.getTicks(), piiKind.getValue()));
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value Date value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final Date value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value Date value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final Date value, final PiiKind piiKind) {
    return setContext(name, new TimeTicks(value), piiKind);
  }

  private static native int nativeSetContextGuidValue(String name, String value, int piiKind);

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance. PiiKind_None is chosen by default.
   *
   * @param name Name of the context property
   * @param value UUID/GUID value of the context property
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final UUID value) {
    return setContext(name, value, PiiKind.None);
  }

  /**
   * Adds or overrides a property of the custom context for the telemetry logging system. Context
   * information set here applies to events generated by all ILogger instances unless it is
   * overwritten on a particular ILogger instance.
   *
   * @param name Name of the context property
   * @param value UUID/GUID value of the context property
   * @param piiKind PIIKind of the context
   * @return Status enum corresponding to the native API execution status_t.
   */
  public static Status setContext(final String name, final UUID value, final PiiKind piiKind) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    return Status.getEnum(nativeSetContextGuidValue(name, value.toString(), piiKind.getValue()));
  }

  private static native long nativeGetLogger();

  /**
   * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
   *
   * @return Logger instance of the ILogger interface
   */
  public static ILogger getLogger() {
    long logger = nativeGetLogger();
    if (logger == 0) return null;
    else return new Logger(logger);
  }

  private static native long nativeGetLoggerWithSource(String source);

  /**
   * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
   *
   * @param source Source name of events sent by this logger instance
   * @return Logger instance of the ILogger interface
   */
  public static ILogger getLogger(final String source) {
    long logger = nativeGetLoggerWithSource(source);
    if (logger == 0) return null;
    else return new Logger(logger);
  }

  private static native long nativeGetLoggerWithTenantTokenAndSource(
      String tenantToken, String source);

  /**
   * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
   *
   * @param tenantToken Token of the tenant with which the application is associated for collecting
   *     telemetry
   * @param source Source name of events sent by this logger instance
   * @return Logger instance of the ILogger interface
   */
  public static ILogger getLogger(final String tenantToken, final String source) {
    long logger = nativeGetLoggerWithTenantTokenAndSource(tenantToken, source);
    if (logger == 0) return null;
    else return new Logger(logger);
  }

  /**
   * Create an instance implementing ILogConfiguration
   *
   * @return a new instance of the private LogConfigurationImpl class
   */
  public static ILogConfiguration logConfigurationFactory() {
    return new LogConfigurationImpl();
  }

  private static native LogConfigurationImpl nativeGetLogConfiguration();

  /** Get a copy of the current configuration */
  public static ILogConfiguration getLogConfigurationCopy() {
    return nativeGetLogConfiguration();
  }

  /**
   * Initializes the default DDV with the machine identifier and enables sending diagnostic data to
   * the remote DDV endpoint.
   *
   * @param machineIdentifier Machine identifier string
   * @param endpoint Remote DDV endpoint connection string
   * @return boolean value for success or failure
   */
  public static native boolean initializeDiagnosticDataViewer(
      String machineIdentifier, String endpoint);

  /** Disable the default data viewer. */
  public static native void disableViewer();

  /**
   * Check if the DDV viewer is enabled.
   *
   * @return boolean value for success or failure
   */
  public static native boolean isViewerEnabled();

  /**
   * Get the current DDV endpoint where the data is being streamed to.
   *
   * @return string denoting the DDV endpoint, empty string if not currently streaming
   */
  public static native String getCurrentEndpoint();

  private static native boolean nativeRegisterPrivacyGuardOnDefaultLogManager();

  /**
   * Register the default instance of Privacy Guard with current LogManager instance.
   * @return `true` if Privacy Guard is initialized and was registered successfully, `false` otherwise.
   */
  public static boolean registerPrivacyGuard() {
    return PrivacyGuard.isInitialized() && nativeRegisterPrivacyGuardOnDefaultLogManager();
  }

  private static native boolean nativeUnregisterPrivacyGuardOnDefaultLogManager();

  /**
   * Unregister the default instance of Privacy Guard from current LogManager instance.
   * @return `true` if Privacy Guard is initialized and was unregistered successfully, `false` otherwise.
   */
  public static boolean unregisterPrivacyGuard() {
    // We need the PG ptr to get the data inspector name to remove it. If PG is already uninitialized,
    // we should let LogManager remove it when it d'tors.
    return PrivacyGuard.isInitialized() && nativeUnregisterPrivacyGuardOnDefaultLogManager();
  }

  private static native boolean nativeRegisterSignalsOnDefaultLogManager();

  public static boolean registerSignals() {
    return Signals.isInitialized() && nativeRegisterSignalsOnDefaultLogManager();
  }

  private static native boolean nativeUnregisterSignalsOnDefaultLogManager();

  public static boolean unregisterSignals() {
    return Signals.isInitialized() && nativeUnregisterSignalsOnDefaultLogManager();
  }

  public static native void pauseActivity();
  public static native void resumeActivity();
  public static native void waitPause();
  public static native boolean startActivity();
  public static native void endActivity();
}
