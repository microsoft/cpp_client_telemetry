package com.microsoft.applications.events;

import java.util.Date;
import java.util.UUID;

public class LogManager {
    private LogManager() {};

    private static native long nativeInitializeWithoutTenantToken();

    /**
     * Initializes the telemetry logging system with default configuration and HTTPClient.
     *
     * @return A logger instance instantiated with the default tenantToken.
     */
    public static ILogger initialize(){
        long logger = nativeInitializeWithoutTenantToken();
        if (logger == 0)
            return null;
        else
            return new Logger(logger);
    }

    private static native long nativeInitializeWithTenantToken(String tenantToken);

    /**
     * Initializes the telemetry logging system with the specified tenantToken.
     *
     * @param tenantToken Token of the tenant with which the application is associated for collecting telemetry
     * @return A logger instance instantiated with the tenantToken.
     */
    public static ILogger initialize(final String tenantToken) {
        if (tenantToken == null || tenantToken.trim().isEmpty())
            throw new IllegalArgumentException("tenantToken is null or empty");

        long logger = nativeInitializeWithTenantToken(tenantToken);
        if (logger == 0)
            return null;
        else
            return new Logger(logger);
    }


    private static native int nativeFlushAndTeardown();

    /**
     * Flush any pending telemetry events in memory to disk and tear down the telemetry logging system.
     *
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status flushAndTeardown() {
        return Status.getEnum(nativeFlushAndTeardown());
    }

    private static native int nativeFlush();

    /**
     * Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
     * This function can be very expensive so should be used sparingly. OS will block the calling thread
     * and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
     * time consuming.
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
     * Pauses the transmission of events to data collector.
     * While paused events will continue to be queued up on client side in cache (either in memory or on disk file).
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

    private static native int nativeSetIntTransmitProfile(int profile);

    /**
     * Sets transmit profile for event transmission to one of the built-in profiles.
     * A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
     * based on which to determine how events are to be transmitted.
     *
     * @param profile Transmit profile
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setTransmitProfile(TransmitProfile profile) {
        if (profile == null)
            throw new IllegalArgumentException("profile is null");

        return Status.getEnum(nativeSetIntTransmitProfile(profile.getValue()));
    }


    private static native int nativeSetTransmitProfileString(String profile);

    /**
     * Sets transmit profile for event transmission.
     * A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
     * based on which to determine how events are to be transmitted.
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

    /**
     * @return Transmit profile name based on built-in profile enum
     */
    public static native String getTransmitProfileName();

    private static native long nativeGetSemanticContext();

    /**
     * Retrieve an ISemanticContext interface through which to specify context information
     * such as device, system, hardware and user information.
     * Context information set via this API will apply to all logger instance unless they
     * are overwritten by individual logger instance.
     *
     * @return ISemanticContext interface pointer
     */
    public static ISemanticContext getSemanticContext() {

        long semanticContext = nativeGetSemanticContext();
        if (semanticContext == 0)
            return null;
        else
            return new SemanticContext(semanticContext);
    }

    private static native int nativeSetContextStringValue(String name, String value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final String value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value String value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final String value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextStringValue(name, value, piiKind.getValue()));
    }

    private static native int nativeSetContextIntValue(String name, int value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Int value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final int value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value Int value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final int value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextIntValue(name, value, piiKind.getValue()));
    }

    private static native int nativeSetContextLongValue(String name, long value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Long value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final long value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value Long value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final long value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextLongValue(name, value, piiKind.getValue()));
    }

    private static native int nativeSetContextDoubleValue(String name, double value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Double value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final double value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value Double value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final double value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextDoubleValue(name, value, piiKind.getValue()));
    }

    private static native int nativeSetContextBoolValue(String name, boolean value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Boolean value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final boolean value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value boolean value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final boolean value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextBoolValue(name, value, piiKind.getValue()));
    }

    private static native int nativeSetContextTimeTicksValue(String name, long value, int piiKind);

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value TimeTicks value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    private static Status setContext(final String name, final TimeTicks value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextTimeTicksValue(name, value.getTicks(), piiKind.getValue()));
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Date value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final Date value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
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
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value UUID/GUID value of the context property
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final UUID value) {
        return setContext(name, value, PiiKind.None);
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     *
     * @param name Name of the context property
     * @param value UUID/GUID value of the context property
     * @param piiKind PIIKind of the context
     * @return Status enum corresponding to the native API execution status_t.
     */
    public static Status setContext(final String name, final UUID value, final PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return Status.getEnum(nativeSetContextGuidValue(name, value.toString(), piiKind.getValue()));
    }

    private static native long nativeGetLogger();

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     *
     * @return Logger instance of the ILogger interface
     */
    public static ILogger GetLogger() {
        long logger = nativeGetLogger();
        if (logger == 0)
            return null;
        else
            return new Logger(logger);
    }

    private static native long nativeGetLoggerWithSource(String source);

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     *
     * @param source Source name of events sent by this logger instance
     * @return Logger instance of the ILogger interface
     */
    public static ILogger GetLogger(final String source) {
        long logger = nativeGetLoggerWithSource(source);
        if (logger == 0)
            return null;
        else
            return new Logger(logger);
    }

    private static native long nativeGetLoggerWithTenantTokenAndSource(String tenantToken, String source);

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     * 
     * @param tenantToken Token of the tenant with which the application is associated for collecting telemetry
     * @param source Source name of events sent by this logger instance
     * @return Logger instance of the ILogger interface
     */
    public static ILogger GetLogger(final String tenantToken, final String source) {
        long logger = nativeGetLoggerWithTenantTokenAndSource(tenantToken, source);
        if (logger == 0)
            return null;
        else
            return new Logger(logger);
    }

    /**
     * Initializes the default DDV with the machine identifier and
     * enables sending diagnostic data to the remote DDV endpoint.
     *
     * @param machineIdentifier Machine identifier string
     * @param endpoint Remote DDV endpoint connection string
     * @return boolean value for success or failure
     */
    public native static boolean initializeDiagnosticDataViewer(String machineIdentifier, String endpoint);

    /**
     * Disable the default data viewer.
     */
    public native static void disableViewer();

    /**
     * Check if the DDV viewer is enabled.
     *
     * @return boolean value for success or failure
     */
    public native static boolean isViewerEnabled();
}
