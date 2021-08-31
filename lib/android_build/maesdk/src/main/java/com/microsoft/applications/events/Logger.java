//
// Copyright (c) Microsoft Corporation. All rights reserved.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.ArrayList;
import java.util.Date;
import java.util.Map;
import java.util.UUID;

class Logger implements ILogger {
  private long m_nativePtr;

  Logger(long nativePtr) {
    m_nativePtr = nativePtr;
    LogManager.registerLogger(this);
  }

  private native long nativeGetSemanticContext(long nativePtr);

  /**
   * Gets an ISemanticContext interface through which you can specify the semantic context for this
   * logger instance.
   *
   * @return An instance of the ISemanticContext interface
   */
  @Override
  public ISemanticContext getSemanticContext() {
    return new SemanticContext(nativeGetSemanticContext(m_nativePtr));
  }

  private static native void nativeSetContextStringValue(
      long nativePtr, String name, String value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a string that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A string that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final String value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextStringValue(m_nativePtr, name, value, piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a string that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value A string that contains the property value.
   */
  @Override
  public void setContext(final String name, final String value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextDoubleValue(
      long nativePtr, String name, double value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a double that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A double that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final double value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextDoubleValue(m_nativePtr, name, value, piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a double that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value A double that contains the property value.
   */
  @Override
  public void setContext(final String name, final double value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextLongValue(
      long nativePtr, String name, long value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, an int64_t that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A long that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final long value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextLongValue(m_nativePtr, name, value, piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, an int64_t that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value A long that contains the property value.
   */
  @Override
  public void setContext(final String name, final long value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextIntValue(
      long nativePtr, String name, int value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, an int32_t that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value An int that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(String name, final int value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextIntValue(m_nativePtr, name, value, piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, an int32_t that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value An int that contains the property value.
   */
  @Override
  public void setContext(String name, final int value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextBoolValue(
      long nativePtr, String name, boolean value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a boolean that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A boolean that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final boolean value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextBoolValue(m_nativePtr, name, value, piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a boolean that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value A boolean that contains the property value.
   */
  @Override
  public void setContext(final String name, final boolean value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextTimeTicksValue(
      long nativePtr, String name, long value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a .NET time_ticks_t that contains the property
   * value, and tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A TimeTicks that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  private void setContext(final String name, final TimeTicks value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextTimeTicksValue(m_nativePtr, name, value.getTicks(), piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a Date that contains the property value, and tags
   * the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value The property's Date value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final Date value, PiiKind piiKind) {
    setContext(name, new TimeTicks(value), piiKind);
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a Date that contains the property value, and tags
   * the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value The property's Date value.
   */
  @Override
  public void setContext(final String name, final Date value) {
    setContext(name, value, PiiKind.None);
  }

  private static native void nativeSetContextGuidValue(
      long nativePtr, String name, String value, int piiKind);

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a UUID/GUID that contains the property value, and
   * tags the property with its PiiKind (Personal Identifiable Information kind).
   *
   * @param name A string that contains the name of the property.
   * @param value A UUID/GUID that contains the property value.
   * @param piiKind One of the ::PiiKind enumeration values.
   */
  @Override
  public void setContext(final String name, final UUID value, final PiiKind piiKind) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (value == null) throw new IllegalArgumentException("value is null");
    if (piiKind == null) throw new IllegalArgumentException("piiKind is null");

    nativeSetContextGuidValue(m_nativePtr, name, value.toString(), piiKind.getValue());
  }

  /**
   * Adds (or overrides) a property of the context associated with this logger instance, taking a
   * string that contains the name of the context, a UUID/GUID that contains the property value, and
   * tags the property with default PiiKind_None.
   *
   * @param name A string that contains the name of the property.
   * @param value A UUID/GUID that contains the property value.
   */
  @Override
  public void setContext(final String name, final UUID value) {
    setContext(name, value, PiiKind.None);
  }

  private native void nativeSetContextEventProperty(
      long nativePtr, String name, EventProperty prop);

  /**
   * Populate event property using EventProperty value object.
   *
   * @param name Property name.
   * @param prop Property value object.
   */
  @Override
  public void SetContext(final String name, final EventProperty prop) {
    if (name == null || !Utils.validatePropertyName(name))
      throw new IllegalArgumentException("name is null or invalid");
    if (prop == null) throw new IllegalArgumentException("prop is null");

    nativeSetContextEventProperty(m_nativePtr, name, prop);
  }

  private native void nativeSetParentContext(long nativeLoggerPtr, long nativeSemanticContextPtr);

  /**
   * Allows the logger to inherit the alternate parent context.
   *
   * <p>Default context wiring rules: host loggers inherit their common host LogManager context.
   * guest loggers do not inherit their host LogManager context due to privacy reasons.
   *
   * @param context he context.
   */
  @Override
  public void setParentContext(final ISemanticContext context) {
    if (context == null) throw new IllegalArgumentException("context is null");

    nativeSetParentContext(m_nativePtr, ((SemanticContext) context).getNativeSemanticContextPtr());
  }

  private native void nativeLogAppLifecycle(
      long nativeLoggerPtr,
      int appLifecycleState,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs the state of the application lifecycle.
   *
   * @param state The state in the application's lifecycle, specified by one of the
   *     AppLifecycleState enum values.
   * @param properties Properties of this AppLifecycle event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logAppLifecycle(final AppLifecycleState state, final EventProperties properties) {
    if (state == null) throw new IllegalArgumentException("state is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogAppLifecycle(
        m_nativePtr,
        state.getValue(),
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogSession(
      long nativeLoggerPtr,
      int sessionState,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs the state of the application session.
   *
   * @param state The state in the application's lifecycle, as one of the SessionState enumeration
   *     values.
   * @param properties Properties of this session event, specified using an EventProperties object.
   */
  @Override
  public void logSession(final SessionState state, final EventProperties properties) {
    if (state == null) throw new IllegalArgumentException("state is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogSession(
        m_nativePtr,
        state.getValue(),
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogEventName(long nativeLoggerPtr, String name);

  /**
   * Logs the custom event with the specified name.
   *
   * @param name A string that contains the name of the custom event.
   */
  @Override
  public void logEvent(final String name) {
    if (name == null || !Utils.validateEventName(name))
      throw new IllegalArgumentException("name is null or invalid");

    nativeLogEventName(m_nativePtr, name);
  }

  private native void nativeLogEventProperties(
      long nativeLoggerPtr,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a custom event with the specified name and properties.
   *
   * @param properties Properties of this custom event, specified using an EventProperties object.
   */
  @Override
  public void logEvent(final EventProperties properties) {
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogEventProperties(
        m_nativePtr,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogFailure(
      long nativeLoggerPtr,
      String signature,
      String detail,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a failure event - such as an application exception.
   *
   * @param signature A string that contains the signature that identifies the bucket of the
   *     failure.
   * @param detail A string that contains a description of the failure.
   * @param properties Properties of this failure event, specified using an EventProperties object.
   */
  @Override
  public void logFailure(
      final String signature, final String detail, final EventProperties properties) {
    if (signature == null || signature.trim().isEmpty())
      throw new IllegalArgumentException("signature is null or empty");
    if (detail == null || detail.trim().isEmpty())
      throw new IllegalArgumentException("detail is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogFailure(
        m_nativePtr,
        signature,
        detail,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogFailureWithCategoryId(
      long nativeLoggerPtr,
      String signature,
      String detail,
      String category,
      String id,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a failure event - such as an application exception.
   *
   * @param signature A string that contains the signature that identifies the bucket of the
   *     failure.
   * @param detail A string that contains a description of the failure.
   * @param category A string that contains the category of the failure - such as an application
   *     error, application not responding, or application crash
   * @param id A string that contains the identifier that uniquely identifies this failure.
   * @param properties Properties of this failure event, specified using an EventProperties object.
   */
  @Override
  public void logFailure(
      final String signature,
      final String detail,
      final String category,
      final String id,
      final EventProperties properties) {
    if (signature == null || signature.trim().isEmpty())
      throw new IllegalArgumentException("signature is null or empty");
    if (detail == null || detail.trim().isEmpty())
      throw new IllegalArgumentException("detail is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogFailureWithCategoryId(
        m_nativePtr,
        signature,
        detail,
        category,
        id,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogPageView(
      long nativeLoggerPtr,
      String id,
      String pageName,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a page view event, taking a string that contains the event identifier, a string that
   * contains a friendly name for the page, and properties of the event.<br>
   * <b>Note:</b> A page view event is normally the result of a user action on a UI page such as a
   * search query, a content request, or a page navigation.
   *
   * @param id A string that contains an identifier that uniquely identifies this page.
   * @param pageName A string that contains the friendly name of the page.
   * @param properties Properties of this page view event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logPageView(
      final String id, final String pageName, final EventProperties properties) {
    if (id == null || id.trim().isEmpty())
      throw new IllegalArgumentException("id is null or empty");
    if (pageName == null || pageName.trim().isEmpty())
      throw new IllegalArgumentException("pageName is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogPageView(
        m_nativePtr,
        id,
        pageName,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogPageViewWithUri(
      long nativeLoggerPtr,
      String id,
      String pageName,
      String category,
      String uri,
      String referrerUri,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a page view event, taking a string that contains the event identifier, a string that
   * contains a friendly name for the page, a string that contains the page category, a string that
   * contains the page's URI, a string that contains the referring page's URI, and properties of the
   * event.<br>
   * <b>Note:</b> A page view event is normally the result of a user action on a UI page such as a
   * search query, a content request, or a page navigation.
   *
   * @param id A string that contains the identifier that uniquely identifies this page.
   * @param pageName A string that contains the friendly name of the page.
   * @param category A string that contains the category to which this page belongs.
   * @param uri A string that contains the URI of this page.
   * @param referrerUri A string that contains the URI of the page that refers to this page.
   * @param properties Properties of this page view event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logPageView(
      final String id,
      final String pageName,
      final String category,
      final String uri,
      final String referrerUri,
      final EventProperties properties) {
    if (id == null || id.trim().isEmpty())
      throw new IllegalArgumentException("id is null or empty");
    if (pageName == null || pageName.trim().isEmpty())
      throw new IllegalArgumentException("pageName is null or empty");
    if (category == null || category.trim().isEmpty())
      throw new IllegalArgumentException("category is null or empty");
    if (uri == null || uri.trim().isEmpty())
      throw new IllegalArgumentException("uri is null or empty");
    if (referrerUri == null || referrerUri.trim().isEmpty())
      throw new IllegalArgumentException("referrerUri is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogPageViewWithUri(
        m_nativePtr,
        id,
        pageName,
        category,
        uri,
        referrerUri,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogPageAction(
      long nativeLoggerPtr,
      String pageViewId,
      int actionType,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a page action event, taking a string that contains the page view identifier, the action
   * type, and the action event properties.
   *
   * @param pageViewId A string that contains an identifier that uniquely identifies the page view.
   * @param actionType The generic type of the page action, specified as one of the ::ActionType
   *     enumeration values.
   * @param properties Properties of this page action event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logPageAction(
      final String pageViewId, final ActionType actionType, final EventProperties properties) {
    if (pageViewId == null || pageViewId.trim().isEmpty())
      throw new IllegalArgumentException("pageViewId is null or empty");
    if (actionType == null) throw new IllegalArgumentException("actionType is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogPageAction(
        m_nativePtr,
        pageViewId,
        actionType.getValue(),
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogPageActionData(
      long nativeLoggerPtr,
      // PageActionData
      String pageViewId,
      int actionType,
      int rawActionType,
      int inputDeviceType,
      String targetItemId,
      String targetItemDataSourceName,
      String targetItemDataSourceCategory,
      String targetItemDataSourceCollection,
      String targetItemLayoutContainer,
      short targetItemLayoutRank,
      String destinationUri,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a detailed page action event, taking a reference to the page action data, and the action
   * event properties.
   *
   * @param pageActionData Detailed information about the page action, contained in a PageActionData
   *     object.
   * @param properties Properties of this page action event, contained in an EventProperties object.
   */
  @Override
  public void logPageAction(final PageActionData pageActionData, final EventProperties properties) {
    if (pageActionData == null) throw new IllegalArgumentException("pageActionData is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogPageActionData(
        m_nativePtr,
        pageActionData.pageViewId,
        pageActionData.actionType.getValue(),
        pageActionData.rawActionType.getValue(),
        pageActionData.inputDeviceType.getValue(),
        pageActionData.targetItemId,
        pageActionData.targetItemDataSourceName,
        pageActionData.targetItemDataSourceCategory,
        pageActionData.targetItemDataSourceCollection,
        pageActionData.targetItemLayoutContainer,
        pageActionData.targetItemLayoutRank,
        pageActionData.destinationUri,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogSampledMetric(
      long nativeLoggerPtr,
      String name,
      double value,
      String units,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a sampled metric event - such as a performance counter, taking a name for the sampled
   * metric, a double that contains the value of the sampled metric, a string that contains the
   * units of measure of the sampled metric, and a reference to an EventProperties object to hold
   * the values.
   *
   * @param name A string that contains the name of the sampled metric.
   * @param value A double that holds the value of the sampled metric.
   * @param units A string that contains the units of the metric value.
   * @param properties Properties of this sampled metric event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logSampledMetric(
      final String name, double value, final String units, final EventProperties properties) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (units == null || units.trim().isEmpty())
      throw new IllegalArgumentException("units is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogSampledMetric(
        m_nativePtr,
        name,
        value,
        units,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogSampledMetricWithObjectId(
      long nativeLoggerPtr,
      String name,
      double value,
      String units,
      String instanceName,
      final String objectClass,
      final String objectId,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a sampled metric event - such as a performance counter, taking a name for the sampled
   * metric, a double that contains the value of the sampled metric, a string that contains the
   * units of measure of the sampled metric, a string that contains the name of the metric instance,
   * a string that contains the name of the object class, a string that contains the object
   * identifier, and a reference to an EventProperties object to hold the values.
   *
   * @param name A string that contains the name of the sampled metric.
   * @param value A double that contains the value of the sampled metric.
   * @param units A string that contains the units of the metric value.
   * @param instanceName A string that contains the name of this metric instance. E.g.,
   *     <i>performance counter</i>.
   * @param objectClass A string that contains the object class for which this metric tracks.
   * @param objectId A string that contains the object identifier for which this metric tracks.
   * @param properties Properties of this sampled metric event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logSampledMetric(
      final String name,
      double value,
      final String units,
      final String instanceName,
      final String objectClass,
      final String objectId,
      final EventProperties properties) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (units == null || units.trim().isEmpty())
      throw new IllegalArgumentException("units is null");
    if (instanceName == null || instanceName.trim().isEmpty())
      throw new IllegalArgumentException("instanceName is null or empty");
    if (objectClass == null || objectClass.trim().isEmpty())
      throw new IllegalArgumentException("objectClass is null or empty");
    if (objectId == null || name.trim().isEmpty())
      throw new IllegalArgumentException("objectId is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogSampledMetricWithObjectId(
        m_nativePtr,
        name,
        value,
        units,
        instanceName,
        objectClass,
        objectId,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogAggregatedMetric(
      long nativeLoggerPtr,
      String name,
      long duration,
      long count,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a precomputed aggregated metric event. For example, <i>queue length</i>.
   *
   * @param name A string that contains the name of the aggregated metric.
   * @param duration A long that contains the duration (in microseconds) over which this metric is
   *     aggregated.
   * @param count A long that contains the count of the aggregated metric observations.
   * @param properties Properties of this aggregated metric event, specified using an
   *     EventProperties object.
   */
  @Override
  public void logAggregatedMetric(
      final String name, long duration, long count, final EventProperties properties) {
    if (name == null || name.trim().isEmpty())
      throw new IllegalArgumentException("name is null or empty");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogAggregatedMetric(
        m_nativePtr,
        name,
        duration,
        count,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogAggregatedMetricData(
      long nativeLoggerPtr,
      // AggregatedMetricData
      String name,
      long duration,
      long count,
      String units,
      String instanceName,
      String objectClass,
      String objectId,
      int[] aggregateTypeKeys,
      double[] aggregateDoubleValues,
      long[] bucketsLongKeys,
      long[] bucketsLongValues,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a precomputed aggregated metrics event, taking a reference to an AggregatedMetricData
   * object, and a reference to a EventProperties object.
   *
   * @param metricData Detailed information about the aggregated metric, contained in an
   *     AggregatedMetricData object.
   * @param properties Properties of this aggregated metric event, specified in an EventProperties
   *     object.
   */
  @Override
  public void logAggregatedMetric(
      final AggregatedMetricData metricData, final EventProperties properties) {
    if (metricData == null) throw new IllegalArgumentException("metricData is null");
    if (properties == null) throw new IllegalArgumentException("properties is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    int[] aggregateTypeKeys = new int[metricData.aggregates.size()];
    double[] aggregateDoubleValues = new double[metricData.aggregates.size()];
    int i = 0;
    for (Map.Entry<AggregateType, Double> entry : metricData.aggregates.entrySet()) {
      aggregateTypeKeys[i] = entry.getKey().getValue();
      aggregateDoubleValues[i] = entry.getValue();
      ++i;
    }

    long[] bucketsLongKeys = new long[metricData.buckets.size()];
    long[] bucketsLongValues = new long[metricData.buckets.size()];
    i = 0;
    for (Map.Entry<Long, Long> entry : metricData.buckets.entrySet()) {
      bucketsLongKeys[i] = entry.getKey();
      bucketsLongValues[i] = entry.getValue();
      ++i;
    }

    nativeLogAggregatedMetricData(
        m_nativePtr,
        metricData.name,
        metricData.duration,
        metricData.count,
        metricData.units,
        metricData.instanceName,
        metricData.objectClass,
        metricData.objectId,
        aggregateTypeKeys,
        aggregateDoubleValues,
        bucketsLongKeys,
        bucketsLongValues,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogTrace(
      long nativeLoggerPtr,
      int traceLevel,
      String message,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a trace event for troubleshooting.
   *
   * @param level Level of the trace, as one of the TraceLevel enumeration values.
   * @param message A string that contains the a description of the trace.
   * @param properties Properties of this trace event, specified using an EventProperties object.
   */
  @Override
  public void logTrace(
      final TraceLevel level, final String message, final EventProperties properties) {
    if (message == null || message.trim().isEmpty())
      throw new IllegalArgumentException("message is null or empty");
    if (level == null) throw new IllegalArgumentException("level is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogTrace(
        m_nativePtr,
        level.getValue(),
        message,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeLogUserState(
      long nativeLoggerPtr,
      int userState,
      long timeToLiveInMillis,
      // EventProperties
      String eventName,
      String eventType,
      int eventLatency,
      int eventPersistence,
      double eventPopSample,
      long eventPolicyBitflags,
      long timestampInMillis,
      Object[] eventPropertyStringKey,
      Object[] eventPropertyValue);

  /**
   * Logs a user's state.
   *
   * @param state The user's reported state, specified using one of the ::UserState enumeration
   *     values.
   * @param timeToLiveInMillis A long that contains the duration (in milliseconds) for which the
   *     state reported is valid.
   * @param properties Properties of this user state event, specified using an EventProperties
   *     object.
   */
  @Override
  public void logUserState(
      final UserState state, final long timeToLiveInMillis, final EventProperties properties) {
    if (state == null) throw new IllegalArgumentException("state is null");

    String eventName = properties.getName();
    String eventType = properties.getType();
    EventLatency eventLatency = properties.getLatency();
    EventPersistence eventPersistence = properties.getPersistence();
    double eventPopSample = properties.getPopSample();
    long eventPolicyBitflags = properties.getPolicyBitFlags();
    long timestampInMillis = properties.getTimestamp();

    ArrayList<String> eventPropertyStringKey = new ArrayList<>();
    ArrayList<EventProperty> eventPropertyValue = new ArrayList<>();
    for (Map.Entry<String, EventProperty> entry : properties.getProperties().entrySet()) {
      eventPropertyStringKey.add(entry.getKey());
      eventPropertyValue.add(entry.getValue());
    }

    nativeLogUserState(
        m_nativePtr,
        state.getValue(),
        timeToLiveInMillis,
        eventName,
        eventType,
        eventLatency.getValue(),
        eventPersistence.getValue(),
        eventPopSample,
        eventPolicyBitflags,
        timestampInMillis,
        eventPropertyStringKey.toArray(),
        eventPropertyValue.toArray());
  }

  private native void nativeSetLevel(long nativeLoggerPtr, int level);

  /**
   * Set default diagnostic level of this logger instance.
   *
   * @param level Diagnostic level.
   */
  @Override
  public void setLevel(final DiagnosticLevel level) {
    nativeSetLevel(m_nativePtr, level.getValue());
  }

  @Override
  public void close() {
    LogManager.removeLogger(this);
    clearNative();
  }

  /**
   * Get the native pointer for ILogger* for components that rely on it
   * @return long representing the native pointer for ILogger *
   */
  @Override
  public long getNativeILoggerPtr()
  {
    return m_nativePtr;
  }

  public synchronized void clearNative() {
    m_nativePtr = 0;
  }
}

