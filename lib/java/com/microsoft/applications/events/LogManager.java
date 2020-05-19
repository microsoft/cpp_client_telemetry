package com.microsoft.applications.events;

import java.util.Date;
import java.util.UUID;

public class LogManager {
    private LogManager() {};

    private static native long initializeWithoutTenantToken();

    /**
     * Initializes the telemetry logging system with default configuration and HTTPClient.
     *
     * @return A logger instance instantiated with the default tenantToken.
     */
    public static ILogger initialize(){
        return new Logger(initializeWithoutTenantToken());
    }

    private static native long initializeWithTenantToken(String tenantToken);

    /**
     * Initializes the telemetry logging system with the specified tenantToken.
     *
     * @param tenantToken Token of the tenant with which the application is associated for collecting telemetry
     * @return A logger instance instantiated with the tenantToken.
     */
    public static ILogger initialize(final String tenantToken) {
        if (tenantToken == null || tenantToken.trim().isEmpty())
            throw new IllegalArgumentException("tenantToken is null or empty");

        return new Logger(initializeWithTenantToken(tenantToken));
    }

    /**
     * Flush any pending telemetry events in memory to disk and tear down the telemetry logging system.
     *
     * @return native value of status_t
     */
    public static native int flushAndTeardown();

    /**
     * Try to send any pending telemetry events in memory or on disk.
     *
     * @return native value of status_t
     */
    public static native int uploadNow();

    /**
     * Flush any pending telemetry events in memory to disk to reduce possible data loss as seen necessary.
     * This function can be very expensive so should be used sparingly. OS will block the calling thread
     * and might flush the global file buffers, i.e. all buffered filesystem data, to disk, which could be
     * time consuming.
     *
     * @return native value of status_t
     */
    public static native int flush();

    /**
     * Pauses the transmission of events to data collector.
     * While paused events will continue to be queued up on client side in cache (either in memory or on disk file).
     *
     * @return native value of status_t
     */
    public static native int pauseTransmission();

    /**
     * Resumes the transmission of events to data collector.
     *
     * @return native value of status_t
     */
    public static native int resumeTransmission();

    private static native int setIntTransmitProfile(int profile);

    /**
     * Sets transmit profile for event transmission to one of the built-in profiles.
     * A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
     * based on which to determine how events are to be transmitted.
     *
     * @param profile Transmit profile
     * @return native value of status_t
     */
    public static int setTransmitProfile(TransmitProfile profile) {
        if (profile == null)
            throw new IllegalArgumentException("profile is null");

        return setIntTransmitProfile(profile.getValue());
    }


    public static native int setTransmitProfileString(String profile);

    /**
     * Sets transmit profile for event transmission.
     * A transmit profile is a collection of hardware and system settings (like network connectivity, power state)
     * based on which to determine how events are to be transmitted.
     *
     * @param profile Transmit profile
     * @return native value of status_t
     */

    public static int setTransmitProfile(String profile) {
        if (profile == null || profile.trim().isEmpty())
            throw new IllegalArgumentException("profile is null or empty");

        return setTransmitProfileString(profile);
    }

    private static native int loadTransmitProfilesString(String profilesJson);

    /**
     * Load transmit profiles from JSON config
     *
     * @param profilesJson JSON config
     * @return native value of status_t
     */
    public static int loadTransmitProfiles(String profilesJson) {
        if (profilesJson == null || profilesJson.trim().isEmpty())
            throw new IllegalArgumentException("profilesJson is null or empty");

        return loadTransmitProfilesString(profilesJson);
    }

    /**
     * Reset transmission profiles to default settings
     *
     * @return native value of status_t
     */
    public static native int resetTransmitProfiles();

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
     * @return ISemanticContext interface pointer
     */
    public static ISemanticContext getSemanticContext() {
        return new SemanticContext(nativeGetSemanticContext());
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final String value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final String value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null || value.trim().isEmpty())
            throw new IllegalArgumentException("value is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextStringValue(name, value, piiKind.getValue());
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final int value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final int value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextIntValue(name, value, piiKind.getValue());
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final long value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final long value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextLongValue(name, value, piiKind.getValue());
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final double value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final double value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextDoubleValue(name, value, piiKind.getValue());
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final boolean value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final boolean value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextBoolValue(name, value, piiKind.getValue());
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
     * @return native value of status_t
     */
    private static int setContext(final String name, final TimeTicks value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextTimeTicksValue(name, value.getTicks(), piiKind.getValue());
    }

    /**
     * Adds or overrides a property of the custom context for the telemetry logging system.
     * Context information set here applies to events generated by all ILogger instances
     * unless it is overwritten on a particular ILogger instance.
     * PiiKind_None is chosen by default.
     *
     * @param name Name of the context property
     * @param value Date value of the context property
     * @return native value of status_t
     */
    public static int setContext(final String name, final Date value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final Date value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

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
     * @return native value of status_t
     */
    public static int setContext(final String name, final UUID value) {
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
     * @return native value of status_t
     */
    public static int setContext(final String name, final UUID value, PiiKind piiKind) {
        if (name == null || name.trim().isEmpty())
            throw new IllegalArgumentException("name is null or empty");
        if (value == null)
            throw new IllegalArgumentException("value is null");
        if (piiKind == null)
            throw new IllegalArgumentException("piiKind is null");

        return nativeSetContextGuidValue(name, value.toString(), piiKind.getValue());
    }

    private static native int nativeGetLogger();

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     *
     * @return Instance of ILogger
     */
    public static ILogger GetLogger() {
        return new Logger(nativeGetLogger());
    }

    private static native int nativeGetLoggerWithSource(String source);

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     *
     * @param source Source name of events sent by this logger instance
     * @return Instance of ILogger
     */
    public static ILogger GetLogger(final String source) {
        return new Logger(nativeGetLoggerWithSource(source));
    }

    private static native int nativeGetLoggerWithTenantTokenAndSource(String tenantToken, String source);

    /**
     * Retrieves the ILogger interface of a Logger instance through which to log telemetry event.
     * 
     * @param tenantToken Token of the tenant with which the application is associated for collecting telemetry
     * @param source Source name of events sent by this logger instance
     * @return Instance of ILogger
     */
    public static ILogger GetLogger(final String tenantToken, final String source) {
        return new Logger(nativeGetLoggerWithTenantTokenAndSource(tenantToken, source));
    }
}
