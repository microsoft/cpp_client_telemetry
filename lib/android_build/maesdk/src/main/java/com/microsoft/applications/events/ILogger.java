//
// Copyright (c) 2015-2020 Microsoft Corporation and Contributors.
// SPDX-License-Identifier: Apache-2.0
//
package com.microsoft.applications.events;

import java.util.ArrayList;
import java.util.Date;
import java.util.Map;
import java.util.UUID;

public interface ILogger extends AutoCloseable {

    /**
     * Gets an ISemanticContext interface through which you can specify the semantic context for this logger instance.
     *
     * @return An instance of the ISemanticContext interface
     */
    public ISemanticContext getSemanticContext();

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final String value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a string that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A string that contains the property value.
     */
    public void setContext(final String name, final String value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final double value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a double that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A double that contains the property value.
     */
    public void setContext(final String name, final double value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     * @param name A string that contains the name of the property.
     *
     * @param value A long that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final long value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int64_t that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A long that contains the property value.
     */
    public void setContext(final String name, final long value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(String name, final int value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * an int32_t that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value An int that contains the property value.
     */
    public void setContext(String name, final int value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final boolean value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a boolean that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A boolean that contains the property value.
     */
    public void setContext(final String name, final boolean value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a Date that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value The property's Date value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final Date value, PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a Date that contains the property value,
     * and tags the property with default PiiKind_None.
     * @param name A string that contains the name of the property.
     * @param value The property's Date value.
     */
    public void setContext(final String name, final Date value);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with its PiiKind (Personal Identifiable Information kind).
     *
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     * @param piiKind One of the ::PiiKind enumeration values.
     */
    public void setContext(final String name, final UUID value, final PiiKind piiKind);

    /**
     * Adds (or overrides) a property of the context associated with this logger instance,
     * taking a string that contains the name of the context,
     * a UUID/GUID that contains the property value,
     * and tags the property with default PiiKind_None.
     *
     * @param name A string that contains the name of the property.
     * @param value A UUID/GUID that contains the property value.
     */
    public void setContext(final String name, final UUID value);

    /**
     * Populate event property using EventProperty value object.
     *
     * @param name Property name.
     * @param prop Property value object.
     */
    public void SetContext(final String name, final EventProperty prop);

    /**
     * Allows the logger to inherit the alternate parent context.
     *
     * Default context wiring rules:
     *  host loggers inherit their common host LogManager context.
     *  guest loggers do not inherit their host LogManager context due to privacy reasons.
     *
     * @param context he context.
     */
    public void setParentContext(final ISemanticContext context);

    /**
     * Logs the state of the application lifecycle.
     *
     * @param state The state in the application's lifecycle, specified by one of the AppLifecycleState enum values.
     * @param properties Properties of this AppLifecycle event, specified using an EventProperties object.
     */
    public void logAppLifecycle(final AppLifecycleState state, final EventProperties properties);

    /**
     * Logs the state of the application session.
     *
     * @param state The state in the application's lifecycle, as one of the SessionState enumeration values.
     * @param properties Properties of this session event, specified using an EventProperties object.
     */
    public void logSession(final SessionState state, final  EventProperties properties);

    /**
     * Logs the custom event with the specified name.
     *
     * @param name A string that contains the name of the custom event.
     */
    public void logEvent(final String name);

    /**
     * Logs a custom event with the specified name and properties.
     *
     * @param properties Properties of this custom event, specified using an EventProperties object.
     */
    public void logEvent(final EventProperties properties);

    /**
     * Logs a failure event - such as an application exception.
     *
     * @param signature A string that contains the signature that identifies the bucket of the failure.
     * @param detail A string that contains a description of the failure.
     * @param properties Properties of this failure event, specified using an EventProperties object.
     */
    public void logFailure(final String signature, final String detail, final EventProperties properties);

    /**
     * Logs a failure event - such as an application exception.
     *
     * @param signature A string that contains the signature that identifies the bucket of the failure.
     * @param detail A string that contains a description of the failure.
     * @param category A string that contains the category of the failure - such as an application error,
     *                 application not responding, or application crash
     * @param id A string that contains the identifier that uniquely identifies this failure.
     * @param properties Properties of this failure event, specified using an EventProperties object.
     */
    public void logFailure(final String signature, final String detail, final String category,
                           final String id, final EventProperties properties);

    /**
     * Logs a page view event,
     * taking a string that contains the event identifier,
     * a string that contains a friendly name for the page,
     * and properties of the event.<br>
     * <b>Note:</b> A page view event is normally the result of a user action on a UI page
     * such as a search query, a content request, or a page navigation.
     *
     * @param id A string that contains an identifier that uniquely identifies this page.
     * @param pageName A string that contains the friendly name of the page.
     * @param properties Properties of this page view event, specified using an EventProperties object.
     */
    public void logPageView(final String id, final String pageName, final EventProperties properties);

    /**
     * Logs a page view event,
     * taking a string that contains the event identifier,
     * a string that contains a friendly name for the page,
     * a string that contains the page category,
     * a string that contains the page's URI,
     * a string that contains the referring page's URI,
     * and properties of the event.<br>
     * <b>Note:</b> A page view event is normally the result of a user action on a UI page
     * such as a search query, a content request, or a page navigation.
     *
     * @param id A string that contains the identifier that uniquely identifies this page.
     * @param pageName A string that contains the friendly name of the page.
     * @param category A string that contains the category to which this page belongs.
     * @param uri A string that contains the URI of this page.
     * @param referrerUri A string that contains the URI of the page that refers to this page.
     * @param properties Properties of this page view event, specified using an EventProperties object.
     */
    public void logPageView(final String id, final String pageName, final String category,
                            final String uri, final String referrerUri, final EventProperties properties);

    /**
     * Logs a page action event,
     * taking a string that contains the page view identifier,
     * the action type,
     * and the action event properties.
     *
     * @param pageViewId A string that contains an identifier that uniquely identifies the page view.
     * @param actionType The generic type of the page action, specified as one of the ::ActionType enumeration values.
     * @param properties Properties of this page action event, specified using an EventProperties object.
     */
    public void logPageAction(final String pageViewId, final ActionType actionType, final EventProperties properties);

    /**
     * Logs a detailed page action event,
     * taking a reference to the page action data,
     * and the action event properties.
     *
     * @param pageActionData Detailed information about the page action, contained in a PageActionData object.
     * @param properties Properties of this page action event, contained in an EventProperties object.
     */
    public void logPageAction(final PageActionData pageActionData, final EventProperties properties);

    /**
     * Logs a sampled metric event - such as a performance counter,
     * taking a name for the sampled metric,
     * a double that contains the value of the sampled metric,
     * a string that contains the units of measure of the sampled metric,
     * and a reference to an EventProperties object to hold the values.
     *
     * @param name A string that contains the name of the sampled metric.
     * @param value A double that holds the value of the sampled metric.
     * @param units A string that contains the units of the metric value.
     * @param properties Properties of this sampled metric event, specified using an EventProperties object.
     */
    public void logSampledMetric(final String name, double value, final String units, final EventProperties properties);

    /**
     * Logs a sampled metric event - such as a performance counter,
     * taking a name for the sampled metric,
     * a double that contains the value of the sampled metric,
     * a string that contains the units of measure of the sampled metric,
     * a string that contains the name of the metric instance,
     * a string that contains the name of the object class,
     * a string that contains the object identifier,
     * and a reference to an EventProperties object to hold the values.
     *
     * @param name A string that contains the name of the sampled metric.
     * @param value A double that contains the value of the sampled metric.
     * @param units A string that contains the units of the metric value.
     * @param instanceName A string that contains the name of this metric instance. E.g., <i>performance counter</i>.
     * @param objectClass A string that contains the object class for which this metric tracks.
     * @param objectId A string that contains the object identifier for which this metric tracks.
     * @param properties Properties of this sampled metric event, specified using an EventProperties object.
     */
    public void logSampledMetric(final String name, double value, final String units, final String instanceName,
                                 final String objectClass, final String objectId, final EventProperties properties);

    /**
     * Logs a precomputed aggregated metric event. For example, <i>queue length</i>.
     * @param name A string that contains the name of the aggregated metric.
     * @param duration A long that contains the duration (in microseconds) over which this metric is aggregated.
     * @param count A long that contains the count of the aggregated metric observations.
     * @param properties Properties of this aggregated metric event, specified using an EventProperties object.
     */
    public void logAggregatedMetric(final String name, long duration, long count, final EventProperties properties);

    /**
     * Logs a precomputed aggregated metrics event,
     * taking a reference to an AggregatedMetricData object,
     * and a reference to a EventProperties object.
     *
     * @param metricData Detailed information about the aggregated metric, contained in an AggregatedMetricData object.
     * @param properties Properties of this aggregated metric event, specified in an EventProperties object.
     */
    public void logAggregatedMetric(final AggregatedMetricData metricData, final EventProperties properties);

    /**
     * Logs a trace event for troubleshooting.
     *
     * @param level Level of the trace, as one of the TraceLevel enumeration values.
     * @param message A string that contains the a description of the trace.
     * @param properties Properties of this trace event, specified using an EventProperties object.
     */
    public void logTrace(final TraceLevel level, final String message, final EventProperties properties);

    /**
     * Logs a user's state.
     *
     * @param state he user's reported state, specified using one of the ::UserState enumeration values.
     * @param timeToLiveInMillis A long that contains the duration (in milliseconds) for which the state reported is valid.
     * @param properties Properties of this user state event, specified using an EventProperties object.
     */
    public void logUserState(final UserState state, final long timeToLiveInMillis, final EventProperties properties);

    /**
     * Set default diagnostic level of this logger instance.
     *
     * @param level Diagnostic level.
     */
    public void setLevel(final DiagnosticLevel level);

    /**
     * Get the native pointer for ILogger* for components that rely on it
     * @return long representing the native pointer for ILogger *
     */
    public long getNativeILoggerPtr();
}

